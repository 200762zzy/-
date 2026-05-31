#ifndef TEST_DEEPSEEKAPI_H
#define TEST_DEEPSEEKAPI_H

#include <QObject>

class TestDeepSeekApi : public QObject
{
    Q_OBJECT

private slots:
    void testSetEndpoint_NormalizesUrl();
    void testSetTemperature_ClampsBelowZero();
    void testSetTemperature_ClampsAboveOne();
    void testSetTemperature_NormalValues();
    void testSetModel_EmptyIgnored();
    void testSetModel_NormalModel();
    void testGenerateCommentSync_NoEndpoint();
    void testGenerateCommentSync_NoApiKey();
};

#endif
