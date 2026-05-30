# `mainwindow.cpp` 逐函数逐语句分析

---

## 一、文件头 — 包含指令 (Lines 1–15)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 1 | `#include "mainwindow.h"` | 引入主窗口类声明（`WhereCondition` 结构体、成员变量与私有方法） | 该文件包含 `Q_OBJECT` 宏 → qmake 自动调用 MOC 生成 `moc_mainwindow.cpp`，提供信号槽、`qobject_cast`、`className()` 等元对象支持 |
| 2 | `#include "ui_mainwindow.h"` | 引入由 `mainwindow.ui` 自动生成的 UI 类，提供 `ui->` 访问界面控件 | `uic` 工具编译 `mainwindow.ui` 生成该类，内部调用 `QMetaObject::connectSlotsByName()` 实现 `on_` 自动连接 |
| 3 | `#include <QTableWidget>` | 引入表格控件 | `QTableWidget` — Qt 高级表格控件，基于 Model/View 架构（Item-based）。提供 `setItem()`、`insertRow()`、`item()` 等 API |
| 4 | `#include <QMessageBox>` | 引入消息弹窗 | `QMessageBox` — 模态对话框，提供静态方法 `warning/information/question/critical` |
| 5 | `#include <QStringList>` | 引入字符串列表 | `QStringList` — `QList<QString>` 子类，增加 `join()`、`filter()` 等便捷方法 |
| 6 | `#include <QFile>` | 引入文件操作 | `QFile` — 继承 `QIODevice`，支持本地文件读写。核心方法 `open()`、`exists()`、`close()` |
| 7 | `#include <QTextStream>` | 引入文本流 | `QTextStream` — 文本编码流，配合 `setCodec("GBK")` 处理中文 CSV。通过 `<<` / `>>` 或 `readLine()` 操作 |
| 8 | `#include <QStatusBar>` | 引入状态栏 | `QStatusBar` — `QMainWindow` 自带底部状态栏，通过 `statusBar()` 获取，`showMessage()` 设置临时/持久消息 |
| 9 | `#include <QHeaderView>` | 引入表头视图 | `QHeaderView` — `QTableWidget` 的头部，`setSectionResizeMode()` 控制列宽策略 |
| 10 | `#include <QSizePolicy>` | 引入大小策略 | `QSizePolicy` — 控件在布局中的伸缩策略，枚举值 `Expanding` / `MinimumExpanding` 等 |
| 11 | `#include <QDir>` | 引入目录操作 | `QDir` — 目录路径抽象，`exists()` 检查目录是否存在，`mkpath(".")` 递归创建目录 |
| 12 | `#include <QVBoxLayout>` | 引入布局操作 | `QVBoxLayout` — 垂直盒布局，`setContentsMargins()` 设外边框，`setSpacing()` 设控件间距 |
| 13 | `#include <QItemSelectionModel>` | 引入选中模型 | `QItemSelectionModel` — 管理选择状态，`selectedRows()` 返回选中行索引列表（`QModelIndexList`） |
| 14 | `#include <QFont>` | 引入字体设置 | `QFont` — 字体描述对象，在 `main.cpp` 中设置全局字体 `Microsoft YaHei UI` |
| 15 | `#include <algorithm>` | 引入 `std::sort` | 非 Qt 库函数，用于删除行时对全局行号做逆序排序（`std::greater<int>()`），避免索引偏移 |

---

## 二、静态辅助函数 (Lines 17–75)

### `indexOfCI` (Lines 21–28)

**功能**：QStringList 大小写不敏感查找。`QStringList::indexOf` 不接受 `Qt::CaseSensitivity` 参数，只能手动遍历 + `QString::compare(CaseInsensitive)` 实现。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 21 | `// QStringList 大小写不敏感查找...` | 注释说明为什么需要独立函数 | — |
| 22–28 | `static int indexOfCI(...)` 实现 | `for` 遍历列表，`compare(value, CaseInsensitive)` 比较，找到返回索引，否则 `-1` | `QString::compare(QString, Qt::CaseSensitivity)` — 跨平台字符串比较，`CaseInsensitive` 属于 `Qt` 命名空间枚举，不依赖 `strcasecmp` 等平台特定函数 |

### `stripQuotes` (Lines 30–35)

**功能**：去除字符串两端双引号。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 32–33 | `s.length() >= 2 && s.starts/endsWith('"')` | 检测是否为 `"..."` 包裹的字符串 | `QString::length()` — 返回字符数；`startsWith(QChar)/endsWith(QChar)` — 检查首/尾字符，比 `operator[]` 更安全（不会越界） |
| 33 | `return s.mid(1, s.length() - 2)` | 去掉首尾引号 | `QString::mid(int pos, int n)` — 返回子串，第二个参数省略时截取到末尾 |

### `splitConditions` (Lines 38–75)

**功能**：按 `and` 分割 WHERE 条件字符串，忽略引号内的 `and`。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 41–42 | `QString current; bool inQuote = false;` | 累积当前条件片段；引号标记 | — |
| 44–49 | `for` 循环，检测 `"` 时翻转 `inQuote` | 跟踪是否在引号字符串内 | `QString::at(int)` — 返回 `QChar`（引用，不拷贝）；`QChar` 可直接与 `char` 字面量 `'"'` 比较 |
| 51–64 | 不在引号内且遇到 `and` 单词 | 检查前后是否为空白/边界 → 是则分割，`i += 2` 跳过 | `QString::mid(int,3).toLower()` — 提取 3 字符子串并转小写比较；`QChar::isSpace()` — 检查是否为空白字符（空格/tab/换行等，跨平台 Unicode 兼容） |
| 66 | `current += c` | 非分割点则追加字符到当前片段 | `QString::operator+=(QChar)` — 尾部追加字符，自动管理内存 |
| 69–71 | 收尾：最后一个条件加入结果列表 | | `QStringList::operator<<(QString)` — 尾部追加元素 |

### 辅助函数解析示例

#### `indexOfCI` 调用追踪

