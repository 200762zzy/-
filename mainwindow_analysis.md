# `mainwindow.cpp` 逐函数逐语句分析

---

## 一、文件头 — 包含指令 (Lines 1–15)

| 行号 | 语句 | 作用 |
|------|------|------|
| 1 | `#include "mainwindow.h"` | 引入主窗口类声明（成员变量、槽函数、私有方法） |
| 2 | `#include "ui_mainwindow.h"` | 引入由 `mainwindow.ui` 自动生成的 UI 类，提供 `ui->` 访问界面控件 |
| 3–14 | `#include <QTableWidget/QMessageBox/...>` | 引入 Qt 核心控件：表格、消息框、文件、文本流、状态栏、表头、布局、选择模型、字体等 |
| 15 | `#include <algorithm>` | 引入 `std::sort`，用于删除行时逆序排序 |

---

## 二、构造函数 `MainWindow::MainWindow` (Lines 17–149)

```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
```

- **`QMainWindow(parent)`** — 调用基类构造函数，将 `parent` 传给 QMainWindow，支持 Qt 的对象树父子管理。
- **`ui(new Ui::MainWindow)`** — 创建 UI 实例，该实例由 `.ui` 文件编译生成，包含所有控件的指针成员。

### 第21行：`ui->setupUi(this);`

**作用**：将 `.ui` 文件中设计的界面填充到当前窗口上。此调用后，`ui->tableWidget`、`ui->lineEdit`、`ui->label` 等指针才可用。

### 第22–23行：窗口基本属性

| 语句 | 作用 |
|------|------|
| `this->setWindowTitle("数据管理系统");` | 设置窗口标题栏文字 |
| `this->resize(1000, 700);` | 设置窗口初始尺寸 1000×700 像素 |

### 第26–31行：欢迎标签修复

| 语句 | 作用 |
|------|------|
| `ui->label->setText("欢迎来到张智毅的数据管理系统");` | 设置标签显示文本 |
| `ui->label->setWordWrap(false);` | 禁止自动换行，防止标题被截断成多行 |
| `ui->label->setAlignment(Qt::AlignCenter);` | 文字居中对齐 |
| `ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);` | 水平方向尽可能扩展，垂直方向至少扩展以容纳内容 |
| `ui->label->setMinimumHeight(48);` | 强制最小高度 48px，防止标题显示不全 |
| `ui->label->adjustSize();` | 根据内容重新计算并调整标签的推荐尺寸 |

### 第34–35行：分页初始参数

| 语句 | 作用 |
|------|------|
| `m_pageSize = 10;` | 每页显示 10 行数据 |
| `m_currentPage = 1;` | 当前页码从第 1 页开始 |

### 第38–42行：布局边距间距

```cpp
QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
```

- **`qobject_cast`** — 安全向下转型，将 `centralwidget` 的布局转换为 `QVBoxLayout*`，若类型不匹配返回 `nullptr`。

| 语句 | 作用 |
|------|------|
| `mainLayout->setContentsMargins(24, 24, 24, 24);` | 设置整体布局四周边距 24px |
| `mainLayout->setSpacing(18);` | 设置布局内控件之间的间距为 18px |

### 第45–149行：现代 UI 样式表 (QSS)

一段多行字符串（raw string literal `R"(...)"`）作为全局样式表。要点：

| QSS 选择器 | 作用 |
|------------|------|
| `QMainWindow` | 窗口背景色 `#F8F9FC`，默认字体 |
| `QLabel#label` | 标题标签：字号 22px、加粗、深色文字、最小高度 |
| `QLabel#label_page` | 页码标签：字号 14px、灰色文字 |
| `QWidget#toolbar, QWidget#widget` | 工具栏/容器：白色背景、圆角 16px、内边距 |
| `QLineEdit` | 输入框：浅灰背景、圆角 12px；**聚焦时**变白底、蓝色边框 |
| `QPushButton` | 通用按钮：圆角 12px、最小宽度 80px |
| `QPushButton#pushButton` | 确定按钮：蓝色 `#4361EE` 主题，带 hover/pressed 状态 |
| `QPushButton#btn_add` | 添加行按钮：绿色 |
| `QPushButton#btn_delete` | 删除行按钮：红色 |
| `QPushButton#btn_save` | 保存按钮：橙色 |
| `QPushButton#btn_pre/btn_next` | 翻页按钮：浅灰背景、蓝色文字、带边框 |
| `QTableWidget` | 表格：白底、圆角、隐藏网格线、交替行色 |
| `QHeaderView::section` | 表头：蓝紫渐变背景、白色文字、上圆角 |
| `QTableWidget::item` | 单元格：下边框浅灰分隔线 |
| `QTableWidget::item:selected` | 选中行：浅蓝背景、保持深色文字 |
| `QTableWidget::item:hover` | 悬停：极浅蓝背景 |
| `QStatusBar` | 状态栏：白底、上边框、灰色文字 |

