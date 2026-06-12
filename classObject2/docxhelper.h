#ifndef DOCXHELPER_H
#define DOCXHELPER_H

#include <QString>

/*
 * DocxHelper - 处理 Word 文档（.docx）的读取和写入
 *
 * 核心思路：
 *   .docx 其实是一个 ZIP 压缩包，里面是 XML 文件
 *   我们用 PowerShell 的 Expand-Archive / Compress-Archive 解压和打包
 *   用 QDomDocument 解析修改 word/document.xml
 *   这样就不用引入第三方库了
 */
class DocxHelper
{
public:
    /*
     * 从 docx 中提取成绩分数
     * 扫描所有文本，找包含"成绩/分数/得分"关键字的段落
     * 用正则提取其中的数字
     * 失败返回空字符串
     */
    static QString extractScore(const QString &docxPath, QString &errorMsg);

    /*
     * 从 docx 中提取实验信息（目的、名称、内容）
     * 扫描段落查找含"实验目的"/"实验名称"/"实验内容"的文本
     * 返回格式化字符串如："实验名称：XXX\n实验目的：XXX\n实验内容：XXX"
     * 如果都没找到返回空字符串（不报错）
     */
    static QString extractPurpose(const QString &docxPath, QString &errorMsg);

    /*
     * 将 AI 评语写入 docx
     * 找到包含"教师评语"或"评语"的段落，替换其内容
     * 如果找不到，在文档末尾追加新段落
     * 写入成功返回 true
     */
    static bool writeComment(const QString &docxPath,
                             const QString &comment,
                             QString &errorMsg);

private:
    // 内部工具方法：解压 docx 到临时目录
    static bool unzipDocx(const QString &docxPath,
                          const QString &destDir,
                          QString &errorMsg);
    // 内部工具方法：将临时目录重新打包为 docx
    static bool zipDocx(const QString &srcDir,
                        const QString &docxPath,
                        QString &errorMsg);
    // 读取 word/document.xml 内容
    static QString readDocumentXml(const QString &docxDir, QString &errorMsg);
    // 写回 word/document.xml
    static bool writeDocumentXml(const QString &docxDir,
                                 const QString &xmlContent,
                                 QString &errorMsg);
};

#endif // DOCXHELPER_H
