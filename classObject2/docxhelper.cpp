#include "docxhelper.h"

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDomDocument>
#include <QDomNodeList>
#include <QDomElement>
#include <QDomText>
#include <QRegularExpression>
#include <QTextStream>
#include <QUuid>

/*
 * ==============================================
 * 通过 PowerShell 解压 docx 文件
 * Expand-Archive 是 PS 5.0+ 内置命令
 * ==============================================
 */
// 内部工具：转义路径中的单引号（PowerShell 单引号字符串内用 '' 表示一个字面单引号）
static QString escapeSingleQuote(const QString &path)
{
    return QString(path).replace("'", "''");
}

bool DocxHelper::unzipDocx(const QString &docxPath,
                            const QString &destDir,
                            QString &errorMsg)
{
    QProcess proc;
    // 先确保目标目录存在
    QDir().mkpath(destDir);

    // 构造 PowerShell 命令：用 .NET ZipFile 解压（不校验 .zip 扩展名）
    // Expand-Archive 会拒绝 .docx 后缀，而 .NET 的 ZipFile 只看文件内容
    QString cmd = QString(
        "Add-Type -AssemblyName System.IO.Compression.FileSystem;"
        "[System.IO.Compression.ZipFile]::ExtractToDirectory('%1', '%2')"
    ).arg(escapeSingleQuote(QDir::toNativeSeparators(docxPath)),
          escapeSingleQuote(QDir::toNativeSeparators(destDir)));

    proc.start("powershell", QStringList() << "-NoProfile" << "-Command" << cmd);
    proc.waitForFinished(30000);

    if (proc.exitCode() != 0) {
        QString errOut = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        errorMsg = QString("解压失败：%1").arg(errOut);
        return false;
    }
    return true;
}

/*
 * ==============================================
 * 通过 PowerShell 重新打包 docx
 * 注意：Compress-Archive 默认会把文件夹本身也打包进去
 * 所以用 * 通配符来压缩内部所有内容
 * ==============================================
 */
bool DocxHelper::zipDocx(const QString &srcDir,
                          const QString &docxPath,
                          QString &errorMsg)
{
    QProcess proc;

    // 先删除旧的 docx（.NET ZipFile.CreateFromDirectory 要求文件不存在）
    if (QFile::exists(docxPath)) {
        QFile::remove(docxPath);
    }

    QString srcPath = QDir::toNativeSeparators(srcDir);
    QString dstPath = QDir::toNativeSeparators(docxPath);

    // 用 .NET ZipFile 压缩（不校验 .zip 扩展名）
    // includeBaseDirectory:$false 确保只包含目录内容，不含目录本身
    QString cmd = QString(
        "Add-Type -AssemblyName System.IO.Compression.FileSystem;"
        "[System.IO.Compression.ZipFile]::CreateFromDirectory('%1', '%2', 'Optimal', $false)"
    ).arg(escapeSingleQuote(srcPath),
          escapeSingleQuote(dstPath));

    proc.start("powershell", QStringList() << "-NoProfile" << "-Command" << cmd);
    proc.waitForFinished(60000);

    if (proc.exitCode() != 0) {
        QString errOut = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        errorMsg = QString("压缩失败：%1").arg(errOut);
        return false;
    }
    return true;
}

/*
 * ==============================================
 * 读取 word/document.xml 全部内容
 * ==============================================
 */
QString DocxHelper::readDocumentXml(const QString &docxDir, QString &errorMsg)
{
    QString xmlPath = docxDir + "/word/document.xml";
    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMsg = QString("无法读取 document.xml：%1").arg(file.errorString());
        return QString();
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();
    return content;
}

/*
 * ==============================================
 * 写回 word/document.xml
 * ==============================================
 */
