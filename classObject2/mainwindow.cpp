#include "mainwindow.h"
#include "workerthread.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>
#include <QFont>
#include <QFontInfo>
#include <QScrollBar>
#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_worker(nullptr)
    , m_settings(new QSettings("ClassObject2", "AICommentGenerator", this))
{
    setupUI();
    setupConnections();
    loadSettings();
    m_stopBtn->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if (m_worker && m_worker->isRunning()) {
        m_worker->requestStop();
        m_worker->wait(3000);
    }
}

/*
 * ==============================================
 * 创建毛玻璃卡片
 *
 * 半透明背景 + 圆角 + 阴影
 * 所有的功能区都包在卡片里
 * ==============================================
 */
QFrame* MainWindow::createGlassCard()
{
    QFrame *card = new QFrame();
    card->setObjectName("glassCard");
    card->setStyleSheet(
        "QFrame#glassCard {"
        "  background: rgba(255,255,255,0.68);"
        "  border: 1px solid rgba(255,255,255,0.55);"
        "  border-radius: 16px;"
        "}"
    );
    // 毛玻璃阴影：柔和的大面积阴影，产生悬浮感
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(48);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 28));
    card->setGraphicsEffect(shadow);
    return card;
}

/*
 * ==============================================
 * 全局毛玻璃样式表
 *
 * 覆盖所有控件的默认样式
 * 统一的圆角、半透明、配色
 * ==============================================
 */
void MainWindow::setupGlobalStyle()
{
    setStyleSheet(R"(
        /* ========== 主窗口背景渐变 ========== */
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #e8eaf6,
                stop:0.4 #f3e5f5,
                stop:0.7 #e0f2f1,
                stop:1 #fff3e0);
        }

        /* ========== 输入框 ========== */
        QLineEdit, QTextEdit, QPlainTextEdit {
            background: rgba(255,255,255,0.82);
            border: 1px solid rgba(0,0,0,0.08);
            border-radius: 10px;
            padding: 8px 14px;
            font-size: 13px;
            color: #2d3436;
            selection-background-color: #7c4dff;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border: 2px solid #7c4dff;
            background: rgba(255,255,255,0.95);
        }
        QLineEdit:disabled, QTextEdit:disabled {
            background: rgba(255,255,255,0.35);
            color: #b2bec3;
        }
        QLineEdit[readOnly="true"] {
            background: rgba(255,255,255,0.4);
        }

        /* ========== 下拉框/浏览按钮专用 ========== */
        QPushButton#browseBtn {
            background: rgba(124,77,255,0.12);
            border: 1px solid rgba(124,77,255,0.2);
            border-radius: 10px;
            padding: 8px 18px;
            font-size: 13px;
            color: #7c4dff;
            font-weight: 500;
        }
        QPushButton#browseBtn:hover {
            background: rgba(124,77,255,0.22);
        }
        QPushButton#browseBtn:pressed {
            background: rgba(124,77,255,0.32);
        }

        /* ========== 标签 ========== */
        QLabel {
            color: #2d3436;
            background: transparent;
        }

        /* ========== 表单标签（加粗） ========== */
        QFormLayout QLabel {
            font-size: 13px;
            font-weight: 500;
            color: #444;
        }

        /* ========== 主要按钮 - 渐变紫蓝 ========== */
        QPushButton#startBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7c4dff, stop:1 #448aff);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 10px 32px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton#startBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #8e6cff, stop:1 #5c9aff);
        }
        QPushButton#startBtn:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6a3de6, stop:1 #2979ff);
        }
        QPushButton#startBtn:disabled {
            background: rgba(0,0,0,0.15);
            color: rgba(0,0,0,0.25);
        }

        /* ========== 停止按钮 - 红色渐变 ========== */
        QPushButton#stopBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff5252, stop:1 #ff6e40);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 10px 28px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton#stopBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff6e6e, stop:1 #ff8a6e);
        }
        QPushButton#stopBtn:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #e04848, stop:1 #e06030);
        }
        QPushButton#stopBtn:disabled {
            background: rgba(0,0,0,0.12);
            color: rgba(0,0,0,0.22);
        }

        /* ========== 重置按钮 - 透明轮廓 ========== */
        QPushButton#resetBtn {
            background: rgba(0,0,0,0.04);
            border: 1px solid rgba(0,0,0,0.12);
            border-radius: 10px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: 500;
            color: #555;
        }
        QPushButton#resetBtn:hover {
            background: rgba(0,0,0,0.08);
            border-color: rgba(0,0,0,0.2);
        }
        QPushButton#resetBtn:pressed {
            background: rgba(0,0,0,0.12);
        }
        QPushButton#resetBtn:disabled {
            background: rgba(0,0,0,0.04);
            color: rgba(0,0,0,0.2);
            border-color: rgba(0,0,0,0.06);
        }

        /* ========== 进度条 ========== */
        QProgressBar {
            background: rgba(0,0,0,0.06);
            border: none;
            border-radius: 8px;
            text-align: center;
            font-size: 12px;
            color: #666;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #7c4dff, stop:1 #448aff);
            border-radius: 8px;
        }

        /* ========== 横向滑块 ========== */
        QSlider::groove:horizontal {
            height: 6px;
            background: rgba(0,0,0,0.08);
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: white;
            border: 2px solid #7c4dff;
            width: 18px;
            height: 18px;
            margin: -7px 0;
            border-radius: 10px;
        }
        QSlider::handle:horizontal:hover {
            background: #f5f0ff;
            border-color: #6a3de6;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #7c4dff, stop:1 #448aff);
            border-radius: 3px;
        }

        /* ========== 复选框 ========== */
        QCheckBox {
            spacing: 8px;
            font-size: 13px;
            color: #444;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 5px;
            border: 2px solid rgba(0,0,0,0.18);
            background: rgba(255,255,255,0.7);
        }
        QCheckBox::indicator:hover {
            border-color: #7c4dff;
        }
        QCheckBox::indicator:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7c4dff, stop:1 #448aff);
            border: none;
        }

        /* ========== 日志区域 ========== */
        QTextEdit#logEdit {
            background: rgba(0,0,0,0.25);
            border: 1px solid rgba(255,255,255,0.12);
            border-radius: 10px;
            padding: 8px;
            color: #e8e8e8;
            font-size: 12px;
        }

        /* ========== 日志区滚动条 ========== */
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
        }
        QScrollBar::handle:vertical {
            background: rgba(0,0,0,0.2);
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(0,0,0,0.3);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        /* ========== 全局滚动条（其他区域） ========== */
        QScrollBar:horizontal {
            background: transparent;
            height: 6px;
        }
        QScrollBar::handle:horizontal {
            background: rgba(0,0,0,0.15);
            border-radius: 3px;
            min-width: 30px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )");
}