| 输入 | 遍历过程（`QString::compare(查找值, Qt::CaseInsensitive)`） | 返回 |
|------|---------|------|
| `indexOfCI(["ID","NAME","SEX"], "name")` | `"ID".compare("name",CI)=1` → `"NAME".compare("name",CI)=0` → 命中索引 1 | **1** |
| `indexOfCI(["ID","NAME","SEX"], "Name")` | 同上（大写 N 在 CaseInsensitive 下等效小写 n） | **1** |
| `indexOfCI(["ID","NAME","SEX"], "foo")` | `"ID".compare("foo",CI)=1` → `"NAME".compare("foo",CI)=1` → `"SEX".compare("foo",CI)=1` → 全部不匹配 | **-1** |

#### `splitConditions` 解析追踪

| 输入 | 执行过程 | 返回 `QStringList` |
|------|---------|-------------------|
| `tab_class.id=tab_student.clazz_id and tab_class.id=101` | 遍历 i=0→40，在 i=40 处 `wherePart.mid(40,3).toLower()=="and"`，前后均为空格 → 分割，追加前半段到结果，`current` 清空，`i+=2` 跳过 | `["tab_class.id=tab_student.clazz_id", "tab_class.id=101"]` |
| `name="Zhang and Li" and id=1` | i=5 遇到 `"` → `inQuote=true`；i=12 处 `mid(12,3)=="and"` 但 `inQuote=true` → 不分割，继续追加；i=22 遇到 `"` → `inQuote=false`；后续 i=24 处再次遇到 `and`，`inQuote=false` 且前后空格 → 分割 | `["name=\"Zhang and Li\"", "id=1"]` |
| `a=1 and b=2` | i=4 处 `mid(4,3)=="and"`，前后均为空格 → 分割 | `["a=1", "b=2"]` |

---

## 三、构造函数 `MainWindow::MainWindow` (Lines 81–215)

### 构造初始化 (Lines 81–87)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 81–83 | `MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)` | 基类初始化 + 创建 UI 实例 | `QMainWindow` 构造：Qt 对象树，`parent` 为 `nullptr` 时成为顶层窗口；`Ui::MainWindow` 由 `uic` 生成，`setupUi()` 会创建所有子控件并挂入该窗口对象树 |
| 85 | `ui->setupUi(this);` | 加载 UI 布局到当前窗口 | 该方法遍历 `.ui` 描述的控件树，调用 `new QWidget/QLabel/...` + `setObjectName()` + 属性设值，最后调用 `QMetaObject::connectSlotsByName(this)` 自动绑定 `on_` 命名 slot |
| 86 | `this->setWindowTitle("数据管理系统");` | 设置窗口标题 | `QWidget::setWindowTitle(QString)` — 设置标题栏文字 |
| 87 | `this->resize(1000, 700);` | 设置窗口初始尺寸 | `QWidget::resize(int w, int h)` — 设置窗口客户区大小 |

### welcome 标签设置 (Lines 89–95)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 90 | `ui->label->setText("欢迎来到张智毅的数据管理系统");` | 设置欢迎文字 | `QLabel::setText(QString)` — 设置标签显示的纯文本（支持 HTML 子集） |
| 91 | `ui->label->setWordWrap(false);` | 禁止自动换行 | `QLabel::setWordWrap(bool)` — 单行模式下配合 `adjustSize()` 自适应宽度 |
| 92 | `ui->label->setAlignment(Qt::AlignCenter);` | 居中对齐 | `Qt::AlignmentFlag` — `Qt::AlignCenter` = `AlignVCenter \| AlignHCenter` |
| 93 | `ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);` | 水平可扩展、垂直最小+可扩展 | `QSizePolicy` 构造(Horiz, Vert) — `Expanding` = 空间多余时尽量扩展；`MinimumExpanding` = 有最小尺寸但可扩展。影响布局中的控件伸缩行为 |
| 94 | `ui->label->setMinimumHeight(48);` | 设置最小高度 | `QWidget::setMinimumHeight(int)` — 布局缩放时不低于该值 |
| 95 | `ui->label->adjustSize();` | 根据内容调整大小 | `QWidget::adjustSize()` — 子类可重写，`QLabel` 默认按文本+换行策略计算 |

### 分页参数 & 布局 (Lines 97–107)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 98 | `m_pageSize = 10;` | 每页 10 行 | 纯成员变量赋值 |
| 99 | `m_currentPage = 1;` | 初始页码为 1 | 纯成员变量赋值 |
| 100 | `m_isMultiTable = false;` | 初始化多表标记为 false | 纯成员变量赋值 |
| 103 | `QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());` | 安全获取主布局指针 | `qobject_cast<T>()` — 依赖 `Q_OBJECT` 元对象系统的运行时类型识别，替代 `dynamic_cast`。失败返回 `nullptr`，适用于 `QObject` 继承体系 |
| 105 | `mainLayout->setContentsMargins(24, 24, 24, 24);` | 设置布局外边框 24px | `QLayout::setContentsMargins(int left, top, right, bottom)` — 控件与布局边框之间的留白 |
| 106 | `mainLayout->setSpacing(18);` | 子控件间距 18px | `QBoxLayout::setSpacing(int)` — 子控件之间的空白间隙 |

### QSS 样式表 (Lines 109–214)
大量 Qt 样式表描述 UI 外观。Qt 使用 CSS2.1 语法的 QSS 子集，支持 `#objectName` 选择器、伪状态（`:hover/:pressed`）、`qlineargradient` 渐变。不涉及 API 调用，仅字符串传给 `setStyleSheet()`。
| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 110 | `setStyleSheet(R"()");` | 一次性设置全局样式 | `QWidget::setStyleSheet(QString)` — 原始字符串字面量 `R"(...)"` 是 C++11 特性，非 Qt。Qt 在运行时解析 QSS → 应用到窗口及其所有子控件 |

---

## 四、析构函数 `~MainWindow` (Lines 217–220)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 219 | `delete ui;` | 释放 UI 实例 | Qt 对象树机制：`ui->setupUi(this)` 时将子控件以 `this` 为父对象创建，`QMainWindow` 析构时会自动递归删除所有子对象。但 `Ui::MainWindow` 对象本身（非 `QObject` 子类）需要在 `~MainWindow` 中手动释放 |

---

## 五、`validateAndParseSql` — SQL 解析与验证 (Lines 235–464)

**功能**：完整解析 `SELECT field FROM table [WHERE cond]`，检测 5 种语法错误，结果写入 `m_queryFields` / `m_queryTables` / `m_queryConditions` / `m_isMultiTable` / `m_joinHeader` / `m_fieldIndexes`。