bool DocxHelper::writeDocumentXml(const QString &docxDir,
                                   const QString &xmlContent,
                                   QString &errorMsg)
{
    QString xmlPath = docxDir + "/word/document.xml";
    QFile file(xmlPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorMsg = QString("无法写入 document.xml：%1").arg(file.errorString());
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << xmlContent;
    file.close();
    return true;
}

/*
 * ==============================================
 * 从 docx 中提取成绩分数
 *
 * 逻辑：
 *   1. 解压 docx 到临时目录
 *   2. 读取 word/document.xml
 *   3. 按 <w:p>（段落）分组拼接 <w:t> 文本
 *   4. 第一遍：找含关键词的段落，提取关键词之后的数字
 *      （跨 <w:t> 节点——成绩数字常独立成节点）
 *   5. 第二遍：从底向上扫，返回最后一个 0-100 的数字
 *      （成绩通常在文档末尾）
 *   6. 清理临时目录
 * ==============================================
 */
QString DocxHelper::extractScore(const QString &docxPath, QString &errorMsg)
{
    // 搞个唯一的临时目录名，避免多线程打架
    QString tempDir = QDir::tempPath() + "/docx_" +
                      QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);

    // 解压
    if (!unzipDocx(docxPath, tempDir, errorMsg)) {
        QDir(tempDir).removeRecursively();
        return QString();
    }

    // 读取 XML
    QString xmlContent = readDocumentXml(tempDir, errorMsg);
    if (xmlContent.isEmpty()) {
        QDir(tempDir).removeRecursively();
        return QString();
    }

    // ==============================================
    // 解析 XML 提取分数
    // ==============================================
    QDomDocument doc;
    if (!doc.setContent(xmlContent)) {
        errorMsg = "document.xml 格式解析失败";
        QDir(tempDir).removeRecursively();
        return QString();
    }

    // 获取所有 w:p（段落）元素
    // 注：Qt 5.14.2 MinGW 的 elementsByTagNameNS 有 bug，
    // 所以用 elementsByTagName("w:p") 按全限定标签名查找
    QDomNodeList paragraphs = doc.elementsByTagName("w:p");

    // 收集每个段落中的 w:t 文本，按段落拼接
    QStringList paraTexts;
    for (int i = 0; i < paragraphs.size(); i++) {
        QDomElement pElem = paragraphs.item(i).toElement();
        QDomNodeList tNodes = pElem.elementsByTagName("w:t");
        QString combined;
        for (int j = 0; j < tNodes.size(); j++) {
            combined += tNodes.item(j).toElement().text().trimmed();
        }
        if (!combined.isEmpty())
            paraTexts.append(combined);
    }

    if (paraTexts.isEmpty()) {
        QDir(tempDir).removeRecursively();
        errorMsg = "文档中没有找到文本内容";
        return QString();
    }

    // ==============================================
    // 第一遍：关键词匹配（基于段落全文）
    // ==============================================
    QRegularExpression numRe("(\\d+[\\.]?\\d*)");
    QStringList scoreKeywords = {
        "成绩", "分数", "得分", "评分", "总评",
        "总分", "最终", "等级", "得分率", "正确率",
        "综合成绩", "实验成绩", "考试分数", "操作成绩"
    };

    for (int i = 0; i < paraTexts.size(); i++) {
        const QString &text = paraTexts[i];
        for (const QString &kw : scoreKeywords) {
            int kwPos = text.indexOf(kw);
            if (kwPos < 0)
                continue;
            // 取关键词后面的部分，提取第一个数字
            QString afterKw = text.mid(kwPos + kw.length());
            QRegularExpressionMatch match = numRe.match(afterKw);
            if (match.hasMatch()) {
                QString score = match.captured(1);
                // "评分"/"总评" 且分数 ≤ 10 时，很可能是 10 分制的教师评分
                // 跳过它，继续找百分制的成绩（如 "成绩"、"分数"、"得分"）
                if (kw == "评分" || kw == "总评") {
                    double val = score.toDouble();
                    if (val <= 10)
                        continue;
                }
                QDir(tempDir).removeRecursively();
                return score;
            }
        }
    }

    // ==============================================
    // 第二遍：从底向上扫描段落，返回最后一个 0-100 的数字
    // 成绩通常在文档末尾（实验步骤/总结之后）
    //
    // 注意：优先返回 > 10 的分数（百分制），
    // ≤ 10 的分数（如教师评分 9.23，10 分制）作为兜底
    // ==============================================
    QString weakScore; // 兜底分数（≤ 10）
    for (int i = paraTexts.size() - 1; i >= 0; i--) {
        const QString &text = paraTexts[i];
        QRegularExpressionMatchIterator it = numRe.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            bool ok = false;
            double val = m.captured(1).toDouble(&ok);
            if (ok && val >= 0 && val <= 100) {
                if (val > 10) {
                    // 百分制成绩，直接返回
                    QDir(tempDir).removeRecursively();
                    return m.captured(1);
                }
                // ≤ 10 的分数（可能是 10 分制教师评分），先留着兜底，继续往上找
                if (weakScore.isEmpty())
                    weakScore = m.captured(1);
            }
        }
    }

    // 实在没有 > 10 的分数，用 ≤ 10 的作为兜底
    if (!weakScore.isEmpty()) {
        QDir(tempDir).removeRecursively();
        return weakScore;
    }

    // 没找到分数
    QDir(tempDir).removeRecursively();
    errorMsg = "未在文档中找到成绩分数";
    return QString();
}

