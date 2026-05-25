// =============================================================================
// mainwindow.cpp — 主窗口类实现
// =============================================================================
// 文件结构（按阅读顺序）：
//   1. 构造器           → 加载 UI、创建工具栏、应用样式表
//   2. parseSql()       → SQL 解析（模拟）
//   3. loadCsvToTable() → CSV 加载（确定按钮触发）
//   4. showPageData()   → 分页渲染
//   5. syncTableToData()→ 表格→内存同步（核心方法，到处调用）
//   6. 翻页             → 上一页/下一页
//   7. 添加行/删除行/保存 → CRUD 操作
//   8. saveCsv()        → 写回 CSV
// =============================================================================

#include "mainwindow.h"         // 自己的头文件
#include "ui_mainwindow.h"      // ★ 由 qmake 从 mainwindow.ui 自动生成
                                //   里面定义了 Ui::MainWindow 类，包含所有控件指针

// Qt 控件头文件
#include <QTableWidget>         // 表格控件
#include <QMessageBox>          // 消息对话框（错误提示、确认对话框等）
#include <QStringList>          // 字符串列表
#include <QFile>                // 文件读写
#include <QTextStream>          // 文本流（方便读写字符串）
#include <QStatusBar>           // 状态栏
#include <QHeaderView>          // 表格的表头（设置列宽模式）
#include <QSizePolicy>          // 控件大小策略

// 额外需要的头文件（不是通过 .ui 自动引入的）
#include <QVBoxLayout>          // 垂直布局（拿到主布局调边距）
#include <QItemSelectionModel>  // 表格行选中状态管理
#include <QFont>                // 字体

#include <algorithm>            // std::sort（给删除行排序用）