### 函数头及重置 (Lines 235–244)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 237–244 | `m_queryFields.clear(); m_queryTables.clear(); m_queryConditions.clear(); m_isMultiTable = false; m_joinHeader.clear(); m_fieldIndexes.clear();` | 重置所有查询状态变量 | `QStringList::clear()` / `QList::clear()` — 清空容器，释放内存 |

### 步骤 ① — 规范化空白 + 检查 SELECT（Lines 245–259）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 246 | `QString s = sql.simplified().trimmed();` | 合并连续空格 → 去除首尾空白（容错：多余空格） | `QString::simplified()` — 将内部连续空白（含 `\n`、`\t`、`\r`）替换为单空格，同时 trim 首尾，返回新 QString；`trimmed()` — 仅去除首尾空白 |
| 247–250 | `if (s.isEmpty())` | SQL 为空则报错 | `QString::isEmpty()` — 等价于 `length() == 0`，O(1) 复杂度 |
| 253–256 | `if (!s.startsWith("select", Qt::CaseInsensitive))` | **不区分大小写**检查 SELECT 关键字，错误类型 1 | `QString::startsWith(QString, Qt::CaseSensitivity)` — 第二个参数传入 `Qt::CaseInsensitive` 实现大小写不敏感前缀匹配 |
| 259 | `s = s.mid(6).trimmed();` | 去掉 "SELECT" 前缀 | `QString::mid(int pos)` — 从 pos 截取到末尾 |

### 步骤 ② — 查找 FROM（Lines 261–271）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 265–266 | `QString lower = s.toLower(); fromIdx = lower.indexOf("from");` | 在小写版本中定位 "from" | `QString::toLower()` — 返回全小写副本（非原地修改）；`indexOf(QString)` — 默认大小写敏感查找，返回索引或 -1 |
| 267–270 | `if (fromIdx < 0)` | 找不到 → 报错 | — |

### 步骤 ③ — 提取字段列表（Lines 273–295）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 274 | `QString fieldPart = s.left(fromIdx).trimmed();` | 截取字段部分 | `QString::left(int n)` — 取前 n 个字符 |
| 277–280 | `if (fieldPart.endsWith(","))` | 末尾逗号 → 错误类型 3 | `QString::endsWith(QString)` — 检查后缀 |
| 283–288 | `split(",", Qt::SkipEmptyParts)` → 逐项 `trimmed()` | 分割字段列表 | `QString::split(QString, Qt::SplitBehavior)` — `Qt::SkipEmptyParts` 跳过空字符串（如 `a,,b` 中间的 `""`）；返回 `QStringList` |
| 290–292 | `if (m_queryFields.isEmpty())` | 空字段列表报错 | `QStringList::isEmpty()` |
| 295 | `bool isStar = (m_queryFields.size() == 1 && m_queryFields[0] == "*");` | 标记 `SELECT *` | `QStringList::size()` / `operator[]` |

### 步骤 ④ — 提取 FROM 后内容（Lines 297–314）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 298–299 | `QString afterFrom = s.mid(fromIdx + 4).trimmed();` | 获取 "from" 后面的部分 | `QString::mid()` |
| 302 | `int whereIdx = afterFromLower.indexOf("where");` | 查找 WHERE 关键字 | `QString::indexOf()` |
| 305–310 | 分割 `tablePart` 和 `wherePart` | 有 where → 分割；无 where → 全部为表名 | `QString::left()`, `QString::mid()` |
| 313–314 | 去除末尾分号 | 兼容 `;` 结尾 | `QString::endsWith(";")`, `QString::chop(1)` — 删除末尾 1 字符（原地修改） |

### 步骤 ⑤ — 解析表名（Lines 316–331）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 317 | `split(",", Qt::SkipEmptyParts)` | 逗号分割多个表 | `QString::split()` |
| 318–326 | 遍历，去除别名（空格后内容） | 支持 `FROM tab_class c` 语法 | `QString::indexOf(' ')` + `left()` |
| 328–331 | `if (m_queryTables.isEmpty())` | 空表名报错（错误类型 2） | `QStringList::isEmpty()` |

### 步骤 ⑥ — 检查表文件存在（Lines 333–342）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 335–339 | `QFile file("D:/qt/Object/" + t + ".csv"); if (!file.exists())` | 检查磁盘文件是否存在 | `QFile::exists()` — 静态方法也可：`QFile::exists(path)`。底层调用 OS 文件系统 API，不打开文件 |
| 342 | `m_isMultiTable = (m_queryTables.size() > 1);` | 标记单表/多表 | `QList::size()` |

### 步骤 ⑦ — 解析 WHERE 条件（Lines 344–408）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 346 | `QStringList condStrs = splitConditions(wherePart);` | 按 `and` 分割条件 | 自定义函数，内部使用 `QString::mid().toLower()`, `QChar::isSpace()` |
| 352–361 | 确定操作符位置 | `>=`/`<=`/`<>` 优先于单字符 `=`/`>`/`<` | `QString::indexOf(QString)` — 查找模式串首次出现位置；优先检查双字符操作符避免误判（如 `>=` 不会因先找到 `=` 而截断） |
| 368–370 | `left` / `op` / `right` 三部分 | 按操作符位置拆分 | `QString::left(opPos)`, `QString::mid(opPos, opLen)`, `QString::mid(opPos + opLen)` |
| 372–379 | 解析左侧 | 含 `.` 则分割为 `leftTable` + `leftField` | `QString::indexOf('.')`, `QString::left()`, `QString::mid()` |
| 381–405 | 解析右侧 | 引号 → 值；含 `.` → 字段引用；数值 → 数值；否则 → 错误类型 5 | `QString::startsWith('"')`, `QString::contains('.')`, `QString::toDouble(&isNumeric)` — 尝试数值转换，成功则 `isNumeric == true` |
| 407 | `m_queryConditions << cond;` | 添加条件到列表 | `QList::operator<<(const T&)` — 尾部追加元素 |

