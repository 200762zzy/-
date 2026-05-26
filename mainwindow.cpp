#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QMessageBox>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QStatusBar>
#include <QHeaderView>
#include <QSizePolicy>
#include <QDir>
#include <QVBoxLayout>
#include <QItemSelectionModel>
#include <QFont>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("数据管理系统");
    this->resize(1000, 700);

    // 修复标题显示不全
    ui->label->setText("欢迎来到张智毅的数据管理系统");
    ui->label->setWordWrap(false);
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    ui->label->setMinimumHeight(48);
    ui->label->adjustSize();

    // 分页初始参数
    m_pageSize = 10;
    m_currentPage = 1;

    // 布局边距间距
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        mainLayout->setContentsMargins(24, 24, 24, 24);
        mainLayout->setSpacing(18);
    }

    // 现代UI样式表
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F8F9FC;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        }
        QLabel#label {
            font-size: 22px;
            font-weight: 600;
            color: #2D3748;
            padding: 12px 4px;
            background: transparent;
            min-height: 48px;
        }
        QLabel#label_page {
            font-size: 14px;
            color: #718096;
            padding: 0 16px;
        }
        QWidget#toolbar, QWidget#widget {
            background-color: #FFFFFF;
            border-radius: 16px;
            padding: 16px;
            border: none;
        }
        QLineEdit {
            border: none;
            border-radius: 12px;
            padding: 12px 16px;
            font-size: 14px;
            background-color: #F1F5F9;
            color: #2D3748;
            min-height: 20px;
        }
        QLineEdit:focus {
            background-color: #FFFFFF;
            border: 2px solid #4361EE;
            outline: none;
        }
        QPushButton {
            border-radius: 12px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: 500;
            border: none;
            min-width: 80px;
        }
        QPushButton#pushButton {
            background-color: #4361EE;
            color: white;
        }
        QPushButton#pushButton:hover { background-color: #3A56D6; }
        QPushButton#pushButton:pressed { background-color: #3146C2; }

        QPushButton#btn_add { background-color: #10B981; color: white; }
        QPushButton#btn_add:hover { background-color: #059669; }
        QPushButton#btn_delete { background-color: #EF4444; color: white; }
        QPushButton#btn_delete:hover { background-color: #DC2626; }
        QPushButton#btn_save { background-color: #F59E0B; color: white; }
        QPushButton#btn_save:hover { background-color: #D97706; }

        QPushButton#btn_pre, QPushButton#btn_next {
            background-color: #F1F5F9;
            color: #4361EE;
            border: 1px solid #E2E8F0;
        }
        QPushButton#btn_pre:hover, QPushButton#btn_next:hover {
            background-color: #DEEFFF;
        }
        QTableWidget {
            background-color: #FFFFFF;
            border-radius: 16px;
            gridline-color: transparent;
            font-size: 14px;
            color: #2D3748;
            padding: 8px;
            alternate-background-color: #FAFBFF;
        }
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4361EE, stop:1 #3A56D6);
            color: white;
            padding: 14px 8px;
            border: none;
            font-weight: 600;
            border-radius: 8px 8px 0 0;
        }
        QTableWidget::item {
            padding: 10px;
            border-bottom: 1px solid #F1F5F9;
        }
        QTableWidget::item:selected {
            background-color: #DEEFFF;
            color: #2D3748;
            border-radius: 8px;
        }
        QTableWidget::item:hover {
            background-color: #F8FAFF;
        }
        QStatusBar {
            background-color: #FFFFFF;
            border-top: 1px solid #E2E8F0;
            color: #718096;
            font-size: 13px;
            padding: 6px;
        }
    )");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 简易SQL解析
QStringList MainWindow::parseSql(QString sql, QString &tableName)
{
    QStringList fields;
    sql = sql.trimmed();
    if (!sql.startsWith("select", Qt::CaseInsensitive)) {
        QMessageBox::warning(this, "错误", "必须以select开头");
        return fields;
    }
    sql = sql.mid(7);
    QString partlower = sql.toLower();
    QStringList parts = partlower.split("from");
    if (parts.size() < 2) {
        QMessageBox::warning(this, "错误", "缺少from");
        return fields;
    }
    fields = parts[0].trimmed().toUpper().split(",");
    tableName = parts[1].trimmed();
    if (tableName.endsWith(";"))
        tableName.chop(1);
    return fields;
}

// 确定按钮
void MainWindow::on_pushButton_clicked()
{
    QString sql = ui->lineEdit->text();
    QString tableName;
    QStringList fields = parseSql(sql, tableName);
    loadCsvToTable(tableName, fields);
}

// 加载CSV到表格
bool MainWindow::loadCsvToTable(QString tableName, QStringList fields)
{
    if (tableName.isEmpty()) {
        QMessageBox::warning(this, "错误", "文件名不能为空");
        return false;
    }
    if (fields.isEmpty()) {
        QMessageBox::warning(this, "错误", "字段列表不能为空");
        return false;
    }

    QDir dir("D:/qt/Object/");
    if (!dir.exists())
        dir.mkpath(".");

    QFile file("D:/qt/Object/" + tableName + ".csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "找不到文件：" + tableName + ".csv");
        return false;
    }

    QTextStream in(&file);
    in.setCodec("GBK");

    QString headerLine = in.readLine().trimmed();
    if (headerLine.isEmpty()) {
        QMessageBox::warning(this, "错误", "CSV表头为空");
        file.close();
        return false;
    }

    QStringList header = headerLine.split(",", Qt::SkipEmptyParts);
    m_csvHeader = header;

    QList<int> indexs;
    for (const QString& f : fields) {
        int idx = header.indexOf(f.trimmed(), Qt::CaseInsensitive);
        if (idx < 0) {
            QMessageBox::warning(this, "错误", "字段不存在: " + f);
            file.close();
            return false;
        }
        indexs << idx;
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(fields.size());
    ui->tableWidget->setHorizontalHeaderLabels(fields);

    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidget->setAlternatingRowColors(true);

    m_allData.clear();
    m_currentPage = 1;
    int row = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        m_allData.append(line.split(",", Qt::KeepEmptyParts));
        row++;
    }
    file.close();

    showPageData();
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->statusbar->showMessage("当前表：" + tableName + ".csv");
    QMessageBox::information(this, "成功", QString("共加载 %1 行数据").arg(row));
    return true;
}