/*
 * ==============================================
 * 从 docx 中提取实验信息（目的、名称、内容）
 *
 * 逻辑：
 *   1. 解压 docx 到临时目录
 *   2. 读取 word/document.xml
 *   3. 按 <w:p>（段落）分组拼接 <w:t> 文本
 *   4. 扫描段落，查找包含以下关键字的段落：
 *      "实验目的"、"实验名称"、"实验内容"、"实验要求"
 *   5. 将找到的内容格式化为："实验名称：XXX\n实验目的：XXX\n实验内容：XXX"
 *   6. 清理临时目录
 * ==============================================
 */
QString DocxHelper::extractPurpose(const QString &docxPath, QString &errorMsg)
{
    // 临时目录
    QString tempDir = QDir::tempPath() + "/docx_" +
                      QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);

    // 解压
    if (!unzipDocx(docxPath, tempDir, errorMsg)) {
        QDir(tempDir).removeRecursively();
        return QString();
    }

    // 读取 XML
    QString xmlContent = readDocumentXml(tempDir, errorMsg);
    if (xmlContent.isEmpty()) {
        QDir(tempDir).removeRecursively();
        return QString();
    }

    // ==============================================
    // 解析 XML
    // ==============================================
    QDomDocument doc;
    if (!doc.setContent(xmlContent)) {
        // 解析失败不报错，直接返回空
        QDir(tempDir).removeRecursively();
        return QString();
    }

    QDomNodeList paragraphs = doc.elementsByTagName("w:p");

    // 收集每个段落中的 w:t 文本，按段落拼接
    QStringList paraTexts;
    for (int i = 0; i < paragraphs.size(); i++) {
        QDomElement pElem = paragraphs.item(i).toElement();
        QDomNodeList tNodes = pElem.elementsByTagName("w:t");
        QString combined;
        for (int j = 0; j < tNodes.size(); j++) {
            combined += tNodes.item(j).toElement().text().trimmed();
        }
        if (!combined.isEmpty())
            paraTexts.append(combined);
    }

    if (paraTexts.isEmpty()) {
        QDir(tempDir).removeRecursively();
        return QString();
    }

    // ==============================================
    // 扫描段落，提取实验相关信息
    // ==============================================
    struct { QString keyword; QString label; } purposeKeywords[] = {
        { "实验目的", "实验目的" },
        { "实验名称", "实验名称" },
        { "实验内容", "实验内容" },
        { "实验要求", "实验要求" },
    };

    QStringList foundLines;
    for (const auto &kw : purposeKeywords) {
        for (int i = 0; i < paraTexts.size(); i++) {
            const QString &text = paraTexts[i];
            int kwPos = text.indexOf(kw.keyword);
            if (kwPos < 0)
                continue;
            // 提取整个段落文本（已含关键词）
            QString line = text.trimmed();
            if (!line.isEmpty()) {
                foundLines.append(line);
            }
            break; // 每个关键词只取第一次出现
        }
    }

    QDir(tempDir).removeRecursively();

    if (foundLines.isEmpty())
        return QString();

    return foundLines.join("\n");
}

/*
 * ==============================================
 * 将 AI 评语写入 docx
 *
 * 逻辑：
 *   1. 解压 docx
 *   2. 读取 word/document.xml
 *   3. 找到包含 "教师评语" 或 "评语" 的段落（w:p）
 *   4. 清空该段落的所有文本，写入新评语
 *   5. 如果找不到，在文档末尾追加新段落
 *   6. 保存 XML，重新打包
 *   7. 清理临时目录
 * ==============================================
 */