/*
 * ==============================================
 * 搭建用户界面 - 毛玻璃风格
 *
 * 布局结构：
 *   1. 顶部标题卡片（毛玻璃）
 *   2. API 配置卡片
 *   3. 文件配置卡片
 *   4. 实验提示词卡片
 *   5. 操作按钮行（渐变按钮）
 *   6. 进度条
 *   7. 运行日志卡片
 * ==============================================
 */
void MainWindow::setupUI()
{
    setWindowTitle("实验报告AI自动评语生成工具");
    resize(860, 780);
    setMinimumSize(700, 600);

    // ---- 全局毛玻璃样式 ----
    setupGlobalStyle();

    // ---- 中央部件 ----
    QWidget *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(28, 16, 28, 24);

    // ==============================
    // 1. 顶部标题卡片
    // ==============================
    QFrame *headerCard = createGlassCard();
    headerCard->setObjectName("headerCard");
    headerCard->setStyleSheet(
        "QFrame#headerCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 rgba(124,77,255,0.12), stop:1 rgba(68,138,255,0.08));"
        "  border: 1px solid rgba(124,77,255,0.15);"
        "  border-radius: 16px;"
        "}"
    );

    QVBoxLayout *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setSpacing(4);
    headerLayout->setContentsMargins(24, 16, 24, 16);

    QLabel *titleLabel = new QLabel("✦  实验报告AI自动评语生成工具");
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: 700; color: #2d3436; background: transparent;"
    );

    QLabel *subtitleLabel = new QLabel("批量读取 .docx  →  提取成绩  →  AI 生成评语  →  自动写入");
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet(
        "font-size: 12px; color: #636e72; background: transparent; letter-spacing: 1px;"
    );

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerCard);

    // ==============================
    // 2. API 配置卡片
    // ==============================
    QFrame *apiCard = createGlassCard();
    QVBoxLayout *apiCardLayout = new QVBoxLayout(apiCard);
    apiCardLayout->setSpacing(12);
    apiCardLayout->setContentsMargins(20, 16, 20, 20);

    QLabel *apiTitle = new QLabel("⚙  API 配置");
    apiTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #2d3436; background: transparent;");
    apiCardLayout->addWidget(apiTitle);

    // URL 行
    QHBoxLayout *urlRow = new QHBoxLayout();
    QLabel *urlLabel = new QLabel("接口地址");
    urlLabel->setFixedWidth(80);
    m_urlEdit = new QLineEdit();
    m_urlEdit->setPlaceholderText("https://api.deepseek.com");
    urlRow->addWidget(urlLabel);
    urlRow->addWidget(m_urlEdit, 1);
    apiCardLayout->addLayout(urlRow);

    // API Key 行
    QHBoxLayout *keyRow = new QHBoxLayout();
    QLabel *keyLabel = new QLabel("API Key");
    keyLabel->setFixedWidth(80);
    m_apiKeyEdit = new QLineEdit();
    m_apiKeyEdit->setPlaceholderText("请输入你的 DeepSeek API Key");
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    keyRow->addWidget(keyLabel);
    keyRow->addWidget(m_apiKeyEdit, 1);
    apiCardLayout->addLayout(keyRow);

    // Temperature 行
    QHBoxLayout *tempRow = new QHBoxLayout();
    QLabel *tempLabel = new QLabel("创造性");
    tempLabel->setFixedWidth(80);
    m_tempSlider = new QSlider(Qt::Horizontal);
    m_tempSlider->setRange(0, 100);
    m_tempSlider->setValue(70);
    m_tempLabel = new QLabel("0.70");
    m_tempLabel->setFixedWidth(36);
    m_tempLabel->setStyleSheet("font-weight: 600; color: #7c4dff;");
    tempRow->addWidget(tempLabel);
    tempRow->addWidget(m_tempSlider, 1);
    tempRow->addWidget(m_tempLabel);
    apiCardLayout->addLayout(tempRow);

    mainLayout->addWidget(apiCard);

    // ==============================
    // 3. 文件配置卡片
    // ==============================
    QFrame *fileCard = createGlassCard();
    QVBoxLayout *fileCardLayout = new QVBoxLayout(fileCard);
    fileCardLayout->setSpacing(10);
    fileCardLayout->setContentsMargins(20, 16, 20, 20);

    QLabel *fileTitle = new QLabel("📁  文件配置");
    fileTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #2d3436; background: transparent;");
    fileCardLayout->addWidget(fileTitle);

    QHBoxLayout *pathRow = new QHBoxLayout();
    m_pathEdit = new QLineEdit();
    m_pathEdit->setReadOnly(true);
    m_pathEdit->setPlaceholderText("请选择存放实验报告（.docx）的文件夹...");
    m_browseBtn = new QPushButton("📂  浏览");
    m_browseBtn->setObjectName("browseBtn");
    m_browseBtn->setFixedWidth(100);
    pathRow->addWidget(m_pathEdit, 1);
    pathRow->addWidget(m_browseBtn);
    fileCardLayout->addLayout(pathRow);

    m_backupCheck = new QCheckBox("处理完成后另存到子文件夹「已评语」（不修改原文件）");
    m_backupCheck->setChecked(true);

    fileCardLayout->addWidget(m_backupCheck);

    mainLayout->addWidget(fileCard);

    // ==============================
    // 4. 实验提示词卡片
    // ==============================
    QFrame *promptCard = createGlassCard();
    QVBoxLayout *promptCardLayout = new QVBoxLayout(promptCard);
    promptCardLayout->setSpacing(10);
    promptCardLayout->setContentsMargins(20, 16, 20, 20);

    QLabel *promptTitle = new QLabel("✏  实验提示词");
    promptTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #2d3436; background: transparent;");
    promptCardLayout->addWidget(promptTitle);

    m_promptEdit = new QTextEdit();
    m_promptEdit->setPlaceholderText(
        "请输入本次实验的要求和评分标准，例如：\n"
        "实验内容：用 Qt 实现一个计算器，要求支持加减乘除运算。\n"
        "评分标准：界面布局是否合理（30%）、功能是否完整（40%）、代码规范（30%）。\n\n"
        "AI 将根据学生成绩 + 你的提示词，自动生成针对性评语。"
    );
    m_promptEdit->setMinimumHeight(90);
    m_promptEdit->setMaximumHeight(130);
    promptCardLayout->addWidget(m_promptEdit);

    mainLayout->addWidget(promptCard);

    // ==============================
    // 5. 操作按钮行
    // ==============================
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_startBtn = new QPushButton("🚀  开始处理");
    m_startBtn->setObjectName("startBtn");
    m_startBtn->setFixedHeight(42);
    m_startBtn->setCursor(Qt::PointingHandCursor);

    m_stopBtn = new QPushButton("⏹  停止");
    m_stopBtn->setObjectName("stopBtn");
    m_stopBtn->setFixedHeight(42);
    m_stopBtn->setCursor(Qt::PointingHandCursor);

    m_resetBtn = new QPushButton("🔄  重置");
    m_resetBtn->setObjectName("resetBtn");
    m_resetBtn->setFixedHeight(42);
    m_resetBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_stopBtn);
    btnLayout->addWidget(m_resetBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // ==============================
    // 6. 进度条
    // ==============================
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(22);
    mainLayout->addWidget(m_progressBar);

    // ==============================
    // 7. 运行日志卡片
    // ==============================
    QFrame *logCard = createGlassCard();
    QVBoxLayout *logCardLayout = new QVBoxLayout(logCard);
    logCardLayout->setSpacing(10);
    logCardLayout->setContentsMargins(20, 16, 20, 20);

    QLabel *logTitle = new QLabel("📋  运行日志");
    logTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #2d3436; background: transparent;");
    logCardLayout->addWidget(logTitle);

    m_logEdit = new QTextEdit();
    m_logEdit->setObjectName("logEdit");
    m_logEdit->setReadOnly(true);
    QFont logFont("Consolas", 10);
    if (!QFontInfo(logFont).fixedPitch()) {
        logFont = QFont("Courier New", 10);
    }
    m_logEdit->setFont(logFont);
    logCardLayout->addWidget(m_logEdit, 1);

    mainLayout->addWidget(logCard, 1);

    // ==============================
    // 初始日志
    // ==============================
    appendLog("✦ 欢迎使用实验报告AI自动评语生成工具", 0);
    appendLog("请先配置 API 信息并选择实验报告文件夹", 0);

    // ==============================
    // Temperature 滑块实时数值
    // ==============================
    connect(m_tempSlider, &QSlider::valueChanged, this, [this](int value) {
        double temp = value / 100.0;
        m_tempLabel->setText(QString::number(temp, 'f', 2));
    });
}

