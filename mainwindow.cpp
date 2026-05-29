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

// =============================================================================
// 静态辅助函数
// =============================================================================

// QStringList 大小写不敏感查找（QStringList::indexOf 不支持 CaseSensitivity）
static int indexOfCI(const QStringList &list, const QString &value)
{
    for (int i = 0; i < list.size(); ++i)
        if (list[i].compare(value, Qt::CaseInsensitive) == 0)
            return i;
    return -1;
}

// 去除字符串两端引号
static QString stripQuotes(const QString &s)
{
    if (s.length() >= 2 && s.startsWith('"') && s.endsWith('"'))
        return s.mid(1, s.length() - 2);
    return s;
}

// 按 "and" 分割 WHERE 条件（忽略引号内的 and）
static QStringList splitConditions(const QString &wherePart)
{
    QStringList result;
    QString current;
    bool inQuote = false;

    for (int i = 0; i < wherePart.length(); i++) {
        QChar c = wherePart[i];
        if (c == '"') {
            inQuote = !inQuote;
            current += c;
            continue;
        }
        if (!inQuote && i + 2 < wherePart.length()) {
            // 检测单词 "and"（前后必须有空白或边界）
            if (wherePart.mid(i, 3).toLower() == "and") {
                bool prevIsWord = (i > 0 && !wherePart[i - 1].isSpace());
                bool nextIsWord = (i + 3 < wherePart.length() && !wherePart[i + 3].isSpace());
                if (!prevIsWord && !nextIsWord) {
                    QString trimmed = current.trimmed();
                    if (!trimmed.isEmpty())
                        result << trimmed;
                    current.clear();
                    i += 2; // 跳过 "and"
                    continue;
                }
            }
        }
        current += c;
    }

    QString trimmed = current.trimmed();
    if (!trimmed.isEmpty())
        result << trimmed;

    return result;
}

// =============================================================================
// 构造 & 析构
// =============================================================================

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
    m_isMultiTable = false;

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

// =============================================================================
// SQL 解析与验证（核心）
// =============================================================================
// 解析结果写入：m_queryFields, m_queryTables, m_queryConditions, m_isMultiTable
// 返回 false 时 errorMsg 包含具体错误描述
// 检测的 5 种错误：
//   1. 关键字拼写错误（select）
//   2. from 后无合法表名 / 表不存在
//   3. 字段后多余逗号
//   4. 列名不存在
//   5. 字符串缺少引号
// =============================================================================

