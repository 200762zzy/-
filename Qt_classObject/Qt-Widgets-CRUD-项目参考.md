# Qt Widgets CRUD 项目参考

## 一、项目整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                     classObject.pro                          │
│  QT += core gui widgets  │  CONFIG += c++11                 │
│  SOURCES += main.cpp mainwindow.cpp                         │
│  HEADERS += mainwindow.h                                    │
│  FORMS  += mainwindow.ui                                    │
└──────────────────────┬──────────────────────────────────────┘
                       │ qmake + make
┌──────────────────────▼──────────────────────────────────────┐
│                       main.cpp                               │
│  QApplication ←── setFont("Microsoft YaHei UI")              │
│       │                                                      │
│  MainWindow w ←── w.show()  ←── a.exec()                    │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                    mainwindow.h                              │
│  class MainWindow : QMainWindow                             │
│  {                                                           │
│      Q_OBJECT                                                │
│  ┌──────────── signals & slots ────────┐                    │
│  │ from .ui (auto-connect):            │                    │
│  │   on_pushButton_clicked()           │                    │
│  │   on_btn_pre_clicked()              │                    │
│  │   on_btn_next_clicked()             │                    │
│  │ from code (manual connect):         │                    │
│  │   on_btn_add_clicked()    ●         │                    │
│  │   on_btn_delete_clicked() ●── CRUD  │                    │
│  │   on_btn_save_clicked()   ●         │                    │
│  ├────────── data model ──────────────┤                    │
│  │ m_allData : QList<QStringList>      │  ← 全部数据行     │
│  │ m_csvHeader : QStringList           │  ← CSV 表头       │
│  │ m_currentPage, m_pageSize : int     │  ← 分页           │
│  ├────────── core methods ────────────┤                    │
│  │ parseSql()     → 解析 SELECT 语句   │                    │
│  │ loadCsvToTable() → 读 CSV → 填表格  │                    │
│  │ showPageData() → 渲染当前页         │                    │
│  │ syncTableToData() → 表格同步到内存  │                    │
│  │ saveCsv()       → 内存写回 CSV      │                    │
│  └─────────────────────────────────────┘                    │
└──────────────────────┬──────────────────────────────────────┘
                       │ setupUi(this)
┌──────────────────────▼──────────────────────────────────────┐
│                    mainwindow.ui                             │
│  QVBoxLayout (центральный виджет)                            │
│  ├─ QLabel "欢迎..."               ← header                 │
│  ├─ QWidget toolbar (code 创建)    ← [添加行][删除行]  [保存] │
│  ├─ QTableWidget                   ← 数据表格               │
│  ├─ QWidget pagination             ← [上一页] label [下一页] │
│  └─ QWidget sqlBar                 ← [lineEdit]   [确定]     │
└──────────────────────────────────────────────────────────────┘
```

---

## 二、核心代码七段

### 1. `.pro` 入门模板

```pro
QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
FORMS   += mainwindow.ui
```

**要点**：`FORMS` 告诉 qmake 要调用 uic 从 `.ui` 生成 `ui_mainwindow.h`。

---

### 2. `.ui` + C++ 双向绑定 — 两种信号连接方式

**方式 A：自动连接（.ui 内控件）**

按钮在 Designer 中命名为 `pushButton`，则 C++ 中：

```cpp
// 自动匹配：on_<objectName>_<signalName>
void MainWindow::on_pushButton_clicked()
{
    // 用户点"确定"时自动被调用
}
```

**方式 B：手动连接（代码创建的控件）**

```cpp
// 构造器中
QPushButton* btn_add = new QPushButton("添加行");
btn_add->setObjectName("btn_add");     // 只为样式表定位，不自动连接

connect(btn_add, &QPushButton::clicked, this, &MainWindow::on_btn_add_clicked);

// 槽函数
void MainWindow::on_btn_add_clicked()
{
    // 处理添加逻辑
}
```

**为什么需要这个区别**：`setupUi(this)` 只遍历 `.ui` 描述树做自动连接。手动 new 出来的控件必须自己 `connect()`。

---

### 3. 布局管理 — `insertWidget` 插入工具栏

```cpp
// 拿到主布局
QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
mainLayout->setContentsMargins(16, 16, 16, 16);
mainLayout->setSpacing(10);

// 创建工具栏
QWidget* toolbar = new QWidget;
QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
toolbarLayout->setContentsMargins(0, 0, 0, 0);

QPushButton* btn_add  = new QPushButton("添加行");
QPushButton* btn_del  = new QPushButton("删除行");
QPushButton* btn_save = new QPushButton("保存");

toolbarLayout->addWidget(btn_add);
toolbarLayout->addWidget(btn_del);
toolbarLayout->addStretch();            // 推右侧
toolbarLayout->addWidget(btn_save);

// 插入到 label 和 tableWidget 之间（索引 1）
mainLayout->insertWidget(1, toolbar);
```

**要点**：`addWidget` 追加到末尾，`insertWidget(index, widget)` 插到指定位置。

---

### 4. 数据模型模式 — 内存 <- 文件 -> 表格 三者同步

```
┌───────────────┐     loadCsvToTable()     ┌───────────────┐
│   CSV 文件     │ ───────────────────────► │   m_allData   │
│ C:/ClassObject │                          │ QList<String> │
│   /a.csv       │ ◄──── saveCsv() ──────── │   m_csvHeader │
└───────────────┘                          └───────┬───────┘
                                                   │ showPageData()
                                                   ▼
                                           ┌───────────────┐
                                           │  QTableWidget  │
                                           │  (当前页 ~10行)│
                                           └───────┬───────┘
                                                   │ syncTableToData()
                                                   ▼
                                           ┌───────────────┐
                                           │   m_allData    │
                                           │   (同步编辑)   │
                                           └───────────────┘
```

**加载**：

```cpp
bool MainWindow::loadCsvToTable(QString tableName, QStringList fields)
{
    QFile file("C:/ClassObject/" + tableName + ".csv");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QStringList header = in.readLine().split(",");
    m_csvHeader = header;                          ← 记住表头

    m_allData.clear();
    while (!in.atEnd()) {
        QStringList row = in.readLine().split(",");
        m_allData.append(row);
    }
    file.close();

    showPageData();
}
```

**同步当前页编辑到内存**（翻页前必须调用）：

```cpp
void MainWindow::syncTableToData()
{
    // 计算当前显示列对应 CSV 哪一列
    QList<int> colMap;
    for (auto& f : fields) colMap << m_csvHeader.indexOf(f);

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        int globalRow = (m_currentPage - 1) * m_pageSize + i;
        // globalRow → m_allData 中的真实行号
        for (int j = 0; j < fields.size(); j++) {
            int csvCol = colMap[j];
            QTableWidgetItem* item = ui->tableWidget->item(i, j);
            QString val = item ? item->text() : "";
            m_allData[globalRow][csvCol] = val;   ← 更新内存
        }
    }
}
```

**写回 CSV**：

```cpp
bool MainWindow::saveCsv(QString tableName)
{
    QFile file("C:/ClassObject/" + tableName + ".csv");
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&file);
    out.setCodec("UTF-8");

    out << m_csvHeader.join(",") << "\n";          // 表头
    for (auto& row : m_allData) {
        out << row.join(",") << "\n";              // 数据
    }
    file.close();
}
```

---

### 5. 分页模式

```cpp
void MainWindow::showPageData()
{
    int start = (m_currentPage - 1) * m_pageSize;  // 从第几条开始
    int end = start + m_pageSize;                   // 到第几条结束
    if (end > m_allData.size()) end = m_allData.size();

    ui->tableWidget->setRowCount(0);

    int row = 0;
    for (int i = start; i < end; i++) {
        ui->tableWidget->insertRow(row);
        for (int j = 0; j < fields.size(); j++) {
            ui->tableWidget->setItem(row, j,
                new QTableWidgetItem(m_allData[i][colMap[j]]));
        }
        row++;
    }

    int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;
    ui->label_page->setText(QString("第 %1 页 / 共 %2 页")
        .arg(m_currentPage).arg(totalPage));
    ui->btn_pre->setEnabled(m_currentPage > 1);
    ui->btn_next->setEnabled(m_currentPage < totalPage);
}