/*
 * ==============================================
 * 连接信号与槽
 * ==============================================
 */
void MainWindow::setupConnections()
{
    connect(m_browseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseFolder);
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);
}

/*
 * ==============================================
 * 加载配置
 * ==============================================
 */
void MainWindow::loadSettings()
{
    m_urlEdit->setText(m_settings->value("deepseek/url", "https://api.deepseek.com").toString());
    m_apiKeyEdit->setText(m_settings->value("deepseek/apikey", "").toString());
    m_tempSlider->setValue(m_settings->value("deepseek/temperature", 70).toInt());
    m_backupCheck->setChecked(m_settings->value("folder/backup", true).toBool());
    QString lastPath = m_settings->value("folder/lastpath", "").toString();
    if (!lastPath.isEmpty()) {
        m_pathEdit->setText(lastPath);
    }
}

/*
 * ==============================================
 * 保存配置
 * ==============================================
 */
void MainWindow::saveSettings()
{
    m_settings->setValue("deepseek/url", m_urlEdit->text());
    m_settings->setValue("deepseek/apikey", m_apiKeyEdit->text());
    m_settings->setValue("deepseek/temperature", m_tempSlider->value());
    m_settings->setValue("folder/backup", m_backupCheck->isChecked());
    m_settings->setValue("folder/lastpath", m_pathEdit->text());
}