bool DocxHelper::writeComment(const QString &docxPath,
                               const QString &comment,
                               QString &errorMsg)
{
    // 参数校验
    if (comment.isEmpty()) {
        errorMsg = "评语内容为空，跳过写入";
        return false;
    }

    // 临时目录
    QString tempDir = QDir::tempPath() + "/docx_" +
                      QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);

    // 解压
    if (!unzipDocx(docxPath, tempDir, errorMsg)) {
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 读取 XML
    QString xmlContent = readDocumentXml(tempDir, errorMsg);
    if (xmlContent.isEmpty()) {
        QDir(tempDir).removeRecursively();
        return false;
    }

    // ==============================================
    // 用 QDomDocument 解析和修改 XML
    // ==============================================
    QDomDocument doc;
    if (!doc.setContent(xmlContent)) {
        errorMsg = "document.xml 格式解析失败";
        QDir(tempDir).removeRecursively();
        return false;
    }

    QString ns = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    // 找到所有 w:t 文本节点
    // 注：Qt 5.14.2 MinGW 的 elementsByTagNameNS 有 bug（始终返回空），
    // 所以改用 elementsByTagName("w:t") 按全限定标签名查找
    QDomNodeList textNodes = doc.elementsByTagName("w:t");

    bool found = false;
    QDomElement targetParagraph; // 要替换的段落

    // 遍历找 "评语" 关键字
    for (int i = 0; i < textNodes.size(); i++) {
        QDomElement tElem = textNodes.item(i).toElement();
        if (tElem.text().contains("评语")) {
            // 找到评语标签，往上找父级 w:p 段落
            QDomNode parent = tElem.parentNode(); // w:r
            while (!parent.isNull() && parent.isElement()) {
                QDomElement pe = parent.toElement();
                if (pe.tagName() == "w:p") {
                    targetParagraph = pe;
                    found = true;
                    break;
                }
                parent = parent.parentNode();
            }
            if (found) break;
        }
    }

    if (found) {
        // ==============================================
        // 找到评语段落了
        // 删掉里面的所有 w:r（文本运行），替换为新的评语
        // 保留段落属性（w:pPr）不删，保持格式
        // ==============================================
        QDomNodeList runs = targetParagraph.elementsByTagName("w:r");

        // 从后往前删，避免索引错乱
        QList<QDomElement> runList;
        for (int i = 0; i < runs.size(); i++) {
            runList.append(runs.item(i).toElement());
        }
        for (int i = runList.size() - 1; i >= 0; i--) {
            targetParagraph.removeChild(runList[i]);
        }

        // 创建新的 w:r → w:t
        QDomElement newRun = doc.createElementNS(ns, "r");
        QDomElement newText = doc.createElementNS(ns, "t");
        // 设置 xml:space="preserve" 保持空格和换行
        newText.setAttribute("xml:space", "preserve");

        QDomText textNode = doc.createTextNode("教师评语：" + comment);
        newText.appendChild(textNode);
        newRun.appendChild(newText);
        targetParagraph.appendChild(newRun);
    } else {
        // ==============================================
        // 没找到评语段落，在文档末尾追加
        // 找到 <w:body>，在 <w:sectPr> 之前插入新段落
        // 如果没有 sectPr，直接追加到 body 末尾
        // ==============================================
    QDomElement docElem = doc.documentElement();
    // firstChildElement 在 Qt 5.14 只支持标签名参数（带前缀）
    // docx 的标准前缀是 w:，所以这里用 "w:body"
    QDomElement body = docElem.firstChildElement("w:body");

    if (body.isNull()) {
        errorMsg = "文档结构异常：未找到 body 元素";
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 创建新段落
    QDomElement newP = doc.createElementNS(ns, "p");
    QDomElement newRun = doc.createElementNS(ns, "r");
    QDomElement newText = doc.createElementNS(ns, "t");
    newText.setAttribute("xml:space", "preserve");

    QDomText textNode = doc.createTextNode("教师评语：" + comment);
    newText.appendChild(textNode);
    newRun.appendChild(newText);
    newP.appendChild(newRun);

    // 在 sectPr 之前插入
    QDomElement sectPr = body.firstChildElement("w:sectPr");
    if (!sectPr.isNull()) {
        body.insertBefore(newP, sectPr);
    } else {
        body.appendChild(newP);
    }
    }

    // ==============================================
    // 把修改后的 DOM 树序列化成字符串
    // 注意：要保留原 XML 声明，不缩进
    // ==============================================
    QString newXmlContent;
    QTextStream ts(&newXmlContent);
    doc.save(ts, 0); // 0 = 不缩进，保持原有格式
    // 确保 XML 声明存在
    if (!newXmlContent.startsWith("<?xml")) {
        newXmlContent = QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n") + newXmlContent;
    }

    // 写回文件
    if (!writeDocumentXml(tempDir, newXmlContent, errorMsg)) {
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 重新打包为 docx
    if (!zipDocx(tempDir, docxPath, errorMsg)) {
        QDir(tempDir).removeRecursively();
        return false;
    }

    // 清理临时目录
    QDir(tempDir).removeRecursively();

    return true;
}