void MainWindow::on_btn_pre_clicked()
{
    syncTableToData();          // ← 先保存当前页编辑
    if (m_currentPage > 1) {
        m_currentPage--;
        showPageData();
    }
}
```

**必须记住**：翻页前先 `syncTableToData()`，否则用户在上一页改的字会丢。

---

### 6. 删除行模式 — 从后往前删

```cpp
void MainWindow::on_btn_delete_clicked()
{
    // 多选支持
    QModelIndexList selected = ui->tableWidget->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    // 确认对话框
    auto reply = QMessageBox::question(this, "确认删除",
        QString("删除 %1 行？").arg(selected.size()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    // 收集全局行号
    QList<int> globalRows;
    for (auto& idx : selected)
        globalRows.append((m_currentPage - 1) * m_pageSize + idx.row());

    // ★ 从大到小排序，从后往前删，索引才不会乱
    std::sort(globalRows.begin(), globalRows.end(), std::greater<int>());

    for (int row : globalRows)
        m_allData.removeAt(row);

    showPageData();
}
```

**关键技巧**：`std::sort(..., std::greater<int>())` 让最后一个元素先删，前面的行号就不会变化。

---

### 7. QSS 现代化主题 — 语义化按钮配色

```cpp
setStyleSheet(R"(
    /* 全局统一按钮 */
    QPushButton {
        background-color: #1976D2; color: white; border: none;
        border-radius: 4px; padding: 7px 18px; font-size: 13px;
    }
    QPushButton:hover   { background-color: #1565C0; }
    QPushButton:pressed { background-color: #0D47A1; }
    QPushButton:disabled{ background-color: #BDBDBD; }

    /* 按功能分色 — 用 objectName */
    QPushButton#btn_add    { background-color: #FF9800; }
    QPushButton#btn_delete { background-color: #f44336; }
    QPushButton#btn_save   { background-color: #43A047; }

    /* 表头 */
    QHeaderView::section {
        background-color: #1976D2; color: white; padding: 6px;
        border: none; font-weight: bold;
    }

    /* 表格 */
    QTableWidget {
        alternate-background-color: #f5f9ff;
        selection-background-color: #1976D2; selection-color: white;
        border: 1px solid #e0e0e0; border-radius: 4px;
    }

    /* 输入框聚焦 */
    QLineEdit:focus { border-color: #1976D2; }
)");
```

**配色规律**：蓝色 = 查询/默认，橙色 = 添加，红色 = 删除，绿色 = 保存。

---

## 三、做项目的步骤流

```
第1步：pro 文件 —— 声明 QT modules + sources/headers/forms
第2步：Designer 画 UI —— 摆好 QTableWidget + 分页 + 输入栏
第3步：写 header —— 声明槽函数 + 数据成员 + 私有方法
第4步：构造器里做三件事
   ├─ setupUi()
   ├─ 创建 CRUD 按钮 + connect() + insertWidget()
   └─ 贴 QSS 样式表
第5步：实现加载流程（点"确定"）
   └─ parseSql → loadCsvToTable → showPageData
第6步：实现编辑流程
   ├─ 设置表格 DoubleClicked 可编辑
   ├─ syncTableToData() 负责表格→内存
   └─ 翻页/保存前都调一次 syncTableToData()
第7步：实现保存流程
   └─ syncTableToData → saveCsv(写回文件)
第8步：实现删除 → 从后往前删
第9步：实现添加 → 追加空行 + 跳到末页
```

---

## 最终自检表

| 容易忘的事 | 后果 |
|-----------|------|
| 翻页前没调 `syncTableToData()` | 编辑内容丢失 |
| `std::sort(..., greater)` 从后往前删 | 索引错乱 → 删错行 |
| 代码创建的按钮忘了 `connect()` | 点按钮没反应 |
| `.ui` 改了但没重跑 qmake | `ui_mainwindow.h` 不会更新 |
| 保存前没从表格读回数据 | 编辑内容写不回文件 |