// =============================================================================
//  构造器 — 程序启动时执行一次
// =============================================================================
//  执行顺序：
//    1. 初始化列表：给基类 QMainWindow 传 parent，创建 Ui::MainWindow 实例
//    2. setupUi：解析 mainwindow.ui，创建所有控件，建立 auto-connect
//    3. 布局微调：给主布局加间距
//    4. 创建 CRUD 按钮并手动连接信号
//    5. 应用全局样式表
// =============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)       // 调用基类构造器
    , ui(new Ui::MainWindow)    // 创建 UI 对象（内存由我们管理）
{
    // ---- 第 1 步：加载 UI 表单 --------------------------------------------
    ui->setupUi(this);          // ← 这个调用做了很多事：
                                //    - 根据 .ui XML 创建所有控件
                                //    - 设置它们的属性（文字、大小、样式）
                                //    - 自动连接 on_xxx_xxx 命名的槽函数

    // 让顶部标题标签可以水平拉伸（窗口变宽时它也跟着变宽）
    ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 分页初始值
    m_pageSize = 10;            // 每页 10 行
    m_currentPage = 1;          // 当前第 1 页

    // ---- 第 2 步：布局微调 ------------------------------------------------
    // ui->centralwidget 是 Qt Designer 自动生成的中心控件
    // 它内部已经有一个 QVBoxLayout（verticalLayout）
    // 我们拿到这个布局，调整边距和间距
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        mainLayout->setContentsMargins(16, 16, 16, 16); // 左 上 右 下 各 16px
        mainLayout->setSpacing(10);                      // 控件之间间距 10px
    }

    // ---- 第 3 步：全局样式表 ----------------------------------------------
    // setStyleSheet 是 QWidget 的方法，应用到自身及其所有子控件
    // 语法类似 CSS，但选择器有限：
    //   QPushButton           → 所有 QPushButton
    //   QPushButton#btn_add  → 对象名为 btn_add 的按钮
    //   :hover               → 鼠标悬停状态
    //   :pressed             → 按下状态
    //   :disabled            → 禁用状态
    setStyleSheet(R"(
        /* ----- 主窗口背景 ----- */
        QMainWindow {
            background-color: #f0f2f5;
        }

        /* ----- 顶部标题 ----- */
        QLabel#label {
            font-size: 18px; font-weight: bold; color: #1565C0;
            padding: 10px 0; background: transparent;
        }

        /* ----- 通用按钮（所有 QPushButton 的默认样式）----- */
        QPushButton {
            background-color: #1976D2; color: white; border: none;
            border-radius: 4px; padding: 7px 18px;
            font-size: 13px; font-weight: bold;
        }
        QPushButton:hover     { background-color: #1565C0; }
        QPushButton:pressed   { background-color: #0D47A1; }
        QPushButton:disabled  { background-color: #BDBDBD; color: #fff; }

        /* ----- 分色按钮（通过 objectName 精确选中）----- */
        QPushButton#btn_add    { background-color: #FF9800; }  /* 添加 = 橙色 */
        QPushButton#btn_add:hover    { background-color: #F57C00; }
        QPushButton#btn_delete { background-color: #f44336; }  /* 删除 = 红色 */
        QPushButton#btn_delete:hover { background-color: #D32F2F; }
        QPushButton#btn_save   { background-color: #43A047; }  /* 保存 = 绿色 */
        QPushButton#btn_save:hover   { background-color: #388E3C; }

        /* 翻页按钮 = 白底蓝框（区别于操作按钮） */
        QPushButton#btn_pre, QPushButton#btn_next {
            background-color: #fff; color: #1976D2; border: 1px solid #1976D2;
        }
        QPushButton#btn_pre:hover, QPushButton#btn_next:hover {
            background-color: #E3F2FD;
        }

        /* 确定按钮 = 蓝色主色 */
        QPushButton#pushButton {
            background-color: #1976D2; min-width: 80px;
        }

        /* ----- 表格样式 ----- */
        QTableWidget {
            background-color: #fff;
            alternate-background-color: #f5f9ff;  /* 斑马纹行色 */
            selection-background-color: #1976D2;   /* 选中行背景 */
            selection-color: white;                 /* 选中行文字 */
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            gridline-color: #e0e0e0;
            font-size: 13px;
        }
        QTableWidget::item:hover {
            background-color: #E3F2FD;            /* 悬停行高亮 */
        }
        QTableWidget::item:selected:hover {
            background-color: #1565C0;
        }

        /* 表头 */
        QHeaderView::section {
            background-color: #1976D2; color: white; padding: 6px;
            border: none; font-weight: bold; font-size: 13px;
        }

        /* ----- SQL 输入框 ----- */
        QLineEdit {
            border: 2px solid #e0e0e0; border-radius: 4px;
            padding: 8px 12px; font-size: 13px; background-color: #fff;
        }
        QLineEdit:focus {
            border-color: #1976D2;          /* 聚焦时蓝框 */
        }

        /* ----- 状态栏 ----- */
        QStatusBar {
            background-color: #fff;
            border-top: 1px solid #e0e0e0;
            color: #616161; font-size: 12px;
        }

        /* ----- 页码标签 ----- */
        QLabel#label_page {
            font-size: 13px; color: #616161; padding: 0 10px;
        }

        /* ----- 工具栏容器 ----- */
        QWidget#toolbar {
            background: transparent;
            border-bottom: 1px solid #e0e0e0;
            padding-bottom: 4px;
        }
    )");
}

// =============================================================================
//  析构器
// =============================================================================
MainWindow::~MainWindow()
{
    // ui 是用 new 创建的，必须手动释放
    // 但 ui 指向的所有子控件（表格、按钮等）由 Qt 对象树自动管理
    delete ui;
}