### 步骤 ⑧ — 检查字段名存在（Lines 411–461）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 413–415 | `if (isStar)` | `*` 跳过检查，执行时展开 | — |
| 416–448 | 多表：字段必须 `table.field` 格式 | 先验证表名在 FROM 列表中，再用 `indexOfCI()` 验证字段在表头中存在 | `QString::compare(table, Qt::CaseInsensitive)` — 表名比较 L430（大小写不敏感）；`loadCsvFile()` 内部使用 `QFile` + `QTextStream` + 编码设置 |
| 449–460 | 单表：直接用 `indexOfCI()` 检查字段在表头中 | 错误类型 4，**大小写兼容** | `indexOfCI()` 为自定义静态辅助函数，内部使用 `QString::compare(CaseInsensitive)` |

### 解析示例

#### 示例 1：简单单表查询

```
输入：SELECT ID, NAME FROM tab_class WHERE ID=101
```

| 步骤 | 代码行 | 中间结果 |
|------|-------|---------|
| ① 规范化空白 | L246 | `s = "ID, NAME FROM tab_class WHERE ID=101"`（已去掉 SELECT） |
| ② 查找 FROM | L265–266 | `lower = "id, name from tab_class where id=101"`, `fromIdx = 9` |
| ③ 提取字段 | L274 | `fieldPart = "ID, NAME"`；L283 `split(",", SkipEmptyParts)` → `m_queryFields = ["ID", "NAME"]` |
| ④ FROM 后内容 | L298 | `afterFrom = "tab_class WHERE ID=101"`；WHERE 在 L302 `whereIdx=10` |
| ⑤ 解析表名 | L317 | `m_queryTables = ["tab_class"]` |
| ⑥ 文件存在性 | L335 | `QFile("D:/qt/Object/tab_class.csv").exists() → true` ✓ |
| ⑦ WHERE 条件 | L352 | `condStr="ID=101"`, `opPos=2 (opLen=1)`, `left="ID"`, `right="101"`；`right.toDouble(&isNumeric)` 成功 → `cond.rightValue="101"`, `isFieldCompare=false` |
| ⑧ 字段检查 | L452 | `indexOfCI(["ID","NAME","SEX"], "ID")=0` ✓、`indexOfCI(..., "NAME")=1` ✓ |

**解析结果成员变量：**
```
m_queryFields     → ["ID", "NAME"]
m_queryTables     → ["tab_class"]
m_queryConditions → [{leftField:"ID", op:"=", rightValue:"101", isFieldCompare:false}]
m_isMultiTable    → false
```

#### 示例 2：多表 JOIN（字段 vs 字段比较）

```
输入：SELECT tab_class.name,tab_student.name FROM tab_class, tab_student WHERE tab_class.id=tab_student.clazz_id
```

| 步骤 | 代码行 | 中间结果 |
|------|-------|---------|
| ③ 字段分割 | L283–287 | `m_queryFields = ["tab_class.name", "tab_student.name"]` |
| ⑤ 表名解析 | L317–325 | `m_queryTables = ["tab_class", "tab_student"]` → L342 `m_isMultiTable = true` |
| ⑦ 条件解析 | L352 | `condStr="tab_class.id=tab_student.clazz_id"`；`opPos=10(opLen=1)`；左侧 L374 `dot=9` → `leftTable="tab_class"`, `leftField="id"`；右侧 L388 `dot=13` → `rightTable="tab_student"`, `rightField="clazz_id"`, `isFieldCompare=true` |
| ⑧ 字段检查 | L418–448 | 分别检查 `tab_class.name` 在 `tab_class` 表头中存在、`tab_student.name` 在 `tab_student` 表头中存在 ✓ |

**解析结果成员变量：**
```
m_queryFields     → ["tab_class.name", "tab_student.name"]
m_queryTables     → ["tab_class", "tab_student"]
m_queryConditions → [{leftTable:"tab_class", leftField:"id", op:"=",
                       rightTable:"tab_student", rightField:"clazz_id", isFieldCompare:true}]
m_isMultiTable    → true
```

#### 示例 3：触发错误类型 ⑤（字符串值缺少引号）

```
输入：SELECT ID FROM tab_student WHERE sex=男
```

| 步骤 | 代码行 | 中间结果 |
|------|-------|---------|
| ①–⑥ | L246–339 | 全部通过：关键字、表名 `tab_student`、字段 `ID` 均合法 ✓ |
| ⑦ 解析右侧 `"男"` | L382–404 | `startsWith('"')` → false（无引号）；`contains('.')` → false；`toDouble(&isNumeric)` → false（非数值）→ 进入 L402 |
| ⑧ 错误返回 | L402–403 | `errorMsg = "SQL语法错误：字符串值缺少引号：男（错误类型 5）"`，`return false` |

**QMessageBox 最终弹出内容：** `"SQL语法错误：字符串值缺少引号：男（错误类型 5）"`

---

## 六、`loadCsvFile` — 通用 CSV 加载 (Lines 470–497)

**功能**：读取 `D:/qt/Object/<tableName>.csv`，返回表头和数据。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 473–475 | `QFile file(...) → open(ReadOnly \| Text)` | 以只读文本模式打开文件 | `QFile::open(QIODevice::OpenMode)` — `QIODevice::ReadOnly` 只读；`Text` 标志使 Windows 下 `\r\n` 自动转换为 `\n`。失败返回 false |
| 477–478 | `QTextStream in(&file); in.setCodec("GBK");` | 创建文本流，编码 GBK | `QTextStream::setCodec("GBK")` — Qt5 API，设置字符编码。Qt6 移除该函数改用 `setEncoding()`。GBK 兼容 GB2312 简体中文 |
| 480 | `QString headerLine = in.readLine().trimmed();` | 读取第一行（表头） | `QTextStream::readLine()` — 读取直到 `\n` 或 EOF，返回 `QString`。调用后文件指针前进到下一行 |
| 481–484 | `if (headerLine.isEmpty())` | 空表头报错 | — |
| 486 | `header = headerLine.split(",", Qt::SkipEmptyParts);` | 按逗号拆分表头 | `QString::split()` — `SkipEmptyParts` 忽略空字段（如 `a,,b` 的中间空项） |
| 489–492 | `while (!in.atEnd())` 逐行读取 | 跳过空行，按逗号拆分为 `QStringList` 追加到 `data` | `QTextStream::atEnd()` — 文件末尾返回 true（非阻塞）；此处用 `Qt::KeepEmptyParts` 保留空 CSV 字段（如 `,,` 中间的空字符串） |
| 495 | `file.close();` | 释放文件句柄 | `QFile::close()` — 关闭文件描述符。`QFile` 析构也会自动 close |

