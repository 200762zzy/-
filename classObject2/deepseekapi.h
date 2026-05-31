#ifndef DEEPSEEKAPI_H
#define DEEPSEEKAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

/*
 * DeepSeekApi - 负责调用 DeepSeek 大模型接口
 * 设计成同步模式，方便在工作线程中使用
 * 工作线程里跑 QEventLoop 等网络返回，不阻塞主界面
 */
class DeepSeekApi : public QObject
{
    Q_OBJECT
public:
    explicit DeepSeekApi(QObject *parent = nullptr);

    void setEndpoint(const QString &url);
    void setApiKey(const QString &key);
    void setTemperature(double temp);
    void setModel(const QString &model);

    /*
     * 同步调用 DeepSeek API
     * 传入 成绩分数 + 自定义提示词
     * 返回 AI 生成的评语字符串
     * 如果出错，errorMsg 会附带错误描述
     */
    QString generateCommentSync(const QString &score,
                                const QString &prompt,
                                QString &errorMsg);

private:
    QNetworkAccessManager *m_manager;
    QString m_endpoint;       // DeepSeek 接口地址
    QString m_apiKey;         // API Key
    QString m_model;          // 模型名称
    double  m_temperature;    // 创造度参数 0~1
};

#endif // DEEPSEEKAPI_H