// =============================================================================
//  解析 SQL 语句（模拟）
// =============================================================================
//  功能：解析形如 "SELECT field1,field2 FROM tablename" 的语句
//  返回：字段列表 fields（如 ["FIELD1","FIELD2"]）
//  通过引用返回：表名 tableName（如 "students"）
//
//  ★ 这不是真正的 SQL：
//    - 不支持 WHERE、ORDER BY、JOIN 等
//    - 只是一个字符串解析器
//    - 实际做的事情：选一个 CSV 文件，只读取指定的列
// =============================================================================
QStringList MainWindow::parseSql(QString sql, QString &tableName)
{
    QStringList fields;
    sql = sql.trimmed();            // 去掉首尾空格

    // 必须以 select 开头（不区分大小写）
    if (!sql.startsWith("select", Qt::CaseInsensitive)) {
        QMessageBox::warning(this, "错误", "必须以select开头");
        return fields;              // 返回空列表表示失败
    }

    sql = sql.mid(7);                // 去掉前 7 个字符（"select "）
    QString partlower = sql.toLower();  // 转小写，方便拆分

    // 按 "from" 分割
    // parts[0] = 字段列表（如 "field1,field2"）
    // parts[1] = 表名（如 "students"）
    QStringList parts = partlower.split("from");
    if (parts.size() < 2) {
        QMessageBox::warning(this, "错误", "缺少from");
        return fields;
    }

    // 处理字段列表：去掉空格 → 转大写 → 按逗号拆成列表
    fields = parts[0].trimmed().toUpper().split(",");

    // 处理表名
    tableName = parts[1].trimmed();
    if (tableName.endsWith(";")) {
        tableName.chop(1);          // 去掉末尾分号（如果有）
    }

    return fields;
}

// =============================================================================
//  用户点击"确定"按钮
// =============================================================================
//  流程：读取 SQL 输入框 → 解析 → 加载对应 CSV 到表格
//  ★ 注意：这会丢弃所有未保存的修改，因为 reload 重新读了文件
// =============================================================================
void MainWindow::on_pushButton_clicked()
{
    QString sql = ui->lineEdit->text();         // 从输入框获取 SQL 语句
    QString tableName;
    QStringList fields = parseSql(sql, tableName); // 解析
    loadCsvToTable(tableName, fields);           // 加载数据
}

// =============================================================================
//  加载 CSV 数据到表格
// =============================================================================
//  做的事情：
//    1. 打开 C:/ClassObject/<表名>.csv
//    2. 读取第一行（表头），验证用户要求的字段是否存在
//    3. 把全部数据行读入 m_allData
//    4. 保存表头到 m_csvHeader（供后续编辑和写回使用）
//    5. 调用 showPageData() 显示第一页
// =============================================================================
bool MainWindow::loadCsvToTable(QString tableName, QStringList fields)
{
    // ---- 参数校验 ----------------------------------------------------------
    if (tableName.isEmpty()) {
        QMessageBox::warning(this, "错误", "文件名不能为空");
        return false;
    }
    if (fields.isEmpty()) {
        QMessageBox::warning(this, "错误", "字段列表不能为空");
        return false;
    }

    // ---- 打开文件 ----------------------------------------------------------
    // ★ 路径硬编码了：数据文件必须放在 C:/ClassObject/ 目录下
    QFile file("C:/ClassObject/" + tableName + ".csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误",
            "找不到文件或文件无法打开：" + tableName + ".csv");
        return false;
    }

    // 用文本流读取文件，指定 UTF-8 编码（Qt5 API）
    QTextStream in(&file);
    in.setCodec("UTF-8");

    // ---- 读表头 ------------------------------------------------------------
    QString headerLine = in.readLine().trimmed();
    if (headerLine.isEmpty()) {
        QMessageBox::warning(this, "错误", "CSV文件表头为空");
        file.close();
        return false;
    }
    // 按逗号拆分成字段名列表
    // Qt::SkipEmptyParts 跳过空项（连续的逗号不会产生空字符串）
    QStringList header = headerLine.split(",", Qt::SkipEmptyParts);

    // ★ 把表头保存到成员变量，写回 CSV 时需要它
    m_csvHeader = header;

    // ---- 验证用户要求的字段是否存在于 CSV 表头中 -------------------------
    QList<int> indexs;    // 记录每个要求字段在表头中的下标
    for (const QString& f : fields) {
        QString field = f.trimmed();
        // indexOf 搜索字符串在列表中的位置
        // Qt::CaseInsensitive 忽略大小写
        int idx = header.indexOf(field, Qt::CaseInsensitive);
        if (idx < 0) {
            QMessageBox::warning(this, "错误", "字段不存在: " + field);
            file.close();
            return false;   // 有一个字段不存在就返回失败
        }
        indexs << idx;      // 记录下标
    }

    // ---- 清空表格 ----------------------------------------------------------
    ui->tableWidget->clearContents();    // 清空单元格内容
    ui->tableWidget->setRowCount(0);     // 行数设为 0
    ui->tableWidget->setColumnCount(fields.size());  // 列数 = 查询字段数
    ui->tableWidget->setHorizontalHeaderLabels(fields); // 设置列标题

    // ---- 设置表格的交互行为 ------------------------------------------------
    // 允许双击或按 F2 编辑单元格（原先是 NoEditTriggers 只读）
    ui->tableWidget->setEditTriggers(
        QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);    // 按行选中
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection); // 支持多选
    ui->tableWidget->setAlternatingRowColors(true);  // 斑马纹

    // ---- 读取数据行 --------------------------------------------------------
    int row = 0;                // 行计数器
    m_allData.clear();          // 清空旧数据
    m_currentPage = 1;          // 重置到第一页

    while (!in.atEnd()) {       // 逐行读取直到文件末尾
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;   // 跳过空行
        QStringList data = line.split(",", Qt::KeepEmptyParts);
        m_allData.append(data);         // 每行是一个 QStringList
        row++;
    }
    file.close();               // 关闭文件

    // ---- 显示第一页 --------------------------------------------------------
    showPageData();

    // 设置列宽为"拉伸模式"：所有列平均分配表格宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 在状态栏显示当前表名
    ui->statusbar->showMessage("当前表：" + tableName + ".csv");

    // 弹出成功提示
    QMessageBox::information(this, "成功", QString("共加载 %1 行数据").arg(row));
    return true;
}