---

## 七、`compareValues` — 值比较 (Lines 503–533)

**功能**：比较两个值（优先数值比较，回退字符串比较）。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 509–511 | `l.toDouble(&lOk); r.toDouble(&rOk);` | 尝试转换为数值 | `QString::toDouble(bool *ok)` — 输出参数 `ok` 标记转换是否成功。Qt 还提供 `toInt/toFloat/toULongLong` 等系列方法 |
| 513–520 | 两者均为数值 | 使用直接比较运算符（`==`/`!=`/`>`/`<`/`>=`/`<=`）进行浮点数值比较 | — |
| 523–530 | 字符串比较 | 使用 `QString::compare(Qt::CaseInsensitive)` | `QString::compare(QString, Qt::CaseSensitivity)` — 返回 `int`（类似 `strcmp`），`CaseInsensitive` 通过 `QChar::toCaseFolded()` 逐字符折叠比较，非简单 `tolower()`，兼容更多 Unicode 字符 |

## 八、`checkConditions` — 条件匹配 (Lines 540–591)

**功能**：对单行数据检查所有 WHERE 条件是否满足。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 547–549 | 构建左侧查询键 | 有 `leftTable` → `table.field`；无 → 直接 `field` | `QString` 拼接：`cond.leftTable + "." + cond.leftField` |
| 552–558 | **手动遍历** header 数组，逐元素大小写不敏感比较 | 替代可能失效的 `indexOf`，在表头中定位列索引 | `QString::compare(value, Qt::CaseInsensitive) == 0` — 逐个比较 header 元素 |
| 562 | `QString leftVal = row[leftIdx].trimmed();` | 获取左侧值并去除空白 | `QStringList::operator[]` + `trimmed()` |
| 564–587 | 分支：字段间比较 / 字段-值比较 | 字段间 → 再次手动遍历查找右侧索引再比较；值 → 直接对比 | QList 遍历 + `QString::compare()` 实现大小写不敏感列查找 |

---

## 九、`executeSingleTable` — 单表查询执行 (Lines 598–655)

**功能**：加载单表 CSV → WHERE 过滤 → 显示表格。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 603–608 | `loadCsvFile(tableName, header, rawData)` | 加载完整 CSV | 内部调用 `QFile` + `QTextStream`，失败则弹出警告：`QMessageBox::warning(this, "错误", msg)` — 模态警告弹窗，阻塞直到用户关闭 |
| 611 | `m_csvHeader = header;` | 保存完整表头（写回用） | `QStringList` 深拷贝赋值 |
| 614–616 | `if (isStar) m_queryFields = header;` | `*` 展开为所有列 | `QStringList` 赋值 |
| 618–622 | 构建 `m_fieldIndexes`（使用 `indexOfCI()`） | 查询字段 → CSV 列索引映射，**大小写兼容** | `QList<int>::operator<<` — 追加元素 |
| 625–630 | `for (row : rawData) if (checkConditions(row, header)) m_allData << row;` | WHERE 过滤，存储**完整行** | C++11 range-based for 遍历 `QList<QStringList>` |
| 632–642 | 设置表格属性 | 可编辑、可多选、交替行色、列宽拉伸 | 见下方拆分说明 |
| 645–647 | `btn_add/delete/save->setEnabled(true)` | 启用 CRUD | `QWidget::setEnabled(bool)` — 启用/禁用控件交互；禁用时子控件一并灰显 |
| 649 | `m_currentPage = 1;` | 重置为第一页 | — |
| 650 | `showPageData();` | 分页渲染第一页 | — |
| 652–653 | 状态栏信息 | 显示表名和行数 | `QStatusBar::showMessage(QString, int timeout)` — 显示消息；timeout 为 0 表示持久显示 |

### 表格设置详细说明（Lines 632–642）

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 633 | `ui->tableWidget->clearContents();` | 清除所有单元格内容（保留行列表头） | `QTableWidget::clearContents()` — 不清除行列数，不同于 `clear()` |
| 634 | `ui->tableWidget->setRowCount(0);` | 重置行数为 0 | `QTableWidget::setRowCount(int)` — 也会清除已有 item |
| 635 | `ui->tableWidget->setColumnCount(m_queryFields.size());` | 设置列数 = SELECT 字段数 | `QTableWidget::setColumnCount(int)` |
| 636 | `ui->tableWidget->setHorizontalHeaderLabels(m_queryFields);` | 设置列标题 | `QTableWidget::setHorizontalHeaderLabels(QStringList)` — 内部创建 `QTableWidgetItem` 作为表头标签 |
| 637–638 | `setEditTriggers(DoubleClicked \| EditKeyPressed)` | 双击或 F2 编辑 | `QAbstractItemView::EditTriggers` — 枚举值组合：`DoubleClicked` 双击进入编辑，`EditKeyPressed` F2 进入编辑 |
| 639 | `setSelectionBehavior(SelectRows)` | 点击选中整行 | `QAbstractItemView::SelectionBehavior::SelectRows` |
| 640 | `setSelectionMode(ExtendedSelection)` | 支持多选（Ctrl/Shift） | `QAbstractItemView::SelectionMode::ExtendedSelection` — 配合 Ctrl 点选、Shift 连选 |
| 641 | `setAlternatingRowColors(true)` | 交替行背景色 | `QTableWidget::setAlternatingRowColors(bool)` — 配合 QSS 中的 `alternate-background-color` 使用 |
| 642 | `horizontalHeader()->setSectionResizeMode(Stretch)` | 列宽等分填充空白 | `QHeaderView::setSectionResizeMode(ResizeMode)` — `Stretch` 使所有列均分总宽度，无空白列

---

## 十、`executeMultiTable` — 多表连接查询执行 (Lines 661–767)

**功能**：多表笛卡尔积 → WHERE 过滤 → 提取 SELECT 列 → 只读显示。

### 加载所有表 (Lines 667–676)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 667–676 | 遍历 `m_queryTables`，依次 `loadCsvFile()` | 加载全部表入内存 | 失败时调用 `QMessageBox::warning()` |

### 构建组合表头 (Lines 678–684)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 680–683 | 双层循环：表 → 字段 | 组合表头格式：`表名.字段名`（如 `tab_class.ID`） | `QString::operator+(QString)` — 拼接；`QStringList::operator<<` — 追加到列表 |