// 分页渲染数据
void MainWindow::showPageData()
{
    QString sql = ui->lineEdit->text();
    QString tableName;
    QStringList fields = parseSql(sql, tableName);
    if (fields.isEmpty() || m_csvHeader.isEmpty()) {
        ui->label_page->setText("无数据");
        ui->btn_pre->setEnabled(false);
        ui->btn_next->setEnabled(false);
        return;
    }

    QList<int> indexs;
    for (const QString& f : fields)
        indexs << m_csvHeader.indexOf(f.trimmed());

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(fields.size());
    ui->tableWidget->setHorizontalHeaderLabels(fields);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (m_allData.isEmpty()) {
        ui->label_page->setText("第 1 页 / 共 0 页");
        ui->btn_pre->setEnabled(false);
        ui->btn_next->setEnabled(false);
        return;
    }

    int start = (m_currentPage - 1) * m_pageSize;
    int end = start + m_pageSize;
    if (end > m_allData.size()) end = m_allData.size();

    int row = 0;
    for (int i = start; i < end; i++) {
        QStringList data = m_allData[i];
        ui->tableWidget->insertRow(row);
        for (int j = 0; j < fields.size(); j++) {
            int idx = indexs[j];
            QString val = (idx >= 0 && idx < data.size()) ? data[idx].trimmed() : "";
            ui->tableWidget->setItem(row, j, new QTableWidgetItem(val));
        }
        row++;
    }

    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    ui->label_page->setText(QString("第 %1 页 / 共 %2 页").arg(m_currentPage).arg(totalPage));
    ui->btn_pre->setEnabled(m_currentPage > 1);
    ui->btn_next->setEnabled(m_currentPage < totalPage);
}

// 表格编辑同步到内存
void MainWindow::syncTableToData()
{
    QString sql = ui->lineEdit->text();
    QString tableName;
    QStringList fields = parseSql(sql, tableName);
    if (fields.isEmpty() || m_csvHeader.isEmpty()) return;

    QList<int> indexs;
    for (const QString& f : fields)
        indexs << m_csvHeader.indexOf(f.trimmed());

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        int globalRow = (m_currentPage - 1) * m_pageSize + i;
        if (globalRow < 0 || globalRow >= m_allData.size()) continue;
        for (int j = 0; j < fields.size(); j++) {
            int csvCol = indexs[j];
            if (csvCol < 0) continue;
            QTableWidgetItem* item = ui->tableWidget->item(i, j);
            QString val = item ? item->text() : "";
            while (m_allData[globalRow].size() <= csvCol)
                m_allData[globalRow] << "";
            m_allData[globalRow][csvCol] = val;
        }
    }
}

// 上一页
void MainWindow::on_btn_pre_clicked()
{
    syncTableToData();
    if (m_currentPage > 1) {
        m_currentPage--;
        showPageData();
    }
}

// 下一页
void MainWindow::on_btn_next_clicked()
{
    syncTableToData();
    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    if (m_currentPage < totalPage) {
        m_currentPage++;
        showPageData();
    }
}

// 添加行
void MainWindow::on_btn_add_clicked()
{
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先加载数据表");
        return;
    }
    syncTableToData();
    QStringList emptyRow;
    for (int i = 0; i < m_csvHeader.size(); i++)
        emptyRow << "";
    m_allData.append(emptyRow);
    m_currentPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    showPageData();
}

// 删除行
void MainWindow::on_btn_delete_clicked()
{
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先加载数据表");
        return;
    }
    syncTableToData();

    QModelIndexList selectedRows = ui->tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要删除的行");
        return;
    }

    if (QMessageBox::question(this, "确认删除", "确定删除选中数据？") != QMessageBox::Yes)
        return;

    QList<int> globalRows;
    for (const auto& idx : selectedRows)
        globalRows.append((m_currentPage-1)*m_pageSize + idx.row());
    std::sort(globalRows.begin(), globalRows.end(), std::greater<int>());
    for (int r : globalRows)
        if (r >=0 && r < m_allData.size())
            m_allData.removeAt(r);

    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    if (m_currentPage > totalPage) m_currentPage = totalPage;
    if (m_currentPage < 1) m_currentPage = 1;
    showPageData();
}

// 保存CSV
void MainWindow::on_btn_save_clicked()
{
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "无数据可保存");
        return;
    }
    syncTableToData();
    QString tableName;
    parseSql(ui->lineEdit->text(), tableName);
    if (saveCsv(tableName)) {
        ui->statusbar->showMessage("保存成功！", 3000);
        QMessageBox::information(this, "成功", "数据已保存到文件");
    }
}

// 写入CSV文件
bool MainWindow::saveCsv(QString tableName)
{
    if (tableName.isEmpty()) return false;
    QDir dir("D:/qt/Object/");
    if (!dir.exists()) dir.mkpath(".");
    QFile file("D:/qt/Object/" + tableName + ".csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "保存失败！");
        return false;
    }
    QTextStream out(&file);
    out.setCodec("GBK");
    out << m_csvHeader.join(",") << "\n";
    for (const auto& row : m_allData)
        out << row.join(",") << "\n";
    file.close();
    return true;
}