// =============================================================================
//  显示当前页数据到表格
// =============================================================================
//  核心计算：
//    start = (当前页-1) × 每页行数
//    end   = start + 每页行数（不超过数据总数）
//  例如：第 2 页，每页 10 行
//    start = (2-1) × 10 = 10
//    end   = 10 + 10 = 20（显示第 10~19 行）
// =============================================================================
void MainWindow::showPageData()
{
    // 重新解析 SQL（获取当前查询的字段列表和表名）
    QString sql = ui->lineEdit->text();
    QString tableName;
    QStringList fields = parseSql(sql, tableName);

    // 如果无数据，显示"无数据"并禁用翻页按钮
    if (fields.isEmpty() || m_csvHeader.isEmpty()) {
        ui->label_page->setText("无数据");
        ui->btn_pre->setEnabled(false);
        ui->btn_next->setEnabled(false);
        return;
    }

    // 计算当前显示的列对应 CSV 的哪一列
    // indexs[j] = 表格第 j 列对应 CSV 文件的第几列
    // 例如：SELECT NAME,AGE FROM students
    //       表头是 ID,NAME,AGE,SCORE
    //       indexs = [1, 2]（NAME在第1列，AGE在第2列）
    QList<int> indexs;
    for (const QString& f : fields) {
        QString field = f.trimmed();
        indexs << m_csvHeader.indexOf(field);
    }

    // 清空表格并重新设置列
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(fields.size());
    ui->tableWidget->setHorizontalHeaderLabels(fields);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 如果没有数据，显示空页信息
    if (m_allData.isEmpty()) {
        ui->label_page->setText("第 1 页 / 共 0 页");
        ui->btn_pre->setEnabled(false);
        ui->btn_next->setEnabled(false);
        return;
    }

    // ---- 分页计算 ----------------------------------------------------------
    int start = (m_currentPage - 1) * m_pageSize;  // 起始行（在 m_allData 中的下标）
    int end = start + m_pageSize;                   // 结束行
    if (end > m_allData.size()) {
        end = m_allData.size();                     // 最后一页可能不满
    }

    // 逐行插入数据到表格
    int row = 0;    // 表格中的行号（从 0 开始）
    for (int i = start; i < end; i++) {
        QStringList data = m_allData[i];            // 从全局数据中取一行
        ui->tableWidget->insertRow(row);           // 在表格中插入新行

        for (int j = 0; j < fields.size(); j++) {
            int idx = indexs[j];                    // 映射到 CSV 的列
            // 取单元格值，如果下标越界则取空字符串
            QString val = (idx >= 0 && idx < data.size()) ? data[idx].trimmed() : "";
            // 创建单元格并设置到表格
            ui->tableWidget->setItem(row, j, new QTableWidgetItem(val));
        }
        row++;
    }

    // ---- 更新页码显示 ------------------------------------------------------
    // 总页数 = 向上取整(总行数 / 每页行数)
    // 公式: (总行数 + 每页行数 - 1) / 每页行数
    // 例如: 25 行 ÷ 10 = 2.5 → (25+9)/10 = 3 页
    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    ui->label_page->setText(
        QString("第 %1 页 / 共 %2 页").arg(m_currentPage).arg(totalPage));

    // 控制翻页按钮是否可用
    ui->btn_pre->setEnabled(m_currentPage > 1);         // 第一页时不能上一页
    ui->btn_next->setEnabled(m_currentPage < totalPage); // 最后一页时不能下一页
}