bool MainWindow::validateAndParseSql(const QString &sql, QString &errorMsg)
{
    // 重置查询状态
    m_queryFields.clear();
    m_queryTables.clear();
    m_queryConditions.clear();
    m_isMultiTable = false;
    m_joinHeader.clear();
    m_fieldIndexes.clear();

    // -------- 1. 规范化空白 + 检查 SELECT 关键字 --------
    QString s = sql.simplified().trimmed();
    if (s.isEmpty()) {
        errorMsg = "SQL 语句不能为空";
        return false;
    }

    // 检查关键字拼写（错误类型 1）
    if (!s.startsWith("select", Qt::CaseInsensitive)) {
        errorMsg = "SQL语法错误：关键字拼写错误，语句必须以 SELECT 开头";
        return false;
    }

    // 去掉 "SELECT"
    s = s.mid(6).trimmed();

    // -------- 2. 查找 FROM --------
    int fromIdx = -1;
    // 在 s 的小写版本中查找 "from"
    {
        QString lower = s.toLower();
        fromIdx = lower.indexOf("from");
        if (fromIdx < 0) {
            errorMsg = "SQL语法错误：缺少 FROM 关键字";
            return false;
        }
    }

    // -------- 3. 提取字段列表 --------
    QString fieldPart = s.left(fromIdx).trimmed();

    // 检查末尾逗号（错误类型 3）
    if (fieldPart.endsWith(",")) {
        errorMsg = "SQL语法错误：字段列表末尾有多余逗号";
        return false;
    }

    // 分割字段
    QStringList rawFields = fieldPart.split(",", Qt::SkipEmptyParts);
    for (const QString &f : rawFields) {
        QString tf = f.trimmed();
        if (!tf.isEmpty())
            m_queryFields << tf;
    }

    if (m_queryFields.isEmpty()) {
        errorMsg = "SQL语法错误：字段列表为空";
        return false;
    }

    bool isStar = (m_queryFields.size() == 1 && m_queryFields[0] == "*");

    // -------- 4. 提取 FROM 后内容 --------
    QString afterFrom = s.mid(fromIdx + 4).trimmed();
    QString afterFromLower = afterFrom.toLower();

    // 查找 WHERE
    int whereIdx = afterFromLower.indexOf("where");
    QString tablePart, wherePart;

    if (whereIdx >= 0) {
        tablePart = afterFrom.left(whereIdx).trimmed();
        wherePart = afterFrom.mid(whereIdx + 5).trimmed();
    } else {
        tablePart = afterFrom.trimmed();
    }

    // 去除末尾分号
    if (tablePart.endsWith(";"))  tablePart.chop(1);
    if (wherePart.endsWith(";"))  wherePart.chop(1);

    // -------- 5. 解析表名（支持别名） --------
    QStringList rawTables = tablePart.split(",", Qt::SkipEmptyParts);
    for (const QString &t : rawTables) {
        QString trimmed = t.trimmed();
        // 去除别名（如 "tab_class c" → "tab_class"）
        int spaceIdx = trimmed.indexOf(' ');
        if (spaceIdx > 0)
            trimmed = trimmed.left(spaceIdx).trimmed();
        if (!trimmed.isEmpty())
            m_queryTables << trimmed;
    }

    if (m_queryTables.isEmpty()) {
        errorMsg = "SQL语法错误：FROM 后缺少表名（错误类型 2）";
        return false;
    }

    // -------- 6. 检查表文件是否存在（错误类型 2） --------
    for (const QString &t : m_queryTables) {
        QFile file("D:/qt/Object/" + t + ".csv");
        if (!file.exists()) {
            errorMsg = "表不存在：" + t + "（错误类型 2）";
            return false;
        }
    }

    m_isMultiTable = (m_queryTables.size() > 1);

    // -------- 7. 解析 WHERE 条件 --------
    if (!wherePart.isEmpty()) {
        QStringList condStrs = splitConditions(wherePart);

        for (const QString &condStr : condStrs) {
            WhereCondition cond;

            // 确定操作符位置（>=、<=、<> 优先于单字符）
            int opPos = -1, opLen = 0;
            {
                int p;
                p = condStr.indexOf(">="); if (p >= 0 && (opPos < 0 || p < opPos)) { opPos = p; opLen = 2; }
                p = condStr.indexOf("<="); if (p >= 0 && (opPos < 0 || p < opPos)) { opPos = p; opLen = 2; }
                p = condStr.indexOf("<>"); if (p >= 0 && (opPos < 0 || p < opPos)) { opPos = p; opLen = 2; }
                p = condStr.indexOf('=');  if (p >= 0 && (opPos < 0 || p < opPos)) { opPos = p; opLen = 1; }
                p = condStr.indexOf('>');  if (p >= 0 && (opPos < 0 || p < opPos)) { opPos = p; opLen = 1; }
                p = condStr.indexOf('<');  if (p >= 0 && (opPos < 0 || p < opPos)) { opPos = p; opLen = 1; }
            }

            if (opPos < 0) {
                errorMsg = "SQL语法错误：无法解析 WHERE 条件：" + condStr;
                return false;
            }

            QString left  = condStr.left(opPos).trimmed();
            QString right = condStr.mid(opPos + opLen).trimmed();
            cond.op = condStr.mid(opPos, opLen);

            // 解析左侧（可能带表前缀 table.field）
            int dot = left.indexOf('.');
            if (dot >= 0) {
                cond.leftTable = left.left(dot).trimmed();
                cond.leftField = left.mid(dot + 1).trimmed();
            } else {
                cond.leftField = left;
            }

            // 解析右侧
            if (right.startsWith('"')) {
                // 引号字符串值
                cond.rightValue = stripQuotes(right);
                cond.isFieldCompare = false;
            } else if (right.contains('.')) {
                // 字段引用（如 tab_student.clazz_id）
                dot = right.indexOf('.');
                cond.rightTable = right.left(dot).trimmed();
                cond.rightField = right.mid(dot + 1).trimmed();
                cond.isFieldCompare = true;
            } else {
                // 可能是数值或未加引号的字符串
                bool isNumeric = false;
                right.toDouble(&isNumeric);
                if (isNumeric) {
                    cond.rightValue = right;
                    cond.isFieldCompare = false;
                } else {
                    // 检查是否为字段名引用（单表查询时允许）
                    // 但含有非数字字符且无引号 → 错误类型 5
                    errorMsg = "SQL语法错误：字符串值缺少引号：" + right + "（错误类型 5）";
                    return false;
                }
            }

            m_queryConditions << cond;
        }
    }

    // -------- 8. 检查字段名是否存在（错误类型 4）-------
    // 加载各表头用于验证
    if (isStar) {
        // * 表示全部字段，跳过列名检查，执行时展开
    } else {
        if (m_isMultiTable) {
            // 多表查询：字段必须带表前缀，分别检查
            for (const QString &f : m_queryFields) {
                int dot = f.indexOf('.');
                if (dot < 0) {
                    errorMsg = "SQL语法错误：多表查询中字段必须用 表名.字段名 格式（如 tab_class.name）";
                    return false;
                }
                QString table = f.left(dot).trimmed();
                QString field = f.mid(dot + 1).trimmed();

                // 表名是否在 FROM 列表中
                bool tableFound = false;
                for (const QString &t : m_queryTables) {
                    if (t.compare(table, Qt::CaseInsensitive) == 0) {
                        tableFound = true;
                        break;
                    }
                }
                if (!tableFound) {
                    errorMsg = "SQL语法错误：表名 " + table + " 不在 FROM 列表中";
                    return false;
                }

                // 检查字段是否存在（错误类型 4）
                QStringList hdr; QList<QStringList> dummy;
                if (loadCsvFile(table, hdr, dummy)) {
                    if (indexOfCI(hdr, field) < 0) {
                        errorMsg = "字段不存在：" + f + "（错误类型 4）";
                        return false;
                    }
                }
            }
        } else {
            // 单表查询
            QStringList hdr; QList<QStringList> dummy;
            if (loadCsvFile(m_queryTables[0], hdr, dummy)) {
                for (const QString &f : m_queryFields) {
                    if (indexOfCI(hdr, f) < 0) {
                        errorMsg = "字段不存在：" + f + "（错误类型 4）";
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

// =============================================================================
// 通用 CSV 加载
// =============================================================================

bool MainWindow::loadCsvFile(const QString &tableName, QStringList &header,
                               QList<QStringList> &data)
{
    QFile file("D:/qt/Object/" + tableName + ".csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    in.setCodec("GBK");

    QString headerLine = in.readLine().trimmed();
    if (headerLine.isEmpty()) {
        file.close();
        return false;
    }

    header = headerLine.split(",", Qt::SkipEmptyParts);
    data.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        data.append(line.split(",", Qt::KeepEmptyParts));
    }

    file.close();
    return true;
}

// =============================================================================
// 值比较
// =============================================================================

bool MainWindow::compareValues(const QString &l, const QString &op, const QString &r)
{
    QString lv = l.trimmed();
    QString rv = r.trimmed();

    // 先尝试数值比较
    bool lOk = false, rOk = false;
    double ld = lv.toDouble(&lOk);
    double rd = rv.toDouble(&rOk);

    if (lOk && rOk) {
        // 都是数值：用精确比较
        if (op == "=")  return ld == rd;
        if (op == "<>") return ld != rd;
        if (op == ">")  return ld > rd;
        if (op == "<")  return ld < rd;
        if (op == ">=") return ld >= rd;
        if (op == "<=") return ld <= rd;
    }

    // 字符串比较（忽略大小写 + 兼容 GBK 中文）
    int cmp = lv.compare(rv, Qt::CaseInsensitive);
    if (op == "=")  return cmp == 0;
    if (op == "<>") return cmp != 0;
    if (op == ">")  return cmp > 0;
    if (op == "<")  return cmp < 0;
    if (op == ">=") return cmp >= 0;
    if (op == "<=") return cmp <= 0;

    return false;
}

// =============================================================================
// 条件匹配（对单行数据检查所有 WHERE 条件）
// header: 单表时为 CSV 原表头，多表时为带表前缀的组合表头
// =============================================================================

bool MainWindow::checkConditions(const QStringList &row, const QStringList &header)
{
    if (m_queryConditions.isEmpty())
        return true;

    for (const WhereCondition &cond : m_queryConditions) {
        // 构建左侧查询键
        QString leftKey = cond.leftTable.isEmpty()
                          ? cond.leftField
                          : cond.leftTable + "." + cond.leftField;

        // 手动遍历实现大小写不敏感查找（替代可能失效的 indexOf）
        int leftIdx = -1;
        for (int i = 0; i < header.size(); ++i) {
            if (header[i].compare(leftKey, Qt::CaseInsensitive) == 0) {
                leftIdx = i;
                break;
            }
        }
        if (leftIdx < 0 || leftIdx >= row.size()) {
            return false; // 字段不存在或越界
        }
        QString leftVal = row[leftIdx].trimmed(); // 去除前后空白

        if (cond.isFieldCompare) {
            // 字段 vs 字段
            QString rightKey = cond.rightTable.isEmpty()
                               ? cond.rightField
                               : cond.rightTable + "." + cond.rightField;
            int rightIdx = -1;
            for (int i = 0; i < header.size(); ++i) {
                if (header[i].compare(rightKey, Qt::CaseInsensitive) == 0) {
                    rightIdx = i;
                    break;
                }
            }
            if (rightIdx < 0 || rightIdx >= row.size()) {
                return false;
            }
            QString rightVal = row[rightIdx].trimmed();
            if (!compareValues(leftVal, cond.op, rightVal))
                return false;
        } else {
            // 字段 vs 常量值
            QString rightVal = cond.rightValue.trimmed();
            if (!compareValues(leftVal, cond.op, rightVal))
                return false;
        }
    }

    return true;
}


// =============================================================================
// 单表查询执行
// =============================================================================

bool MainWindow::executeSingleTable()
{
    QString tableName = m_queryTables[0];

    // 加载 CSV（全部列）
    QStringList header;
    QList<QStringList> rawData;
    if (!loadCsvFile(tableName, header, rawData)) {
        QMessageBox::warning(this, "错误", "无法加载文件：" + tableName + ".csv");
        return false;
    }

    // 保存完整表头（用于写回）
    m_csvHeader = header;

    // * 展开
    if (m_queryFields.size() == 1 && m_queryFields[0] == "*") {
        m_queryFields = header;
    }

    // 字段到 CSV 列索引映射
    m_fieldIndexes.clear();
    for (const QString &f : m_queryFields) {
        m_fieldIndexes << indexOfCI(header, f);
    }

    // 按 WHERE 条件过滤，存储完整行
    m_allData.clear();
    for (const QStringList &row : rawData) {
        if (checkConditions(row, header)) {
            m_allData << row;
        }
    }

    // 设置表格
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(m_queryFields.size());
    ui->tableWidget->setHorizontalHeaderLabels(m_queryFields);
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked
                                     | QAbstractItemView::EditKeyPressed);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 启用 CRUD
    ui->btn_add->setEnabled(true);
    ui->btn_delete->setEnabled(true);
    ui->btn_save->setEnabled(true);

    m_currentPage = 1;
    showPageData();

    ui->statusbar->showMessage(QString("当前表：%1.csv  |  共 %2 行记录")
                               .arg(tableName).arg(m_allData.size()));
    return true;
}

// =============================================================================
// 多表连接查询执行
// =============================================================================

bool MainWindow::executeMultiTable()
{
    // 加载所有表
    QList<QStringList> allHeaders;
    QList<QList<QStringList>> allData;

    for (const QString &table : m_queryTables) {
        QStringList header;
        QList<QStringList> data;
        if (!loadCsvFile(table, header, data)) {
            QMessageBox::warning(this, "错误", "无法加载文件：" + table + ".csv");
            return false;
        }
        allHeaders << header;
        allData << data;
    }

    // 构建带表前缀的组合表头
    QStringList combinedHeader;
    for (int t = 0; t < m_queryTables.size(); t++) {
        for (const QString &h : allHeaders[t]) {
            combinedHeader << m_queryTables[t] + "." + h;
        }
    }

    // * 展开
    if (m_queryFields.size() == 1 && m_queryFields[0] == "*") {
        m_queryFields = combinedHeader;
    }

    // SELECT 字段 → 组合表头列索引
    QList<int> selectIndexes;
    for (const QString &f : m_queryFields) {
        int idx = indexOfCI(combinedHeader, f);
        if (idx < 0) {
            QMessageBox::warning(this, "错误", "无法找到字段：" + f);
            return false;
        }
        selectIndexes << idx;
    }

    // ---------- 笛卡尔积 + WHERE 过滤 ----------
    m_allData.clear();

    // 从第一张表开始
    QList<QStringList> product;
    for (const QStringList &row : allData[0]) {
        product << row;
    }

    // 依次与其余表做交叉连接
    for (int t = 1; t < allData.size(); t++) {
        QList<QStringList> newProduct;
        for (const QStringList &existingRow : product) {
            for (const QStringList &newRow : allData[t]) {
                QStringList combined = existingRow;
                combined << newRow;
                newProduct << combined;
            }
        }
        product = newProduct;

        // 内存保护
        if (product.size() > 1000000) {
            QMessageBox::warning(this, "警告", "连接结果超过 100 万行，请添加更多过滤条件");
            return false;
        }
    }

    // 逐行检查条件，提取 SELECT 列
    for (const QStringList &row : product) {
        if (checkConditions(row, combinedHeader)) {
            QStringList outRow;
            for (int idx : selectIndexes) {
                outRow << (idx < row.size() ? row[idx].trimmed() : "");
            }
            m_allData << outRow;
        }
    }

    // 保存组合表头（供 showPageData 使用）
    m_joinHeader = combinedHeader;
    m_csvHeader = m_queryFields;

    // 设置表格（只读）
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(m_queryFields.size());
    ui->tableWidget->setHorizontalHeaderLabels(m_queryFields);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 禁用 CRUD
    ui->btn_add->setEnabled(false);
    ui->btn_delete->setEnabled(false);
    ui->btn_save->setEnabled(false);

    m_currentPage = 1;
    showPageData();

    ui->statusbar->showMessage(QString("多表连接查询结果  |  共 %1 行记录")
                               .arg(m_allData.size()));
    return true;
}

// =============================================================================
// 确定按钮 — 入口
// =============================================================================

void MainWindow::on_pushButton_clicked()
{
    QString sql = ui->lineEdit->text();
    QString errorMsg;

    if (!validateAndParseSql(sql, errorMsg)) {
        QMessageBox::warning(this, "SQL语法错误", errorMsg);
        return;
    }

    if (m_isMultiTable)
        executeMultiTable();
    else
        executeSingleTable();
}

// =============================================================================
// 分页渲染
// =============================================================================

void MainWindow::showPageData()
{
    if (m_queryFields.isEmpty()) {
        ui->label_page->setText("无数据");
        ui->btn_pre->setEnabled(false);
        ui->btn_next->setEnabled(false);
        return;
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(m_queryFields.size());
    ui->tableWidget->setHorizontalHeaderLabels(m_queryFields);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (m_allData.isEmpty()) {
        ui->label_page->setText("第 1 页 / 共 0 页");
        ui->btn_pre->setEnabled(false);
        ui->btn_next->setEnabled(false);
        return;
    }

    int start = (m_currentPage - 1) * m_pageSize;
    int end = qMin(start + m_pageSize, m_allData.size());

    int row = 0;
    for (int i = start; i < end; i++) {
        const QStringList &dataRow = m_allData[i];
        ui->tableWidget->insertRow(row);

        if (m_isMultiTable) {
            // 多表：dataRow 已是 SELECT 列，直接显示
            for (int j = 0; j < m_queryFields.size() && j < dataRow.size(); j++) {
                ui->tableWidget->setItem(row, j, new QTableWidgetItem(dataRow[j]));
            }
        } else {
            // 单表：dataRow 是完整 CSV 行，按字段索引提取
            for (int j = 0; j < m_queryFields.size(); j++) {
                int idx = m_fieldIndexes[j];
                QString val = (idx >= 0 && idx < dataRow.size()) ? dataRow[idx].trimmed() : "";
                ui->tableWidget->setItem(row, j, new QTableWidgetItem(val));
            }
        }
        row++;
    }

    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    ui->label_page->setText(QString("第 %1 页 / 共 %2 页")
                            .arg(m_currentPage).arg(totalPage));
    ui->btn_pre->setEnabled(m_currentPage > 1);
    ui->btn_next->setEnabled(m_currentPage < totalPage);
}

// =============================================================================
// 编辑同步
// =============================================================================

void MainWindow::syncTableToData()
{
    // 多表查询只读，不执行同步
    if (m_isMultiTable || m_queryFields.isEmpty() || m_csvHeader.isEmpty())
        return;

    // 字段 → CSV 列索引
    QList<int> fieldToCsv;
    for (const QString &f : m_queryFields) {
        fieldToCsv << indexOfCI(m_csvHeader, f);
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        int globalRow = (m_currentPage - 1) * m_pageSize + i;
        if (globalRow < 0 || globalRow >= m_allData.size()) continue;
        for (int j = 0; j < m_queryFields.size(); j++) {
            int csvCol = fieldToCsv[j];
            if (csvCol < 0) continue;
            QTableWidgetItem* item = ui->tableWidget->item(i, j);
            QString val = item ? item->text() : "";
            while (m_allData[globalRow].size() <= csvCol)
                m_allData[globalRow] << "";
            m_allData[globalRow][csvCol] = val;
        }
    }
}

// =============================================================================
// 上一页
// =============================================================================

void MainWindow::on_btn_pre_clicked()
{
    syncTableToData();
    if (m_currentPage > 1) {
        m_currentPage--;
        showPageData();
    }
}

// =============================================================================
// 下一页
// =============================================================================

void MainWindow::on_btn_next_clicked()
{
    syncTableToData();
    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    if (m_currentPage < totalPage) {
        m_currentPage++;
        showPageData();
    }
}

// =============================================================================
// 添加行
// =============================================================================

void MainWindow::on_btn_add_clicked()
{
    if (m_isMultiTable) {
        QMessageBox::warning(this, "提示", "多表查询结果不支持编辑");
        return;
    }
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

// =============================================================================
// 删除行
// =============================================================================

void MainWindow::on_btn_delete_clicked()
{
    if (m_isMultiTable) {
        QMessageBox::warning(this, "提示", "多表查询结果不支持编辑");
        return;
    }
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
        globalRows.append((m_currentPage - 1) * m_pageSize + idx.row());
    std::sort(globalRows.begin(), globalRows.end(), std::greater<int>());
    for (int r : globalRows)
        if (r >= 0 && r < m_allData.size())
            m_allData.removeAt(r);

    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    if (m_currentPage > totalPage) m_currentPage = totalPage;
    if (m_currentPage < 1) m_currentPage = 1;
    showPageData();
}

// =============================================================================
// 保存 CSV
// =============================================================================

void MainWindow::on_btn_save_clicked()
{
    if (m_isMultiTable) {
        QMessageBox::warning(this, "提示", "多表查询结果不支持保存");
        return;
    }
    if (m_csvHeader.isEmpty()) {
        QMessageBox::warning(this, "提示", "无数据可保存");
        return;
    }
    syncTableToData();

    QString tableName = m_queryTables.isEmpty() ? "" : m_queryTables[0];
    if (saveCsv(tableName)) {
        ui->statusbar->showMessage("保存成功！", 3000);
        QMessageBox::information(this, "成功", "数据已保存到文件");
    }
}

// =============================================================================
// 写入 CSV 文件
// =============================================================================

bool MainWindow::saveCsv(const QString &tableName)
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