### 展开 + 索引映射 (Lines 686–700)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 687–689 | `*` 展开 | 替换为所有组合字段 | `QStringList` 赋值 |
| 691–700 | 映射 SELECT 字段 → 组合表头索引（使用 `indexOfCI()`） | 找不到则报错，**大小写兼容** | `QList<int> selectIndexes` — 存储映射结果；找不到用 `QMessageBox::warning()` 报错 |

### 笛卡尔积 (Lines 702–728)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 706–709 | 从第一张表初始化 `product` | 所有行为原始行 | `QList<QStringList> product` — 泛型列表，`operator<<` 深拷贝 |
| 712–728 | 依次交叉连接其余表 | 双层循环：现有行 × 新表行 → `combined << newRow` | `QList::size()` 获取表数据长度；`QStringList::operator<<(QStringList)` — 合并两个列表 |
| 724–727 | 内存保护 | 超过 100 万行时终止并提示 | `QMessageBox::warning()` |

### 过滤 + 提取 (Lines 730–739)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 731–739 | 逐行检查 `checkConditions(row, combinedHeader)` | 满足条件的提取 SELECT 列加入 `m_allData` | `QList::operator<<(QStringList)` — 追加单行 |

### 保存状态 + 设置表格（只读）(Lines 741–767)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 742–743 | `m_joinHeader = combinedHeader; m_csvHeader = m_queryFields;` | 保存组合表头和查询字段（供 `showPageData` 使用） | `QStringList` 深拷贝 |
| 746–754 | `clearContents / setRowCount / setColumnCount / setHorizontalHeaderLabels / ...` | 重置表格 | 同单表段（L633–642），区别仅在 L750 |
| 750 | `setEditTriggers(NoEditTriggers)` | 表格只读，不可编辑 | `QAbstractItemView::NoEditTriggers` — 禁用所有编辑触发方式 |
| 757–759 | `btn_add/delete/save->setEnabled(false)` | 禁用 CRUD | `QWidget::setEnabled(false)` — 控件灰显不可点击 |
| 761–762 | `m_currentPage = 1; showPageData();` | 重置页码并渲染 | — |
| 764–765 | 状态栏信息 | 显示多表查询行数 | `QStatusBar::showMessage()` 持久显示（timeout=0） |

---

## 十一、`on_pushButton_clicked` — 确定按钮 (Lines 773–787)

**功能**：入口函数，验证 → 分发。函数名遵循 `on_<objectName>_<signal>()` 命名约定，`setupUi(this)` 内部调用 `QMetaObject::connectSlotsByName()` 自动完成信号槽绑定。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 775 | `QString sql = ui->lineEdit->text();` | 获取输入框 SQL | `QLineEdit::text()` — 返回当前输入框文本 |
| 778–781 | `if (!validateAndParseSql(sql, errorMsg))` | 校验失败 → 弹窗报错并 return | `QMessageBox::warning(this, title, msg)` — 模态弹窗 |
| 783–786 | `if (m_isMultiTable) executeMultiTable() else executeSingleTable()` | 按单表/多表分发执行 | — |

---

## 十二、`showPageData` — 分页渲染 (Lines 793–844)

**功能**：根据当前页码和页大小，从 `m_allData` 中截取数据填充到表格。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 795–800 | `if (m_queryFields.isEmpty())` | 无查询字段时显示"无数据"并禁用翻页 | `QLabel::setText("无数据")`; `QPushButton::setEnabled(false)` |
| 802–806 | 重置表格列结构 | 清空 → 设置列数 → 设表头 → 列宽拉伸 | `QTableWidget::clearContents()` + `setRowCount(0)` + `setColumnCount()` + `setHorizontalHeaderLabels()` |
| 808–813 | `if (m_allData.isEmpty())` | 空数据显示"第 1 页 / 共 0 页" | `QLabel::setText(QString)`; `QPushButton::setEnabled(false)` |
| 815–816 | `start = (page-1)*pageSize; end = qMin(...)` | 计算当前页起止索引 | `qMin(a, b)` — 非 Qt API，而是 Qt 全局模板函数（`<QtGlobal>`），返回较小值 |
| 818–837 | 双层循环填充表格 | **多表分支**：`dataRow` 已为 SELECT 列，直接显示；**单表分支**：通过 `m_fieldIndexes` 从完整行提取显示列 | `QTableWidget::insertRow(int)` — 在指定位置插入空白行；`QTableWidget::setItem(row, col, new QTableWidgetItem(val))` — 设置单元格，`QTableWidgetItem` 对象所有权由 QTableWidget 接管（Qt Model/View 架构：无需手动 delete） |
| 839–843 | 页码标签 + 按钮状态 | 更新页码显示 + 根据首末页启用/禁用翻页按钮 | `QString::arg(int)` — 格式化占位符 `%1`；`QPushButton::setEnabled(bool)` |

---

## 十三、`syncTableToData` — 编辑同步 (Lines 850–875)

**功能**：将表格编辑写回 `m_allData`（仅单表模式有效）。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 853 | `if (m_isMultiTable \|\| m_queryFields.isEmpty() \|\| m_csvHeader.isEmpty()) return;` | 多表查询跳过同步（只读） | — |
| 856–860 | 构建 `fieldToCsv` 映射（使用 `indexOfCI()`） | 查询字段 → CSV 全列索引，**大小写兼容** | `QList<int>::operator<<`; `indexOfCI()` 内部 `QString::compare(CaseInsensitive)` |
| 862–874 | 双层循环写回 | 遍历可见行 → 计算全局行号 → 映射列 → **补齐行长度（`while` 添加空串）** → 写回 `m_allData` | `QTableWidget::rowCount()` — 当前表格行数；`QTableWidget::item(int row, int col)` — 返回 `QTableWidgetItem*`，空单元格返回 `nullptr`；`QTableWidgetItem::text()` — 获取单元格文本；`while` 补齐用 `QStringList::operator<<("")` 确保不越界 |

---

## 十四、`on_btn_pre_clicked` — 上一页 (Lines 881–888)

