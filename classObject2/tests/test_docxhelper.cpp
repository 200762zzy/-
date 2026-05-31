#include "test_docxhelper.h"
#include "docxhelper.h"
#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDateTime>
#include <QUuid>


// 转义 PowerShell 单引号
static QString esc(const QString &s)
{
    return QString(s).replace("'", "''");
}

// ==============================================
// 通过 PowerShell + .NET ZipFile 创建 .docx
// ==============================================
static bool runPowerShellZip(const QString &srcDir, const QString &dstPath)
{
    QProcess proc;
    QString cmd = QString(
        "Add-Type -AssemblyName System.IO.Compression.FileSystem;"
        "[System.IO.Compression.ZipFile]::CreateFromDirectory('%1', '%2', 'Optimal', $false)"
    ).arg(esc(QDir::toNativeSeparators(srcDir)),
          esc(QDir::toNativeSeparators(dstPath)));
    proc.start("powershell", QStringList() << "-NoProfile" << "-Command" << cmd);
    proc.waitForFinished(60000);
    return proc.exitCode() == 0;
}

// ==============================================
// 生成标准的 docx XML 结构
// ==============================================
static bool writeDocxStructure(const QString &baseDir,
                                const QString &documentXml)
{
    // [Content_Types].xml
    QFile ctFile(baseDir + "/[Content_Types].xml");
    if (!ctFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    ctFile.write(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n"
        "  <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
        "  <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n"
        "  <Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>\n"
        "</Types>\n");
    ctFile.close();

    // _rels/.rels
    QDir().mkpath(baseDir + "/_rels");
    QFile relsFile(baseDir + "/_rels/.rels");
    if (!relsFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    relsFile.write(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
        "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>\n"
        "</Relationships>\n");
    relsFile.close();

    // word/_rels/document.xml.rels
    QDir().mkpath(baseDir + "/word/_rels");
    QFile wrelsFile(baseDir + "/word/_rels/document.xml.rels");
    if (!wrelsFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    wrelsFile.write(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
        "</Relationships>\n");
    wrelsFile.close();

    // word/document.xml
    QFile docFile(baseDir + "/word/document.xml");
    if (!docFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    docFile.write(documentXml.toUtf8());
    docFile.close();

    return true;
}

// ==============================================
// 构建包含指定 body 内容的标准 document.xml
// ==============================================
QString TestDocxHelper::buildDocxXml(const QString &bodyContent)
{
    return QString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">\n"
        "  <w:body>\n"
        "%1\n"
        "  </w:body>\n"
        "</w:document>"
    ).arg(bodyContent);
}

// ==============================================
// 创建测试 .docx 文件，返回路径
// ==============================================
QString TestDocxHelper::createTestDocx(const QString &documentXml)
{
    QString uniq = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    QString tmpSrc = m_tempDir + "/src_" + uniq;
    QString docxPath = m_tempDir + "/test_" + uniq + ".docx";

    QDir().mkpath(tmpSrc);

    if (!writeDocxStructure(tmpSrc, documentXml)) {
        QDir(tmpSrc).removeRecursively();
        return QString();
    }

    if (!runPowerShellZip(tmpSrc, docxPath)) {
        QDir(tmpSrc).removeRecursively();
        return QString();
    }

    QDir(tmpSrc).removeRecursively();
    return docxPath;
}

// ==============================================
// 助手：生成一个包含指定文本的段落 XML
// ==============================================
static QString paragraph(const QString &text)
{
    // 转义 XML 特殊字符
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&apos;");
    return QString(
        "    <w:p>\n"
        "      <w:r>\n"
        "        <w:t>%1</w:t>\n"
        "      </w:r>\n"
        "    </w:p>\n"
    ).arg(escaped);
}

// ==============================================
// 包含教师评语的段落（用于 writeComment 测试）
// ==============================================
static QString commentParagraph(const QString &text)
{
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    return QString(
        "    <w:p>\n"
        "      <w:r>\n"
        "        <w:t>%1</w:t>\n"
        "      </w:r>\n"
        "    </w:p>\n"
    ).arg(escaped);
}

// ==============================================
// 设置/清理
// ==============================================
void TestDocxHelper::initTestCase()
{
    m_tempDir = QDir::tempPath() + "/test_classObject2_" +
                QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    QDir().mkpath(m_tempDir);
}

void TestDocxHelper::cleanupTestCase()
{
    QDir(m_tempDir).removeRecursively();
}

// ==============================================
// extractScore 测试
// ==============================================

void TestDocxHelper::testExtractScore_WithScoreKeyword()
{
    // "成绩：85分"
    QString body = paragraph("学生姓名：张三")
                 + paragraph("实验名称：数据结构")
                 + paragraph("成绩：85分");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY2(!docxPath.isEmpty(), "创建测试 docx 失败");

    QString errorMsg;
    QString score = DocxHelper::extractScore(docxPath, errorMsg);

    QCOMPARE(score, QString("85"));
    QFile::remove(docxPath);
}

void TestDocxHelper::testExtractScore_WithMultipleKeywords()
{
    // 测试各种关键词：分数、得分、评分、总评
    struct TestCase {
        QString label;
        QString keyword;
        QString expected;
    };

    QList<TestCase> cases = {
        {"分数",   "分数：92",    "92"},
        {"得分",   "得分：78.5",  "78.5"},
        {"评分",   "评分：88",    "88"},
        {"总评",   "总评：95",    "95"},
        {"最终",   "最终成绩：60","60"},
        {"实验成绩","实验成绩：73","73"},
    };

    for (const auto &tc : cases) {
        QString body = paragraph(tc.label + tc.keyword);
        QString xml = buildDocxXml(body);
        QString docxPath = createTestDocx(xml);
        QVERIFY2(!docxPath.isEmpty(),
                 ("创建测试 docx 失败：" + tc.label).toUtf8().constData());

        QString errorMsg;
        QString score = DocxHelper::extractScore(docxPath, errorMsg);
        QCOMPARE(score, tc.expected);

        QFile::remove(docxPath);
    }
}

void TestDocxHelper::testExtractScore_WithDecimalScore()
{
    // 小数成绩
    QString body = paragraph("综合成绩：89.5分");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    QString score = DocxHelper::extractScore(docxPath, errorMsg);

    QCOMPARE(score, QString("89.5"));
    QFile::remove(docxPath);
}

void TestDocxHelper::testExtractScore_FallbackNoKeyword()
{
    // 第二遍扫描：没有关键词但存在 0~100 的数字
    QString body = paragraph("实验报告")
                 + paragraph("最终评定")
                 + paragraph("73"); // 没有关键词，但数字在 0-100 范围内
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    QString score = DocxHelper::extractScore(docxPath, errorMsg);

    QCOMPARE(score, QString("73"));
    QFile::remove(docxPath);
}

void TestDocxHelper::testExtractScore_NoScore()
{
    // 没有任何数字
    QString body = paragraph("这是一份实验报告")
                 + paragraph("没有成绩信息");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    QString score = DocxHelper::extractScore(docxPath, errorMsg);

    QVERIFY2(score.isEmpty(), "没有成绩时应返回空字符串");
    QVERIFY2(!errorMsg.isEmpty(), "没有成绩时应设置错误消息");
    QVERIFY(errorMsg.contains("成绩") || errorMsg.contains("分数"));
    QFile::remove(docxPath);
}

void TestDocxHelper::testExtractScore_InvalidFile()
{
    // 无效文件路径
    QString errorMsg;
    QString score = DocxHelper::extractScore("Z:/nonexistent/doc.docx", errorMsg);

    QVERIFY2(score.isEmpty(), "无效文件应返回空结果");
    QVERIFY2(!errorMsg.isEmpty(), "无效文件应设置错误消息");
}

// ==============================================
// writeComment 测试
// ==============================================

void TestDocxHelper::testWriteComment_ReplaceExisting()
{
    // 文档包含 "教师评语：" 段落，应替换
    QString body = paragraph("成绩：90分")
                 + commentParagraph("教师评语：该生表现良好。");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    bool ok = DocxHelper::writeComment(docxPath, "该生表现优秀，代码规范。", errorMsg);

    QVERIFY2(ok, ("写入评语失败：" + errorMsg).toUtf8().constData());
    QFile::remove(docxPath);
}

void TestDocxHelper::testWriteComment_AppendNew()
{
    // 文档没有 "评语" 关键词，应在末尾追加
    QString body = paragraph("成绩：90分")
                 + paragraph("实验内容：实现一个计算器")
                 + paragraph("总结：实验完成。");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    bool ok = DocxHelper::writeComment(docxPath, "该生实验态度端正，代码规范。", errorMsg);

    QVERIFY2(ok, ("追加评语失败：" + errorMsg).toUtf8().constData());
    QFile::remove(docxPath);
}

void TestDocxHelper::testWriteComment_EmptyComment()
{
    // 空评语应返回 false
    QString body = paragraph("成绩：85分");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    bool ok = DocxHelper::writeComment(docxPath, "", errorMsg);

    QVERIFY2(!ok, "空评语应返回 false");
    QVERIFY2(!errorMsg.isEmpty(), "空评语应设置错误消息");
    QFile::remove(docxPath);
}

void TestDocxHelper::testWriteComment_VerifyContent()
{
    // 写入后重新读取验证内容
    QString body = paragraph("成绩：95分")
                 + commentParagraph("教师评语：原始评语。");
    QString xml = buildDocxXml(body);
    QString docxPath = createTestDocx(xml);
    QVERIFY(!docxPath.isEmpty());

    QString errorMsg;
    bool ok = DocxHelper::writeComment(docxPath, "新评语：该生非常优秀。", errorMsg);
    QVERIFY2(ok, ("写入评语失败：" + errorMsg).toUtf8().constData());

    // 写入后提取成绩（验证文档仍然有效）
    QString score = DocxHelper::extractScore(docxPath, errorMsg);
    QCOMPARE(score, QString("95"));

    QFile::remove(docxPath);
}