void MainWindow::setControlsEnabled(bool enabled)
{
    m_urlEdit->setEnabled(enabled);
    m_apiKeyEdit->setEnabled(enabled);
    m_pathEdit->setEnabled(enabled);
    m_browseBtn->setEnabled(enabled);
    m_promptEdit->setEnabled(enabled);
    m_tempSlider->setEnabled(enabled);
    m_backupCheck->setEnabled(enabled);
    m_startBtn->setEnabled(enabled);
    m_resetBtn->setEnabled(enabled);
    m_stopBtn->setEnabled(!enabled);
}

/*
 * ==============================================
 * 日志输出
 * type: 0=信息 1=成功 2=警告 3=错误
 * ==============================================
 */
void MainWindow::appendLog(const QString &msg, int type)
{
    QString color;
    switch (type) {
        case 0:  color = "#b0b0b0"; break;
        case 1:  color = "#69db7c"; break;
        case 2:  color = "#ffd43b"; break;
        case 3:  color = "#ff6b6b"; break;
        default: color = "#b0b0b0";
    }
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString html = QString("<span style='color:%1'>[%2] %3</span>")
                       .arg(color, timestamp, msg.toHtmlEscaped());
    m_logEdit->append(html);
    QScrollBar *scrollBar = m_logEdit->verticalScrollBar();
    if (scrollBar) {
        scrollBar->setValue(scrollBar->maximum());
    }
}

