#include "deepseekapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QSslError>
#include <QSslSocket>
#include <QDebug>
#include <QUrl>

DeepSeekApi::DeepSeekApi(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_endpoint("https://api.deepseek.com")
    , m_apiKey("")
    , m_model("deepseek-chat")
    , m_temperature(0.7)
{
}

void DeepSeekApi::setEndpoint(const QString &url)
{
    m_endpoint = url;
    // 去掉末尾斜杠，免得拼 URL 时出双斜杠
    while (m_endpoint.endsWith('/'))
        m_endpoint.chop(1);
}

void DeepSeekApi::setApiKey(const QString &key)
{
    m_apiKey = key;
}

void DeepSeekApi::setTemperature(double temp)
{
    m_temperature = qBound(0.0, temp, 1.0);
}

void DeepSeekApi::setModel(const QString &model)
{
    if (!model.isEmpty())
        m_model = model;
}

/*
 * ==============================================
 * 同步调用 DeepSeek API
 * 思路：用 QEventLoop 等待网络回复
 * 加个 60s 超时，避免网络卡死
 * ==============================================
 */
QString DeepSeekApi::generateCommentSync(const QString &score,
                                          const QString &prompt,
                                          QString &errorMsg)
{
    // 参数校验
    if (m_endpoint.isEmpty() || m_apiKey.isEmpty()) {
        errorMsg = "API 配置不完整，请检查 URL 和 API Key";
        return QString();
    }

    // 检查 SSL 支持（Windows 上常因缺少 OpenSSL DLL 导致 HTTPS 失败）
    if (!QSslSocket::supportsSsl()) {
        errorMsg = "SSL 初始化失败，找不到 OpenSSL 库（libeay32.dll / ssleay32.dll）\n"
                   "请将 Qt 安装目录下的这两个 DLL 复制到程序所在目录。";
        qWarning() << "SSL 库版本：" << QSslSocket::sslLibraryBuildVersionString();
        qWarning() << "缺少 OpenSSL DLL，HTTPS 请求将失败。";
        return QString();
    }

    // 构建请求 body
    QJsonObject body;
    body["model"] = m_model;

    QJsonArray messages;

    // system 角色：告诉 AI 它要扮演什么
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = "你是一名高校专业课教师。请根据学生实验成绩和实验要求，"
                        "生成一段专业、具体、有针对性的教师评语。"
                        "评语要求：语气客观公正，先肯定优点再指出不足，最后给出改进建议。"
                        "字数控制在50-120字之间。";
    messages.append(sysMsg);

    // user 角色：传入实际数据
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = QString("学生实验成绩：%1分\n实验要求：%2\n请生成教师评语。")
                             .arg(score, prompt);
    messages.append(userMsg);

    body["messages"] = messages;
    body["temperature"] = m_temperature;
    // 让回复稳定一点，少发散
    body["top_p"] = 0.9;

    QJsonDocument doc(body);
    QByteArray postData = doc.toJson(QJsonDocument::Compact);

    // 构建网络请求：从用户 URL 中提取 scheme + host[:port]，忽略任何已有路径
    QUrl parsedUrl(m_endpoint);
    QString apiPath = parsedUrl.scheme() + "://" + parsedUrl.host();
    if (parsedUrl.port() > 0)
        apiPath += ":" + QString::number(parsedUrl.port());
    apiPath += "/chat/completions";
    QUrl url(apiPath);
    QNetworkRequest request(url);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_apiKey).toUtf8());

    // 发 POST
    QNetworkReply *reply = m_manager->post(request, postData);

    // 处理 SSL 错误（Windows 上缺少 OpenSSL DLL 时必现）
    // 忽略 SSL 错误以便继续获取详细错误信息
    QObject::connect(reply, &QNetworkReply::sslErrors,
                     [reply](const QList<QSslError> &errors) {
        QStringList errList;
        for (const QSslError &e : errors)
            errList << e.errorString();
        qWarning("SSL 错误：%s", errList.join("; ").toUtf8().constData());
        reply->ignoreSslErrors();
    });

    // 用事件循环等待回复
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    // 超时处理：取消请求
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        reply->abort();
        timer.stop();
    });

    // 收到回复就退出事件循环
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start(60000); // 60 秒超时
    loop.exec();

    // 处理结果
    QString result;
    if (timer.isActive()) {
        timer.stop(); // 正常完成，关闭定时器

        QByteArray responseData = reply->readAll();
        int httpStatus = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() != QNetworkReply::NoError) {
            // 附带 HTTP 状态码和返回内容，方便排查
            QString respPreview = QString::fromUtf8(responseData.left(500));
            errorMsg = QString("网络请求失败（HTTP %1）：%2")
                           .arg(httpStatus)
                           .arg(reply->errorString());
            if (!respPreview.isEmpty())
                errorMsg += "\n返回内容：" + respPreview;
        } else {
            QJsonDocument respDoc = QJsonDocument::fromJson(responseData);

            if (respDoc.isNull() || !respDoc.isObject()) {
                errorMsg = QString("API 返回数据格式异常，不是有效的 JSON（HTTP %1）").arg(httpStatus);
            } else {
                QJsonObject respObj = respDoc.object();

                // 检查是否有 error 字段（API 返回的错误）
                if (respObj.contains("error")) {
                    QJsonObject errObj = respObj["error"].toObject();
                    errorMsg = QString("API 返回错误（HTTP %1）：%2")
                                   .arg(httpStatus)
                                   .arg(errObj["message"].toString());
                } else {
                    // 正常解析 choices[0].message.content
                    QJsonArray choices = respObj["choices"].toArray();
                    if (choices.isEmpty()) {
                        errorMsg = QString("API 返回的 choices 数组为空（HTTP %1）").arg(httpStatus);
                    } else {
                        QJsonObject choice = choices[0].toObject();
                        QJsonObject message = choice["message"].toObject();
                        result = message["content"].toString().trimmed();
                        if (result.isEmpty()) {
                            errorMsg = "AI 生成的评语为空";
                        }
                    }
                }
            }
        }
    } else {
        // 定时器超时了
        errorMsg = "请求超时（60秒），请检查网络或 API 地址是否正确";
    }

    reply->deleteLater();
    return result;
}