---

## 三、析构函数 `~MainWindow` (Lines 152–155)

```cpp
MainWindow::~MainWindow()
{
    delete ui;
}
```

- 删除 `ui` 对象释放 UI 资源。由于 `ui` 是在初始化列表 `new` 出来的，必须在析构时手动 `delete`。

---

## 四、`parseSql` — 简易 SQL 解析 (Lines 157–178)

**功能**：将用户输入的 SQL 字符串（格式：`SELECT field1,field2 FROM tablename`）拆解为字段列表和表名。

| 行号 | 语句 | 作用 |
|------|------|------|
| 160 | `QStringList fields;` | 初始化空的字段列表 |
| 161 | `sql = sql.trimmed();` | 去除首尾空白字符 |
| 162–165 | `if (!sql.startsWith("select", Qt::CaseInsensitive))` | **不区分大小写**检查是否以 `select` 开头；否则弹出警告并返回空列表 |
| 166 | `sql = sql.mid(7);` | 去掉前 7 个字符（即 "select" 或 "SELECT"），得到剩余部分 |
| 167 | `QString partlower = sql.toLower();` | 剩余部分转小写，方便定位 `from` |
| 168 | `QStringList parts = partlower.split("from");` | 以 `from` 分割为两部分：前 = 字段列表，后 = 表名 |
| 169–172 | `if (parts.size() < 2)` | 分割后不到两部分说明缺少 `from`，报错 |
| 173 | `fields = parts[0].trimmed().toUpper().split(",");` | 字段部分去空白 → 转大写 → 按逗号拆分 |
| 174 | `tableName = parts[1].trimmed();` | 表名部分去空白 |
| 175–176 | `if (tableName.endsWith(";")) tableName.chop(1);` | 去除末尾可能的分号 |
| 177 | `return fields;` | 返回字段列表，`tableName` 通过引用传出 |

---

## 五、`on_pushButton_clicked` — 确定按钮 (Lines 180–187)

**功能**：点击"确定"按钮时触发，读取 SQL 输入框内容，解析后加载 CSV 数据。

| 行号 | 语句 | 作用 |
|------|------|------|
| 183 | `QString sql = ui->lineEdit->text();` | 从输入框获取 SQL 语句 |
| 184–185 | `QString tableName; QStringList fields = parseSql(sql, tableName);` | 调用解析函数，获取字段列表和表名 |
| 186 | `loadCsvToTable(tableName, fields);` | 根据表名和字段列表加载 CSV 并填充表格 |

---

## 六、`loadCsvToTable` — 加载 CSV 到表格 (Lines 189–261)

**功能**：打开 `C:/ClassObject/<tableName>.csv` 文件，读取指定字段列到 `QTableWidget`，并缓存全部数据到 `m_allData`。

### 前置校验

| 行号 | 语句 | 作用 |
|------|------|------|
| 192–195 | `if (tableName.isEmpty())` | 表名为空则警告并返回 false |
| 196–199 | `if (fields.isEmpty())` | 字段列表为空则警告并返回 false |

### 文件打开

| 行号 | 语句 | 作用 |
|------|------|------|
| 201–203 | `QDir dir("D:/qt/Object/"); if (!dir.exists()) dir.mkpath(".");` | 检查目录是否存在，不存在则递归创建 |
| 205–209 | `QFile file(...)` → `file.open(QIODevice::ReadOnly \| QIODevice::Text)` | 以只读文本模式打开文件；失败则警告 |
| 211–212 | `QTextStream in(&file); in.setCodec("GBK");` | 创建文本流并设置编码为 GBK（Windows 中文 CSV 常用编码） |

### 读取表头

| 行号 | 语句 | 作用 |
|------|------|------|
| 214 | `QString headerLine = in.readLine().trimmed();` | 读取第一行（表头）并去除两端空白 |
| 215–219 | `if (headerLine.isEmpty())` | 空表头则报错 |
| 221 | `QStringList header = headerLine.split(",", Qt::SkipEmptyParts);` | 按逗号拆分表头，跳过空字段 |
| 222 | `m_csvHeader = header;` | 保存完整表头，供后续写回 CSV 使用 |

### 映射用户字段到列索引

| 行号 | 语句 | 作用 |
|------|------|------|
| 224 | `QList<int> indexs;` | 存储每个查询字段在 CSV 表头中的列号 |
| 225–233 | `for (const QString& f : fields) { idx = header.indexOf(f.trimmed(), Qt::CaseInsensitive); ... indexs << idx; }` | 逐个查找字段，找不到则报错 |

