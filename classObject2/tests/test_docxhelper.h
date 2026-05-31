#ifndef TEST_DOCXHELPER_H
#define TEST_DOCXHELPER_H

#include <QObject>
#include <QString>

class TestDocxHelper : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testExtractScore_WithScoreKeyword();
    void testExtractScore_WithMultipleKeywords();
    void testExtractScore_WithDecimalScore();
    void testExtractScore_FallbackNoKeyword();
    void testExtractScore_NoScore();
    void testExtractScore_InvalidFile();

    void testWriteComment_ReplaceExisting();
    void testWriteComment_AppendNew();
    void testWriteComment_EmptyComment();
    void testWriteComment_VerifyContent();

private:
    QString createTestDocx(const QString &documentXml);
    QString buildDocxXml(const QString &bodyContent);
    QString m_tempDir;
};

#endif