// =============================================================================
//  同步表格编辑到内存（★ 关键方法 ★）
// =============================================================================
//  为什么要这个方法？
//    用户在表格中修改数据后，数据只存在于 QTableWidget 的单元格里，
//    还没有写回 m_allData。如果不先同步就翻页，修改就会丢失。
//
//  什么时候调用？
//    - 翻页前（上一页/下一页）
//    - 添加行前（确保当前页编辑已保存）
//    - 删除行前
//    - 保存前
// =============================================================================
void MainWindow::syncTableToData()
{
    // 获取当前 SQL 的字段列表
    QString sql = ui->lineEdit->text();
    QString tableName;
    QStringList fields = parseSql(sql, tableName);
    if (fields.isEmpty() || m_csvHeader.isEmpty()) return;

    // 计算列映射（同 showPageData 中的逻辑）
    QList<int> indexs;
    for (const QString& f : fields) {
        indexs << m_csvHeader.indexOf(f.trimmed());
    }

    // 遍历表格的每一行 → 更新 m_allData 中对应的那一行
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        // 计算这一行在 m_allData 中的全局行号
        int globalRow = (m_currentPage - 1) * m_pageSize + i;
        if (globalRow < 0 || globalRow >= m_allData.size()) continue;

        // 遍历每一列
        for (int j = 0; j < fields.size(); j++) {
            int csvCol = indexs[j];     // 映射到 CSV 的列
            if (csvCol < 0) continue;   // 无效列跳过

            // 从表格中读取值
            QTableWidgetItem* item = ui->tableWidget->item(i, j);
            QString val = item ? item->text() : "";

            // 确保 m_allData 这一行有足够的列（添加空行时可能列数不足）
            while (m_allData[globalRow].size() <= csvCol) {
                m_allData[globalRow] << "";
            }

            // 更新内存中的值
            m_allData[globalRow][csvCol] = val;
        }
    }
}

// =============================================================================
//  上一页
// =============================================================================
void MainWindow::on_btn_pre_clicked()
{
    syncTableToData();          // ★ 先保存当前页编辑
    if (m_currentPage > 1) {
        m_currentPage--;        // 页码减一
        showPageData();         // 重新渲染
    }
}

// =============================================================================
//  下一页
// =============================================================================
void MainWindow::on_btn_next_clicked()
{
    syncTableToData();          // ★ 先保存当前页编辑
    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    if (m_currentPage < totalPage) {
        m_currentPage++;        // 页码加一
        showPageData();
    }
}

// =============================================================================
//  添加行
// =============================================================================
//  逻辑：
//    1. 同步当前页面编辑
//    2. 创建一个全是空字符串的行（列数与 CSV 表头一致）
//    3. 追加到 m_allData 末尾
//    4. 跳转到最后一页，让用户看到新行
// =============================================================================
void MainWindow::on_btn_add_clicked()
{
    // 必须先加载过数据表
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先加载数据表");
        return;
    }

    syncTableToData();          // ★ 保存当前编辑

    // 创建一个空行：列数 = 表头字段数
    QStringList emptyRow;
    for (int i = 0; i < m_csvHeader.size(); i++) {
        emptyRow << "";
    }
    m_allData.append(emptyRow); // 追加到最后

    // 跳转到最后一页
    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    m_currentPage = totalPage;
    showPageData();
}

