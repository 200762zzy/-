#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QString>
#include <QAtomicInt>

/*
 * WorkerThread - 后台处理线程
 *
 * 为什么需要子线程？
 *   因为处理 docx + 调 API 都是耗时操作
 *   如果在主线程跑，界面会卡死，用户以为程序崩溃了
 *
 * 工作流程：
 *   1. 主线程把参数传进来，start() 启动
 *   2. run() 里遍历文件夹所有 .docx
 *   3. 每个文件：提取分数 → 调 API → 写评语
 *   4. 用信号把进度和日志发回主线程
 *   5. 处理完发射 finished 信号
 */
class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread(QObject *parent = nullptr);

    // 设置处理参数（在主线程调用 start 之前设置）
    void setParams(const QString &folderPath,
                   const QString &apiUrl,
                   const QString &apiKey,
                   const QString &prompt,
                   double temperature,
                   bool backupToSubfolder);

    // 请求停止处理（线程安全，原子操作）
    void requestStop();

signals:
    /*
     * 日志消息
     * type: 0=信息 1=成功 2=警告 3=错误
     */
    void logMessage(const QString &msg, int type);

    // 进度更新
    void progressChanged(int current, int total);

    // 全部处理完成（不管成功还是失败）
    void finished(int successCount, int failCount);

protected:
    void run() override;

private:
    QString m_folderPath;
    QString m_apiUrl;
    QString m_apiKey;
    QString m_prompt;
    double  m_temperature;
    bool    m_backupToSubfolder;

    // 停止标志：原子操作，多线程安全
    QAtomicInt m_stopFlag;
};

#endif // WORKERTHREAD_H