函数名遵循 `on_<objectName>_<signal>()` 命名约定，`setupUi()` 通过 `QMetaObject::connectSlotsByName()` 自动绑定 `btn_pre` 的 `clicked()` 信号。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 883–887 | `syncTableToData()` → `if (m_currentPage > 1) m_currentPage--` → `showPageData()` | 先同步编辑，再翻页 | — |

---

## 十五、`on_btn_next_clicked` — 下一页 (Lines 894–902)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 896–901 | `syncTableToData()` → 重新计算 `totalPage` → `if (m_currentPage < totalPage) m_currentPage++` → `showPageData()` | 先同步编辑，再翻页（防止最后一页越界） | — |

---

## 十六、`on_btn_add_clicked` — 添加行 (Lines 908–927)

函数名 `on_btn_add_clicked` 经 `QMetaObject::connectSlotsByName()` 自动绑定按钮的 `clicked()` 信号。

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 910–912 | `if (m_isMultiTable)` | 多表模式禁止编辑 | `QMessageBox::warning()` |
| 914–917 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示 | `QMessageBox::warning()` |
| 918 | `syncTableToData()` | 先同步当前编辑 | — |
| 920–923 | 构造全空行追加到 `m_allData` | 长度 = CSV 列数 | `QStringList` 循环追加 `""`，`QList::append()` 追加整行 |
| 925 | `m_currentPage = ...` | 跳转到最后一页 | `(size + pageSize - 1) / pageSize` — 分页整数上取整公式 |
| 926 | `showPageData()` | 重新渲染 | — |

## 十七、`on_btn_delete_clicked` — 删除行 (Lines 933–966)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 935–937 | `if (m_isMultiTable)` | 多表模式禁止编辑 | `QMessageBox::warning()` |
| 939–942 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示 | `QMessageBox::warning()` |
| 943 | `syncTableToData()` | 同步编辑 | — |
| 945–948 | `selectedRows()` → 空则警告 | 获取选中行 | `QTableWidget::selectionModel()` — 返回 `QItemSelectionModel*`；`selectedRows()` — 返回 `QModelIndexList`，每个元素携带 `row()`/`column()` 信息 |
| 951–952 | 确认对话框 | 防误删除 | `QMessageBox::question(this, title, text)` — 返回 `StandardButton` 枚举（`Yes`/`No`/`Cancel` 等） |
| 954–960 | 逆序删除 | `globalRow = (page-1)*pageSize + row` → `std::sort` 逆序 → `removeAt()` | `std::sort(..., std::greater<int>())` — C++ STL 排序；`QList::removeAt(int)` — 按索引移除元素，后续元素自动前移 |
| 962–965 | 调整页码 → 刷新 | 防止空页 | — |

## 十八、`on_btn_save_clicked` — 保存 CSV (Lines 972–989)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 974–976 | `if (m_isMultiTable)` | 多表模式禁止保存 | `QMessageBox::warning()` |
| 978–981 | `if (m_csvHeader.isEmpty())` | 无数据时提示 | `QMessageBox::warning()` |
| 982 | `syncTableToData()` | 同步编辑 | — |
| 984 | `QString tableName = m_queryTables.isEmpty() ? "" : m_queryTables[0];` | 从存储状态取表名 | — |
| 985–988 | `if (saveCsv(tableName))` | 调用保存函数 | 成功：`QStatusBar::showMessage("保存成功！", 3000)` — 3 秒后自动消失；`QMessageBox::information()` — 非模态成功提示 |

---

## 十九、`saveCsv` — 写入 CSV 文件 (Lines 995–1017)

| 行号 | 语句 | 作用 | 涉及 Qt API / 说明 |
|------|------|------|-------------------|
| 999–1000 | `QDir dir("D:/qt/Object/"); if (!dir.exists()) dir.mkpath(".");` | 确保目标目录存在 | `QDir::exists()` — 检查目录是否存在；`QDir::mkpath(".")` — 递归创建不存在的父目录，相当于 `mkdir -p` |
| 1002–1005 | `QFile(...) → open(WriteOnly \| Text)` | 以只写文本模式打开文件 | `QIODevice::WriteOnly` — 清空写入（非追加）；失败时 `QMessageBox::critical()` — 严重错误弹窗 |
| 1008–1009 | `QTextStream out(&file); out.setCodec("GBK");` | 创建文本流，编码 GBK | `QTextStream` 通过 `<<` 运算符将各种类型（QString、int、double）自动转为文本写入 |
| 1010 | `out << m_csvHeader.join(",") << "\n";` | 写入表头行 | `QStringList::join(QString)` — 用指定分隔符连接列表元素返回 `QString`；`QTextStream::operator<<(QString)` — 流式输出 |
| 1012–1013 | `for (row : m_allData) out << row.join(",") << "\n";` | 逐行写入数据 | 同上 |
| 1015 | `file.close();` | 释放文件句柄 | `QFile::close()` — 显式关闭；`QFile` 析构也会自动关闭 |

---

## 二十、测试用例

### 20.1 SQL 语法校验（5 种错误检测）

| # | 测试输入 | 预期结果 | 覆盖错误类型 |
|---|---------|---------|------------|
| 1 | `selec * from tab_class` | 弹窗"关键字拼写错误" | ① |
| 2 | `select * from` | 弹窗"FROM 后缺少表名" | ② |
| 3 | `select * from not_exist` | 弹窗"表不存在：not_exist" | ② |
| 4 | `select ID,NAME, from tab_class` | 弹窗"字段列表末尾有多余逗号" | ③ |
| 5 | `select na from tab_class` | 弹窗"字段不存在：na" | ④ |
| 6 | `select ID from tab_student where sex=男` | 弹窗"字符串值缺少引号：男" | ⑤ |

### 20.2 容错兼容（合法语句应正常执行）

| # | 输入 | 预期 |
|---|------|------|
| 7 | `select ID,   NAME from tab_class`（多空格） | 正常显示两列 |
| 8 | `SELECT ID , NAME FROM TAB_CLASS`（大写关键字） | 正常显示 |
| 8.5 | `select name from tab_class`（小写字段名） | 正常显示，大小写兼容 |
| 8.6 | `select NAME, name, Name from tab_class`（混合大小写字段） | 正常显示，大小写兼容 |
| 9 | `select * from tab_student where sex = "男"`（标准引号） | 仅显示男生 |
| 9.5 | `select * from tab_student where sex="男"`（无空格紧凑写法） | 仅显示男生 |