### 设置表格列结构

| 行号 | 语句 | 作用 |
|------|------|------|
| 235 | `ui->tableWidget->clearContents();` | 清除表格原有内容（保留表头） |
| 236 | `ui->tableWidget->setRowCount(0);` | 行数归零 |
| 237 | `ui->tableWidget->setColumnCount(fields.size());` | 设置列数 = 查询字段数 |
| 238 | `ui->tableWidget->setHorizontalHeaderLabels(fields);` | 设置列头为用户查询的字段名 |

### 表格交互属性

| 行号 | 语句 | 作用 |
|------|------|------|
| 240 | `setEditTriggers(DoubleClicked \| EditKeyPressed)` | 双击或按编辑键可编辑单元格 |
| 241 | `setSelectionBehavior(SelectRows)` | 点击选中整行 |
| 242 | `setSelectionMode(ExtendedSelection)` | 支持多选（Ctrl/Shift） |
| 243 | `setAlternatingRowColors(true)` | 启用交替行背景色 |

### 读取数据行

| 行号 | 语句 | 作用 |
|------|------|------|
| 245 | `m_allData.clear();` | 清空旧数据 |
| 246 | `m_currentPage = 1;` | 重置到第一页 |
| 247–253 | `while (!in.atEnd()) { line = in.readLine().trimmed(); ... m_allData.append(line.split(...)); }` | 逐行读取 CSV，跳过空行；每行按逗号拆分为 `QStringList` 追加到 `m_allData` |
| 254 | `file.close();` | 关闭文件 |

### 收尾

| 行号 | 语句 | 作用 |
|------|------|------|
| 256 | `showPageData();` | 调用分页渲染函数显示第一页数据 |
| 257 | `horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);` | 列宽自动拉伸填满表格 |
| 258 | `ui->statusbar->showMessage("当前表：" + tableName + ".csv");` | 状态栏显示当前表名 |
| 259 | `QMessageBox::information(...)` | 弹出成功提示，显示加载行数 |

---

## 七、`showPageData` — 分页渲染数据 (Lines 263–313)

**功能**：根据当前页码和页大小，从 `m_allData` 中截取一段数据填充到表格。

| 行号 | 语句 | 作用 |
|------|------|------|
| 266–268 | 重新解析 SQL 获取字段列表和表名 | 确保字段与当前查询一致 |
| 269–274 | `if (fields.isEmpty() || m_csvHeader.isEmpty())` | 无数据时显示"无数据"并禁用翻页按钮 |
| 276–278 | 重新计算字段到 CSV 列索引 | 代码与 `loadCsvToTable` 中的映射逻辑相同 |
| 280–284 | 重置表格列结构 + 列宽拉伸 | 同 `loadCsvToTable` |
| 286–291 | `if (m_allData.isEmpty())` | 数据为空时显示"第 1 页 / 共 0 页"，禁用翻页 |
| 293–295 | `start = (m_currentPage-1)*m_pageSize; end = start + m_pageSize;` | 计算当前页在 `m_allData` 中的起止索引 |
| 297–307 | 双层 for 循环填充表格 | 外层遍历当前页数据行，内层遍历字段列，从 `m_allData[i]` 取值设置到 `QTableWidgetItem` |
| 309 | `int totalPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;` | 计算总页数（向上取整） |
| 310 | `label_page->setText(QString("第 %1 页 / 共 %2 页")...)` | 更新页码标签 |
| 311–312 | `btn_pre->setEnabled(m_currentPage > 1); btn_next->setEnabled(m_currentPage < totalPage);` | 控制翻页按钮可用状态 |

---

## 八、`syncTableToData` — 表格编辑同步到内存 (Lines 315–340)

**功能**：将用户在 `QTableWidget` 中的编辑内容写回 `m_allData`，确保在翻页/添加/删除/保存前数据一致。

| 行号 | 语句 | 作用 |
|------|------|------|
| 318–321 | 解析 SQL，若失败则直接返回 | 保护性检查 |
| 323–325 | 计算列索引映射 | 同前 |
| 327–339 | 双层 for 循环 | 外层遍历表格行，计算对应的全局行号 `globalRow`；内层遍历字段列，读取 `QTableWidgetItem` 的文本，写入 `m_allData[globalRow][csvCol]` |
| 335–336 | `while (m_allData[globalRow].size() <= csvCol) m_allData[globalRow] << "";` | 如果某行数据比 CSV 列数少，用空字符串补齐，防止越界 |

---

## 九、`on_btn_pre_clicked` — 上一页 (Lines 343–350)