/*
 * ==============================================
 * 浏览文件夹
 * ==============================================
 */
void MainWindow::onBrowseFolder()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择实验报告文件夹", m_pathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_pathEdit->setText(QDir::toNativeSeparators(dir));
        appendLog("已选择文件夹：" + dir, 0);
    }
}

/*
 * ==============================================
 * 开始处理
 * ==============================================
 */
void MainWindow::onStart()
{
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        appendLog("请先输入 DeepSeek 接口地址", 3);
        m_urlEdit->setFocus();
        return;
    }
    QString apiKey = m_apiKeyEdit->text().trimmed();
    if (apiKey.isEmpty()) {
        appendLog("请先输入 API Key", 3);
        m_apiKeyEdit->setFocus();
        return;
    }
    QString folderPath = m_pathEdit->text().trimmed();
    if (folderPath.isEmpty()) {
        appendLog("请先选择实验报告文件夹", 3);
        m_browseBtn->setFocus();
        return;
    }
    if (!QDir(folderPath).exists()) {
        appendLog("所选文件夹不存在，请重新选择", 3);
        return;
    }
    QString prompt = m_promptEdit->toPlainText().trimmed();
    if (prompt.isEmpty()) {
        appendLog("请输入本次实验的提示词（实验要求/评分标准）", 3);
        m_promptEdit->setFocus();
        return;
    }

    saveSettings();

    m_logEdit->clear();
    m_progressBar->setValue(0);
    appendLog("═══════════════════════════════════════", 0);
    appendLog("开始批量处理实验报告...", 0);
    appendLog(QString("文件夹：%1").arg(folderPath), 0);

    if (m_worker) {
        if (m_worker->isRunning()) {
            m_worker->requestStop();
            m_worker->wait();
        }
        m_worker->deleteLater();
    }

    m_worker = new WorkerThread(this);
    double temp = m_tempSlider->value() / 100.0;
    m_worker->setParams(folderPath, url, apiKey, prompt, temp,
                        m_backupCheck->isChecked());

    connect(m_worker, &WorkerThread::logMessage,
            this, &MainWindow::onLogMessage);
    connect(m_worker, &WorkerThread::progressChanged,
            this, &MainWindow::onProgressChanged);
    connect(m_worker, &WorkerThread::finished,
            this, &MainWindow::onProcessingFinished);
    connect(m_worker, &WorkerThread::finished,
            m_worker, &QObject::deleteLater);
    connect(m_worker, &QThread::finished, this, [this]() {
        m_worker = nullptr;
    });

    setControlsEnabled(false);
    m_worker->start();
    appendLog("工作线程已启动，请等待处理结果...", 0);
}

void MainWindow::onStop()
{
    if (m_worker && m_worker->isRunning()) {
        m_worker->requestStop();
        appendLog("正在请求停止... 等待当前文件处理完毕", 2);
        m_stopBtn->setEnabled(false);
    }
}

void MainWindow::onReset()
{
    if (m_worker && m_worker->isRunning()) {
        appendLog("正在处理中，请先停止再重置", 3);
        return;
    }
    m_urlEdit->clear();
    m_apiKeyEdit->clear();
    m_pathEdit->clear();
    m_promptEdit->clear();
    m_tempSlider->setValue(70);
    m_backupCheck->setChecked(true);
    m_progressBar->setValue(0);
    m_logEdit->clear();
    appendLog("已重置所有输入", 0);
}

void MainWindow::onLogMessage(const QString &msg, int type)
{
    appendLog(msg, type);
}

void MainWindow::onProgressChanged(int current, int total)
{
    if (total > 0) {
        m_progressBar->setMaximum(total);
        m_progressBar->setValue(current);
        m_progressBar->setFormat(QString("进度：%1 / %2").arg(current).arg(total));
    }
}

void MainWindow::onProcessingFinished(int successCount, int failCount)
{
    setControlsEnabled(true);
    m_stopBtn->setEnabled(false);

    QString summary;
    if (failCount == 0 && successCount > 0) {
        summary = QString("全部处理成功！\n\n成功：%1 份").arg(successCount);
    } else {
        summary = QString("处理完成\n\n成功：%1 份\n失败：%2 份")
                      .arg(successCount).arg(failCount);
    }
    QMessageBox::information(this, "处理完成", summary);
    appendLog("所有任务已结束，可继续操作", 0);
}