### 20.3 单表查询

| # | 输入 | 预期 |
|---|------|------|
| 10 | `select * from tab_class` | 显示全部列 + 全部行 |
| 11 | `select ID,NAME from tab_class` | 仅 ID、NAME 两列 |
| 12 | `select * from tab_student where sex="男"` | 仅男生行（CSV 中 SEX 字段大部分为空，仅少数学生有值） |
| 13 | `select * from tab_score where result>=90` | 成绩 ≥ 90 的行 |
| 14 | `select * from tab_course` | 返回 tab_course 全部行（当前仅一行空数据） |

### 20.4 多表 JOIN

| # | 输入 | 预期 | 说明 |
|---|------|------|------|
| 15 | `SELECT tab_class.name,tab_student.name from tab_class, tab_student where tab_class.id=tab_student.clazz_id and tab_class.id=101` | 101 班学生名单（约 60 人） | 字段全小写，验证大小写兼容 |
| 15.5 | `SELECT TAB_CLASS.NAME,TAB_STUDENT.NAME FROM TAB_CLASS,TAB_STUDENT WHERE TAB_CLASS.ID=TAB_STUDENT.CLAZZ_ID AND TAB_CLASS.ID=102` | 102 班学生名单（约 60 人） | 全大写，验证大写兼容 |
| 16 | `SELECT * from tab_class, tab_student where tab_class.id=tab_student.clazz_id` | 班级+学生全连接（约 168 行，只读） | 基本 2 表笛卡尔积过滤 |
| 17 | `SELECT tab_student.id,tab_student.name from tab_student, tab_score where tab_score.stu_id=tab_student.id and tab_score.result>=90` | 成绩 ≥ 90 的学生（约 500+ 行） | 2 表 JOIN + 值比较条件 |
| 18 | `SELECT tab_student.name,tab_score.result from tab_student, tab_score, tab_class where tab_score.stu_id=tab_student.id and tab_student.clazz_id=tab_class.id and tab_class.id=101` | 101 班每个学生各科成绩 | 3 表 JOIN |

> **注意**：`tab_course.csv` 当前只有一行空数据（`ID,NAME` 表头下一行是 `,`），凡涉及 `tab_course` 的 4 表 JOIN（如课程名称、课程名+成绩过滤）均返回 0 行。需先往 `tab_course.csv` 补充真实课程数据后才能生效。

### 20.5 分页功能

| # | 操作 | 预期 |
|---|------|------|
| 19 | `select * from tab_student` → 点击下一页 | 每页 10 行，页码正常切换 |
| 20 | 末页 → 点击下一页 | 按钮禁用 |
| 21 | 首页 → 点击上一页 | 按钮禁用 |

### 20.6 CRUD 功能

| # | 操作 | 预期 |
|---|------|------|
| 22 | 单表 → 点击添加行 | 末尾新增空白行 |
| 23 | 选中一行 → 点击删除行 | 确认后删除 |
| 24 | 编辑单元格 → 点击保存 | 文件更新 |
| 25 | 多表 JOIN 结果 → 点添加/删除/保存 | 提示"不支持编辑" |

### 20.7 字段大小写兼容性验证（Bugfix 回归）

| # | 输入 | 预期 |
|---|------|------|
| 26 | `select NAME,ID from tab_class` | 正常显示（大写字段匹配） |
| 27 | `select name,id from tab_class` | 正常显示（小写字段兼容） |
| 28 | `select Name,Id from tab_class` | 正常显示（混合大小写兼容） |
| 29 | `select tab_class.NAME from tab_class, tab_student where tab_class.ID=tab_student.CLAZZ_ID` | 多表大写字段正常 |
| 30 | `select tab_class.name from tab_class, tab_student where tab_class.id=tab_student.clazz_id` | 多表小写字段正常 |
| 31 | `select TAB_CLASS.NAME,TAB_STUDENT.NAME from TAB_CLASS,TAB_STUDENT where TAB_CLASS.ID=TAB_STUDENT.CLAZZ_ID and TAB_CLASS.ID=101` | 全大写多表正常 |

---

## 整体数据流总结

```
用户输入 SQL
    ↓
validateAndParseSql()                         ← 5 种语法错误检测
    ├── 规范化空白（多余空格容错）
    ├── 提取字段 / 表名 / WHERE 条件
    ├── 校验关键字 / 表存在 / 列名 / 引号
    │   └── 校验收敛：全部使用 indexOfCI()，字段名大小写不敏感
    └── 写入 m_queryFields / m_queryTables / m_queryConditions / m_joinHeader / m_fieldIndexes

    ↓
on_pushButton_clicked() 分发
    │
    ├── 单表 ──→ executeSingleTable()
    │            ├── loadCsvFile() 加载文件（全部列）
    │            ├── 展开 *  → 建立字段索引映射（indexOfCI, 大小写兼容）
    │            ├── checkConditions() 按 WHERE 过滤
    │            ├── m_allData 存完整行（含全部 CSV 列）
    │            ├── 设置表格（可编辑）+ 启用 CRUD
    │            ├── m_currentPage = 1
    │            └── showPageData() 分页渲染
    │
    └── 多表 ──→ executeMultiTable()
                 ├── loadCsvFile() 加载所有表
                 ├── 构建带前缀组合表头（table.field）
                 ├── 展开 *  → 映射 SELECT 列（indexOfCI, 大小写兼容）
                 ├── 笛卡尔积（嵌套循环, 百万行保护）
                 ├── checkConditions() 过滤
                 ├── 提取 SELECT 列 → m_allData
                 ├── 保存 m_joinHeader / m_csvHeader
                 ├── 设置表格（只读）+ 禁用 CRUD
                 └── showPageData() 分页渲染

用户编辑 + 翻页 / 添加 / 删除 / 保存
    └── syncTableToData()      ← 仅单表模式
         │   将表格编辑写回 m_allData（通过 m_fieldIndexes → m_csvHeader 映射，indexOfCI）
         │   while 补齐行长度确保不越界
         ├── 翻页: showPageData()
         ├── 添加: m_allData 追加空行 → showPageData()
         ├── 删除: 逆序移除 m_allData 元素 → showPageData()
         └── 保存: saveCsv() 将 m_csvHeader + m_allData 写回文件（QDir::mkpath 确保目录存在）
```