| 行号 | 语句 | 作用 |
|------|------|------|
| 345 | `syncTableToData();` | 同步当前页编辑内容到内存 |
| 346–349 | `if (m_currentPage > 1) { m_currentPage--; showPageData(); }` | 页码递减后重新渲染 |

---

## 十、`on_btn_next_clicked` — 下一页 (Lines 353–361)

| 行号 | 语句 | 作用 |
|------|------|------|
| 355 | `syncTableToData();` | 同步编辑内容 |
| 356 | `int totalPage = ...` | 计算总页数 |
| 357–360 | `if (m_currentPage < totalPage) { m_currentPage++; showPageData(); }` | 页码递增后重新渲染 |

---

## 十一、`on_btn_add_clicked` — 添加行 (Lines 363–377)

| 行号 | 语句 | 作用 |
|------|------|------|
| 366–369 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示用户 |
| 370 | `syncTableToData();` | 先同步当前编辑 |
| 371–374 | 构造全空 `QStringList`（长度 = CSV 列数），追加到 `m_allData` | 添加一行空数据 |
| 375 | `m_currentPage = (m_allData.size() + m_pageSize - 1) / m_pageSize;` | 跳转到最后一页，确保用户能看到新行 |
| 376 | `showPageData();` | 重新渲染 |

---

## 十二、`on_btn_delete_clicked` — 删除行 (Lines 379–409)

| 行号 | 语句 | 作用 |
|------|------|------|
| 382–385 | 检查是否已加载数据 |
| 386 | `syncTableToData();` | 同步编辑 |
| 388 | `ui->tableWidget->selectionModel()->selectedRows();` | 获取用户在表格中选中的行集合 |
| 389–392 | 未选择行时警告 |
| 394–395 | `QMessageBox::question(...)` 确认对话框 | 防止误删除 |
| 397–399 | 遍历选中行，换算成 `m_allData` 中的全局行号 | `globalRow = (page-1)*pageSize + row` |
| 400 | `std::sort(globalRows.begin(), globalRows.end(), std::greater<int>());` | **逆序排序**，从后往前删，避免删除导致索引偏移 |
| 401–403 | `for (int r : globalRows) if (r >=0 && r < m_allData.size()) m_allData.removeAt(r);` | 逐个移除 |
| 405–407 | 调整当前页码：如果当前页超过总页数则回退，最少为 1 | 防止空页 |
| 408 | `showPageData();` | 重新渲染 |

---

## 十三、`on_btn_save_clicked` — 保存 CSV (Lines 411–425)

| 行号 | 语句 | 作用 |
|------|------|------|
| 414–417 | 无数据时提示 |
| 418 | `syncTableToData();` | 同步所有编辑到内存 |
| 419–420 | 解析 SQL 获取表名 | 得到要写入的文件名 |
| 421–424 | `if (saveCsv(tableName)) { statusBar 提示成功 + 弹窗 }` | 调用保存函数，成功则更新状态栏 |

---

## 十四、`saveCsv` — 写入 CSV 文件 (Lines 427–445)

| 行号 | 语句 | 作用 |
|------|------|------|
| 430 | `if (tableName.isEmpty()) return false;` | 保护 |
| 431–432 | 确保目录存在 | 同 `loadCsvToTable` |
| 433–437 | 以写模式打开文件 | `WriteOnly | Text`；失败则 `QMessageBox::critical` 弹出严重错误 |
| 438–439 | `QTextStream out(&file); out.setCodec("GBK");` | 创建输出流，编码设为 GBK |
| 440 | `out << m_csvHeader.join(",") << "\n";` | 写入 CSV 表头行 |
| 441–442 | `for (const auto& row : m_allData) out << row.join(",") << "\n";` | 遍历内存数据，每行用逗号拼接后写入文件 |
| 443 | `file.close();` | 关闭文件 |
| 444 | `return true;` | 返回成功 |

---

## 整体数据流总结

```
用户输入 SQL
    ↓
parseSql() ──→ fields (字段列表), tableName (表名)
    ↓
loadCsvToTable()
    ├── 打开 D:/qt/Object/<tableName>.csv
    ├── 读取 CSV 表头 → m_csvHeader
    ├── 映射 fields 到列索引
    ├── 全部数据 → m_allData（QList<QStringList>）
    └── showPageData() 渲染第一页
         ├── 计算 start/end
         └── 填充 QTableWidget

用户编辑表格 + 翻页/添加/删除/保存
    ├── syncTableToData()  ← 每次操作前调用
    │    将表格单元格写回 m_allData
    ├── 翻页: showPageData()
    ├── 添加: m_allData 追加空行 → showPageData()
    ├── 删除: 逆序移除 m_allData 元素 → showPageData()
    └── 保存: saveCsv()
          将 m_csvHeader + m_allData 写回文件
```
