#include "workerthread.h"
#include "docxhelper.h"
#include "deepseekapi.h"

#include <QDir>
#include <QFileInfo>

WorkerThread::WorkerThread(QObject *parent)
    : QThread(parent)
    , m_temperature(0.7)
    , m_backupToSubfolder(true)
    , m_stopFlag(0)
{
}

void WorkerThread::setParams(const QString &folderPath,
                              const QString &apiUrl,
                              const QString &apiKey,
                              const QString &prompt,
                              double temperature,
                              bool backupToSubfolder)
{
    m_folderPath = folderPath;
    m_apiUrl = apiUrl;
    m_apiKey = apiKey;
    m_prompt = prompt;
    m_temperature = temperature;
    m_backupToSubfolder = backupToSubfolder;
}

void WorkerThread::requestStop()
{
    // 原子操作设标志，run() 里会轮询这个标志
    m_stopFlag.storeRelaxed(1);
}

/*
 * ==============================================
 * 子线程主入口
 *
 * 这里才是真正干活的地方
 * 注意：不能直接操作 UI，必须通过信号通信
 * ==============================================
 */
void WorkerThread::run()
{
    m_stopFlag.storeRelaxed(0); // 重置停止标志

    // ==============================================
    // 先扫描文件夹，收集所有 docx 文件
    // ==============================================
    QDir dir(m_folderPath);
    QStringList filters;
    filters << "*.docx";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    if (files.isEmpty()) {
        emit logMessage("文件夹中没有找到 .docx 文件", 2);
        emit finished(0, 0);
        return;
    }

    emit logMessage(QString("找到 %1 份实验报告，开始处理...").arg(files.size()), 0);

    // ==============================================
    // 初始化 DeepSeek API
    // 注意：QNetworkAccessManager 要在当前线程创建
    // 因为它内部依赖事件循环
    // ==============================================
    DeepSeekApi api;
    api.setEndpoint(m_apiUrl);
    api.setApiKey(m_apiKey);
    api.setTemperature(m_temperature);

    // ==============================================
    // 如果勾选了"另存到子文件夹"
    // 就创建 "原文件夹/已评语/" 目录
    // ==============================================
    QString backupDir;
    if (m_backupToSubfolder) {
        backupDir = m_folderPath + "/已评语";
        QDir().mkpath(backupDir);
        emit logMessage(QString("处理后的文件将保存到：%1").arg(backupDir), 0);
    }

    int successCount = 0;
    int failCount = 0;
    int total = files.size();

    // ==============================================
    // 逐个处理文件
    // ==============================================
    for (int i = 0; i < total; i++) {
        // 检查是否要停止
        if (m_stopFlag.loadRelaxed() == 1) {
            emit logMessage("用户中断处理", 2);
            break;
        }

        QFileInfo fileInfo = files[i];
        QString filePath = fileInfo.absoluteFilePath();
        QString fileName = fileInfo.fileName();

        emit logMessage(QString("[%1/%2] 正在处理：%3")
                            .arg(i + 1).arg(total).arg(fileName), 0);
        emit progressChanged(i, total);

        // ---- 步骤1：提取成绩分数 ----
        QString errorMsg;
        QString score = DocxHelper::extractScore(filePath, errorMsg);

        if (score.isEmpty()) {
            emit logMessage(QString("  ⚠ %1：%2").arg(fileName, errorMsg), 2);
            // 分数为空也能继续，给 AI 传个"未知"就行
            score = "未知";
        } else {
            emit logMessage(QString("  ✓ 提取到成绩：%1 分").arg(score), 1);
        }

        // ---- 步骤2：调 API 生成评语 ----
        QString apiError;
        QString comment = api.generateCommentSync(score, m_prompt, apiError);

        if (comment.isEmpty()) {
            emit logMessage(QString("  ✗ %1：%2").arg(fileName, apiError), 3);
            failCount++;
            continue; // 跳过这个文件
        }

        emit logMessage(QString("  ✓ AI 评语生成成功（%1字）")
                            .arg(comment.length()), 1);

        // ---- 步骤3：写入评语到 docx ----
        // 如果勾选了另存，就处理副本，不碰原文件
        QString targetPath = filePath;
        if (m_backupToSubfolder) {
            QString newPath = backupDir + "/" + fileName;
            // 复制原文件到备份目录
            if (QFile::copy(filePath, newPath)) {
                targetPath = newPath;
                emit logMessage(QString("  → 已复制到：%1").arg(newPath), 0);
            } else {
                // 复制失败就用原文件
                emit logMessage(QString("  ⚠ 复制失败，将在原文件上修改").arg(fileName), 2);
                targetPath = filePath;
            }
        }

        bool writeOk = DocxHelper::writeComment(targetPath, comment, errorMsg);

        if (writeOk) {
            emit logMessage(QString("  ✅ %1 评语写入成功！").arg(fileName), 1);
            successCount++;
        } else {
            emit logMessage(QString("  ❌ %1：%2").arg(fileName, errorMsg), 3);
            failCount++;
        }

        // 稍微发一下进度，让 UI 顺畅更新
        emit progressChanged(i + 1, total);
    }

    // ==============================================
    // 处理完成，发汇总信息
    // ==============================================
    emit progressChanged(total, total);

    QString summary = QString("处理完成！成功 %1 份，失败 %2 份，共处理 %3 份文件")
                          .arg(successCount)
                          .arg(failCount)
                          .arg(successCount + failCount);

    if (m_stopFlag.loadRelaxed() == 1) {
        summary = QString("已中断！成功 %1 份，失败 %2 份（已处理 %3/%4 份）")
                      .arg(successCount)
                      .arg(failCount)
                      .arg(successCount + failCount)
                      .arg(total);
    }

    emit logMessage(summary, successCount > 0 ? 1 : 3);
    emit finished(successCount, failCount);
}