// =============================================================================
//  删除行
// =============================================================================
//  逻辑：
//    1. 检查是否加载过数据
//    2. 同步当前编辑
//    3. 获取选中行 → 计算全局行号 → 确认对话框
//    4. ★ 从后往前删除（关键技巧）
//    5. 调整页码（如果当前页变成空页就回退一页）
// =============================================================================
void MainWindow::on_btn_delete_clicked()
{
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先加载数据表");
        return;
    }

    syncTableToData();

    // 获取所有选中的行
    QItemSelectionModel* selection = ui->tableWidget->selectionModel();
    QModelIndexList selectedRows = selection->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的行");
        return;
    }

    // 弹出确认框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除选中的 %1 行数据吗？").arg(selectedRows.size()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    // ---- ★ 关键：从后往前删除 ----------------------------------------------
    // 如果从前往后删，每删一行后面的行号都变了，后面的索引就失效了
    // 例如：要删第 2、5 行
    //  从前往后：删第 2 行 → 原第 5 行变成第 4 行 → 删错
    //  从后往前：删第 5 行 → 删第 2 行（索引不变）→ 正确

    // 先把选中行的全局行号收集起来
    QList<int> globalRows;
    for (const QModelIndex& index : selectedRows) {
        globalRows.append((m_currentPage - 1) * m_pageSize + index.row());
    }

    // 从大到小排序（std::greater<int>() = 降序）
    std::sort(globalRows.begin(), globalRows.end(), std::greater<int>());

    // 逐个删除（现在是从后往前了）
    for (int row : globalRows) {
        if (row >= 0 && row < m_allData.size()) {
            m_allData.removeAt(row);
        }
    }

    // 调整页码：如果当前页已经空了，回退到上一页
    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    if (m_currentPage > totalPage) m_currentPage = totalPage;
    if (m_currentPage < 1) m_currentPage = 1;  // 如果全删光了

    showPageData();
}

// =============================================================================
//  保存
// =============================================================================
//  流程：
//    1. 同步当前编辑到内存
//    2. 从 SQL 解析出表名
//    3. 调用 saveCsv() 写回文件
//    4. 提示保存成功
// =============================================================================
void MainWindow::on_btn_save_clicked()
{
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "无数据可保存");
        return;
    }

    syncTableToData();                      // ★ 先把当前编辑同步到内存

    // 从 SQL 中解析出表名
    QString sql = ui->lineEdit->text();
    QString tableName;
    parseSql(sql, tableName);
    if (tableName.isEmpty()) {
        QMessageBox::warning(this, "提示", "无法解析表名");
        return;
    }

    // 写回 CSV
    if (saveCsv(tableName)) {
        ui->statusbar->showMessage("已保存到 " + tableName + ".csv", 3000);
        QMessageBox::information(this, "成功", "数据已保存到 " + tableName + ".csv");
    }
}

// =============================================================================
//  写回 CSV 文件
// =============================================================================
//  将 m_allData 和 m_csvHeader 写回到 C:/ClassObject/<表名>.csv
//  文件格式与读取时一致：
//    第一行：表头（逗号分隔）
//    后续行：数据（逗号分隔）
// =============================================================================
bool MainWindow::saveCsv(QString tableName)
{
    if (tableName.isEmpty()) return false;

    // ★ 以 WriteOnly 模式打开会覆盖原有文件
    QFile file("C:/ClassObject/" + tableName + ".csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法写入文件：" + tableName + ".csv");
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 写入表头
    out << m_csvHeader.join(",") << "\n";

    // 逐行写入数据
    for (const QStringList& row : m_allData) {
        out << row.join(",") << "\n";
    }

    file.close();
    return true;
}
