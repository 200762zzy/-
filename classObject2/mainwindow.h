#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>

class QLineEdit;
class QTextEdit;
class QPushButton;
class QProgressBar;
class QSlider;
class QCheckBox;
class QLabel;
class WorkerThread;
class QSettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseFolder();
    void onStart();
    void onStop();
    void onReset();
    void onLogMessage(const QString &msg, int type);
    void onProgressChanged(int current, int total);
    void onProcessingFinished(int successCount, int failCount);

private:
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void setControlsEnabled(bool enabled);
    void appendLog(const QString &msg, int type);

    // 毛玻璃 UI 辅助方法
    QFrame* createGlassCard();
    void setupGlobalStyle();

    // ==================== UI 控件成员 ====================

    QLineEdit   *m_urlEdit;
    QLineEdit   *m_apiKeyEdit;
    QSlider     *m_tempSlider;
    QLabel      *m_tempLabel;

    QLineEdit   *m_pathEdit;
    QPushButton *m_browseBtn;
    QCheckBox   *m_backupCheck;

    QTextEdit   *m_promptEdit;

    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_resetBtn;

    QProgressBar *m_progressBar;
    QTextEdit   *m_logEdit;

    // ==================== 非UI成员 ====================

    WorkerThread *m_worker;
    QSettings    *m_settings;
};

#endif // MAINWINDOW_H
