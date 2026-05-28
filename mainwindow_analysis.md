# `mainwindow.cpp` 逐函数逐语句分析

---

## 一、文件头 — 包含指令 (Lines 1–15)

| 行号 | 语句 | 作用 |
|------|------|------|
| 1 | `#include "mainwindow.h"` | 引入主窗口类声明（`WhereCondition` 结构体、成员变量与私有方法） |
| 2 | `#include "ui_mainwindow.h"` | 引入由 `mainwindow.ui` 自动生成的 UI 类，提供 `ui->` 访问界面控件 |
| 3–10 | `#include <QTableWidget/QMessageBox/...>` | 引入 Qt 核心控件 |
| 11 | `#include <QDir>` | 引入目录操作（`saveCsv` 中创建目录用） |
| 12 | `#include <QVBoxLayout>` | 引入布局操作（构造函数中设置间距用） |
| 13 | `#include <QItemSelectionModel>` | 引入选中模型（删除行获取选中行用） |
| 14 | `#include <QFont>` | 引入字体设置 |
| 15 | `#include <algorithm>` | 引入 `std::sort`，用于删除行时逆序排序 |

---

## 二、静态辅助函数 (Lines 17–66)

### `stripQuotes` (Lines 22–27)

**功能**：去除字符串两端双引号。

| 行号 | 语句 | 作用 |
|------|------|------|
| 24–25 | `s.length() >= 2 && s.starts/endsWith('"')` | 检测是否为 `"..."` 包裹的字符串 |
| 25 | `return s.mid(1, s.length() - 2)` | 去掉首尾引号 |

### `splitConditions` (Lines 30–66)

**功能**：按 `and` 分割 WHERE 条件字符串，忽略引号内的 `and`。

| 行号 | 语句 | 作用 |
|------|------|------|
| 33–34 | `QString current; bool inQuote = false;` | 累积当前条件片段；引号标记 |
| 36–41 | `for` 循环，检测 `"` 时翻转 `inQuote` | 跟踪是否在引号字符串内 |
| 43–56 | 不在引号内且遇到 `and` 单词 | 检查前后是否为空白/边界 → 是则分割，`i += 2` 跳过 |
| 58 | `current += c` | 非分割点则追加字符到当前片段 |
| 61–63 | 收尾：最后一个条件加入结果列表 |

---

## 三、构造函数 `MainWindow::MainWindow` (Lines 72–206)

### 构造初始化 (Lines 72–75)
- **`QMainWindow(parent)`** — 调用基类构造函数
- **`ui(new Ui::MainWindow)`** — 创建 UI 实例

### 控件设置 (Lines 76–98)

| 行号 | 语句 | 作用 |
|------|------|------|
| 89 | `m_pageSize = 10;` | 每页 10 行 |
| 90 | `m_currentPage = 1;` | 初始页码为 1 |
| 91 | `m_isMultiTable = false;` | 初始化多表标记为 false |
| 94–98 | `QVBoxLayout* mainLayout = ...; setContentsMargins(24,24,24,24); setSpacing(18);` | 设置主布局边距间距 |

### QSS 样式表 (Lines 100–205)
与旧版完全相同，不做赘述。

---

## 四、析构函数 `~MainWindow` (Lines 208–211)

```cpp
MainWindow::~MainWindow() { delete ui; }
```

与旧版完全相同。

---

## 五、`validateAndParseSql` — SQL 解析与验证 (Lines 226–455)

**功能**：完整解析 `SELECT field FROM table [WHERE cond]`，检测 5 种语法错误，结果写入 `m_queryFields` / `m_queryTables` / `m_queryConditions` / `m_isMultiTable` / `m_joinHeader` / `m_fieldIndexes`。

### 函数头及重置 (Lines 226–235)

| 行号 | 语句 | 作用 |
|------|------|------|
| 228–235 | `m_queryFields.clear(); m_queryTables.clear(); m_queryConditions.clear(); m_isMultiTable = false; m_joinHeader.clear(); m_fieldIndexes.clear();` | 重置所有查询状态变量 |

### 步骤 ① — 规范化空白 + 检查 SELECT（Lines 236–250）

| 行号 | 语句 | 作用 |
|------|------|------|
| 237 | `QString s = sql.simplified().trimmed();` | 合并连续空格 → 去除首尾空白（容错：多余空格） |
| 238–241 | `if (s.isEmpty())` | SQL 为空则报错 |
| 244–247 | `if (!s.startsWith("select", Qt::CaseInsensitive))` | **不区分大小写**检查 SELECT 关键字，错误类型 1 |
| 250 | `s = s.mid(6).trimmed();` | 去掉 "SELECT" 前缀 |

### 步骤 ② — 查找 FROM（Lines 252–262）

| 行号 | 语句 | 作用 |
|------|------|------|
| 256–257 | `QString lower = s.toLower(); fromIdx = lower.indexOf("from");` | 在小写版本中定位 "from" |
| 258–261 | `if (fromIdx < 0)` | 找不到 → 报错 |

### 步骤 ③ — 提取字段列表（Lines 264–286）

| 行号 | 语句 | 作用 |
|------|------|------|
| 265 | `QString fieldPart = s.left(fromIdx).trimmed();` | 截取字段部分 |
| 268–271 | `if (fieldPart.endsWith(","))` | 末尾逗号 → 错误类型 3 |
| 274–279 | `split(",", Qt::SkipEmptyParts)` → 逐项 `trimmed()` | 分割字段列表 |
| 281–283 | `if (m_queryFields.isEmpty())` | 空字段列表报错 |
| 286 | `bool isStar = (m_queryFields.size() == 1 && m_queryFields[0] == "*");` | 标记 `SELECT *` |

### 步骤 ④ — 提取 FROM 后内容（Lines 288–305）

| 行号 | 语句 | 作用 |
|------|------|------|
| 289–290 | `QString afterFrom = s.mid(fromIdx + 4).trimmed();` | 获取 "from" 后面的部分 |
| 293 | `int whereIdx = afterFromLower.indexOf("where");` | 查找 WHERE 关键字 |
| 296–301 | 分割 `tablePart` 和 `wherePart` | 有 where → 分割；无 where → 全部为表名 |
| 304–305 | 去除末尾分号 | 兼容 `;` 结尾 |

### 步骤 ⑤ — 解析表名（Lines 307–322）

| 行号 | 语句 | 作用 |
|------|------|------|
| 308 | `split(",", Qt::SkipEmptyParts)` | 逗号分割多个表 |
| 309–317 | 遍历，去除别名（空格后内容） | 支持 `FROM tab_class c` 语法 |
| 319–322 | `if (m_queryTables.isEmpty())` | 空表名报错（错误类型 2） |

### 步骤 ⑥ — 检查表文件存在（Lines 324–333）

| 行号 | 语句 | 作用 |
|------|------|------|
| 326–330 | `QFile file("D:/qt/Object/" + t + ".csv"); if (!file.exists())` | 检查磁盘文件是否存在，错误类型 2 |
| 333 | `m_isMultiTable = (m_queryTables.size() > 1);` | 标记单表/多表 |

### 步骤 ⑦ — 解析 WHERE 条件（Lines 335–399）

| 行号 | 语句 | 作用 |
|------|------|------|
| 337 | `QStringList condStrs = splitConditions(wherePart);` | 按 `and` 分割条件 |
| 343–352 | 确定操作符位置 | `>=`/`<=`/`<>` 优先于单字符 `=`/`>`/`<` |
| 359–361 | `left` / `op` / `right` 三部分 | 按操作符位置拆分 |
| 363–370 | 解析左侧 | 含 `.` 则分割为 `leftTable` + `leftField` |
| 372–396 | 解析右侧 | 引号 → 值；含 `.` → 字段引用；数值 → 数值；否则 → 错误类型 5 |
| 398 | `m_queryConditions << cond;` | 添加条件到列表 |

### 步骤 ⑧ — 检查字段名存在（Lines 402–452）

| 行号 | 语句 | 作用 |
|------|------|------|
| 404–406 | `if (isStar)` | `*` 跳过检查，执行时展开 |
| 407–439 | 多表：字段必须 `table.field` 格式 | 先验证表名在 FROM 列表中（L419–429），再验证字段在表头中存在 |
| 440–451 | 单表：直接检查字段在表头中 | 错误类型 4 |

---

## 六、`loadCsvFile` — 通用 CSV 加载 (Lines 461–488)

**功能**：读取 `D:/qt/Object/<tableName>.csv`，返回表头和数据。

| 行号 | 语句 | 作用 |
|------|------|------|
| 464–466 | `QFile file(...) → open()` | 以只读文本模式打开文件 |
| 468–469 | `QTextStream in(&file); in.setCodec("GBK");` | 创建文本流，编码 GBK |
| 471 | `QString headerLine = in.readLine().trimmed();` | 读取第一行（表头） |
| 472–475 | `if (headerLine.isEmpty())` | 空表头报错 |
| 477 | `header = headerLine.split(",", Qt::SkipEmptyParts);` | 按逗号拆分表头 |
| 480–483 | `while (!in.atEnd())` 逐行读取 | 跳过空行，按逗号拆分为 `QStringList` 追加到 `data`（`Qt::KeepEmptyParts` 保留空字段） |
| 486 | `file.close();` | 释放文件句柄 |

---

## 七、`compareValues` — 值比较 (Lines 494–524)

**功能**：比较两个值（优先数值比较，回退字符串比较）。

| 行号 | 语句 | 作用 |
|------|------|------|
| 500–502 | `l.toDouble(&lOk); r.toDouble(&rOk);` | 尝试转换为数值 |
| 504–511 | 两者均为数值 | 使用直接比较运算符（`==`/`!=`/`>`/`<`/`>=`/`<=`）进行浮点数值比较 |
| 514–521 | 字符串比较 | 使用 `QString::compare(Qt::CaseInsensitive)` |

---

## 八、`checkConditions` — 条件匹配 (Lines 531–582)

**功能**：对单行数据检查所有 WHERE 条件是否满足。

| 行号 | 语句 | 作用 |
|------|------|------|
| 538–540 | 构建左侧查询键 | 有 `leftTable` → `table.field`；无 → 直接 `field` |
| 543–549 | **手动遍历** header 数组，逐元素大小写不敏感比较 | 替代可能失效的 `indexOf`，在表头中定位列索引 |
| 553 | `QString leftVal = row[leftIdx].trimmed();` | 获取左侧值并去除空白 |
| 555–578 | 分支：字段间比较 / 字段-值比较 | 字段间 → 再次手动遍历查找右侧索引再比较；值 → 直接对比 |

---

## 九、`executeSingleTable` — 单表查询执行 (Lines 589–646)

**功能**：加载单表 CSV → WHERE 过滤 → 显示表格。

| 行号 | 语句 | 作用 |
|------|------|------|
| 594–599 | `loadCsvFile(tableName, header, rawData)` | 加载完整 CSV |
| 602 | `m_csvHeader = header;` | 保存完整表头（写回用） |
| 605–607 | `if (isStar) m_queryFields = header;` | `*` 展开为所有列 |
| 610–613 | 构建 `m_fieldIndexes` | 查询字段 → CSV 列索引映射 |
| 616–621 | `for (row : rawData) if (checkConditions(row, header)) m_allData << row;` | WHERE 过滤，存储**完整行** |
| 623–633 | 设置表格属性 | 可编辑、可多选、交替行色、列宽拉伸 |
| 636–638 | `btn_add/delete/save->setEnabled(true)` | 启用 CRUD |
| 640 | `m_currentPage = 1;` | 重置为第一页 |
| 641 | `showPageData();` | 分页渲染第一页 |
| 643–644 | 状态栏信息 | 显示表名和行数 |

---

## 十、`executeMultiTable` — 多表连接查询执行 (Lines 652–758)

**功能**：多表笛卡尔积 → WHERE 过滤 → 提取 SELECT 列 → 只读显示。

### 加载所有表 (Lines 658–667)

| 行号 | 语句 | 作用 |
|------|------|------|
| 658–667 | 遍历 `m_queryTables`，依次 `loadCsvFile()` | 加载全部表入内存 |

### 构建组合表头 (Lines 669–675)

| 行号 | 语句 | 作用 |
|------|------|------|
| 671–674 | 双层循环：表 → 字段 | 组合表头格式：`表名.字段名`（如 `tab_class.ID`） |

### 展开 + 索引映射 (Lines 677–691)

| 行号 | 语句 | 作用 |
|------|------|------|
| 678–680 | `*` 展开 | 替换为所有组合字段 |
| 682–691 | 映射 SELECT 字段 → 组合表头索引 | 找不到则报错 |

### 笛卡尔积 (Lines 694–719)

| 行号 | 语句 | 作用 |
|------|------|------|
| 697–700 | 从第一张表初始化 `product` | 所有行为原始行 |
| 703–719 | 依次交叉连接其余表 | 双层循环：现有行 × 新表行 → `combined << newRow` |
| 715–718 | 内存保护 | 超过 100 万行时终止并提示 |

### 过滤 + 提取 (Lines 721–730)

| 行号 | 语句 | 作用 |
|------|------|------|
| 722–730 | 逐行检查 `checkConditions(row, combinedHeader)` | 满足条件的提取 SELECT 列加入 `m_allData` |

### 保存状态 + 设置表格（只读）(Lines 732–757)

| 行号 | 语句 | 作用 |
|------|------|------|
| 733–734 | `m_joinHeader = combinedHeader; m_csvHeader = m_queryFields;` | 保存组合表头和查询字段（供 `showPageData` 使用） |
| 741 | `setEditTriggers(NoEditTriggers)` | 表格只读 |
| 748–750 | `btn_add/delete/save->setEnabled(false)` | 禁用 CRUD |
| 752–753 | `m_currentPage = 1; showPageData();` | 重置页码并渲染 |
| 755–756 | 状态栏信息 | 显示多表查询行数 |

---

## 十一、`on_pushButton_clicked` — 确定按钮 (Lines 764–778)

**功能**：入口函数，验证 → 分发。

| 行号 | 语句 | 作用 |
|------|------|------|
| 766 | `QString sql = ui->lineEdit->text();` | 获取输入框 SQL |
| 769–772 | `if (!validateAndParseSql(sql, errorMsg))` | 校验失败 → 弹窗报错并 return |
| 774–777 | `if (m_isMultiTable) executeMultiTable() else executeSingleTable()` | 按单表/多表分发执行 |

---

## 十二、`showPageData` — 分页渲染 (Lines 784–835)

**功能**：根据当前页码和页大小，从 `m_allData` 中截取数据填充到表格。

| 行号 | 语句 | 作用 |
|------|------|------|
| 786–791 | `if (m_queryFields.isEmpty())` | 无查询字段时显示"无数据"并禁用翻页 |
| 793–797 | 重置表格列结构 | 清空 → 设置列数 → 设表头 → 列宽拉伸 |
| 799–804 | `if (m_allData.isEmpty())` | 空数据显示"第 1 页 / 共 0 页" |
| 806–807 | `start = (page-1)*pageSize; end = qMin(...)` | 计算当前页起止索引 |
| 809–828 | 双层循环填充表格 | **多表分支**（L814–818）：`dataRow` 已为 SELECT 列，直接显示；**单表分支**（L819–826）：通过 `m_fieldIndexes` 从完整行提取显示列 |
| 830–834 | 页码标签 + 按钮状态 | 同旧版 |

---

## 十三、`syncTableToData` — 编辑同步 (Lines 841–866)

**功能**：将表格编辑写回 `m_allData`（仅单表模式有效）。

| 行号 | 语句 | 作用 |
|------|------|------|
| 844 | `if (m_isMultiTable || m_queryFields.isEmpty() || m_csvHeader.isEmpty()) return;` | 多表查询跳过同步（只读） |
| 848–851 | 构建 `fieldToCsv` 映射 | 查询字段 → CSV 全列索引 |
| 853–865 | 双层循环写回 | 遍历可见行 → 计算全局行号 → 映射列 → **补齐行长度（`while` 添加空串）** → 写回 `m_allData` |

---

## 十四、`on_btn_pre_clicked` — 上一页 (Lines 872–879)

| 行号 | 语句 | 作用 |
|------|------|------|
| 874–878 | `syncTableToData()` → `if (m_currentPage > 1) m_currentPage--` → `showPageData()` | 先同步编辑，再翻页 |

---

## 十五、`on_btn_next_clicked` — 下一页 (Lines 885–893)

| 行号 | 语句 | 作用 |
|------|------|------|
| 887–892 | `syncTableToData()` → 重新计算 `totalPage` → `if (m_currentPage < totalPage) m_currentPage++` → `showPageData()` | 先同步编辑，再翻页（防止最后一页越界） |

---

## 十六、`on_btn_add_clicked` — 添加行 (Lines 899–918)

| 行号 | 语句 | 作用 |
|------|------|------|
| 901–903 | `if (m_isMultiTable)` | 多表模式禁止编辑 |
| 905–908 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示 |
| 909 | `syncTableToData()` | 先同步当前编辑 |
| 911–914 | 构造全空行追加到 `m_allData` | 长度 = CSV 列数 |
| 916 | `m_currentPage = ...` | 跳转到最后一页（确保用户能看到新行） |
| 917 | `showPageData()` | 重新渲染 |

---

## 十七、`on_btn_delete_clicked` — 删除行 (Lines 924–957)

| 行号 | 语句 | 作用 |
|------|------|------|
| 926–928 | `if (m_isMultiTable)` | 多表模式禁止编辑 |
| 930–933 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示 |
| 934 | `syncTableToData()` | 同步编辑 |
| 936–939 | `selectedRows()` → 空则警告 | 获取选中行 |
| 942–943 | 确认对话框 | 防误删除 |
| 945–951 | 逆序删除 | `globalRow = (page-1)*pageSize + row` → `std::sort` 逆序 → `removeAt()` |
| 953–956 | 调整页码 → 刷新 | 防止空页 |

---

## 十八、`on_btn_save_clicked` — 保存 CSV (Lines 963–980)

| 行号 | 语句 | 作用 |
|------|------|------|
| 965–967 | `if (m_isMultiTable)` | 多表模式禁止保存 |
| 969–972 | `if (m_csvHeader.isEmpty())` | 无数据时提示 |
| 973 | `syncTableToData()` | 同步编辑 |
| 975 | `QString tableName = m_queryTables.isEmpty() ? "" : m_queryTables[0];` | 从存储状态取表名（不再重新解析 SQL） |
| 976–979 | `if (saveCsv(tableName))` | 调用保存函数，成功则更新状态栏显示"保存成功！"并弹 `QMessageBox` 提示 |

---

## 十九、`saveCsv` — 写入 CSV 文件 (Lines 986–1008)

| 行号 | 语句 | 作用 |
|------|------|------|
| 990–991 | `QDir dir("D:/qt/Object/"); if (!dir.exists()) dir.mkpath(".");` | 确保目标目录存在 |
| 993–996 | `QFile(...) → open(WriteOnly)` | 以只写文本模式打开文件 |
| 999–1000 | `QTextStream out(&file); out.setCodec("GBK");` | 创建文本流，编码 GBK |
| 1001 | `out << m_csvHeader.join(",") << "\n";` | 写入表头行 |
| 1003–1004 | `for (row : m_allData) out << row.join(",") << "\n";` | 逐行写入数据 |
| 1006 | `file.close();` | 释放文件句柄 |

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
| 8 | `SELECT ID , NAME FROM TAB_CLASS`（大写） | 正常显示 |
| 9 | `select * from tab_student where sex = "男"`（标准引号） | 仅显示男生 |

### 20.3 单表查询

| # | 输入 | 预期 |
|---|------|------|
| 10 | `select * from tab_class` | 显示全部列 + 全部行 |
| 11 | `select ID,NAME from tab_class` | 仅 ID、NAME 两列 |
| 12 | `select * from tab_student where sex="男"` | 仅男生行 |
| 13 | `select * from tab_score where result>=90` | 成绩 ≥ 90 的行 |
| 14 | `select * from tab_course where name="数据结构"` | 返回数据结构课程 |

### 20.4 多表 JOIN

| # | 输入 | 预期 |
|---|------|------|
| 15 | `SELECT tab_class.name,tab_student.name from tab_class, tab_student where tab_class.id=tab_student.clazz_id and tab_class.id=101` | 101 班所有学生姓名 |
| 16 | `SELECT tab_class.name,tab_student.name,tab_course.name,tab_score.result from tab_class,tab_student,tab_course,tab_score where tab_class.id=101 and tab_course.name="C语言程序设计" and tab_student.clazz_id = tab_class.id and tab_score.stu_id = tab_student.id and tab_score.course_id=tab_course.id` | 101 班 C 语言成绩表 |
| 17 | `SELECT * from tab_class, tab_student where tab_class.id=tab_student.clazz_id` | 班级+学生全连接（只读） |

### 20.5 分页功能

| # | 操作 | 预期 |
|---|------|------|
| 18 | `select * from tab_student` → 点击下一页 | 每页 10 行，页码正常切换 |
| 19 | 末页 → 点击下一页 | 按钮禁用 |
| 20 | 首页 → 点击上一页 | 按钮禁用 |

### 20.6 CRUD 功能

| # | 操作 | 预期 |
|---|------|------|
| 21 | 单表 → 点击添加行 | 末尾新增空白行 |
| 22 | 选中一行 → 点击删除行 | 确认后删除 |
| 23 | 编辑单元格 → 点击保存 | 文件更新 |
| 24 | 多表 JOIN 结果 → 点添加/删除/保存 | 提示"不支持编辑" |

---

## 整体数据流总结

```
用户输入 SQL
    ↓
validateAndParseSql()                         ← 5 种语法错误检测
    ├── 规范化空白（多余空格容错）
    ├── 提取字段 / 表名 / WHERE 条件
    ├── 校验关键字 / 表存在 / 列名 / 引号
    └── 写入 m_queryFields / m_queryTables / m_queryConditions / m_joinHeader / m_fieldIndexes

    ↓
on_pushButton_clicked() 分发
    │
    ├── 单表 ──→ executeSingleTable()
    │            ├── loadCsvFile() 加载文件（全部列）
    │            ├── 展开 *  → 建立字段索引映射
    │            ├── checkConditions() 按 WHERE 过滤
    │            ├── m_allData 存完整行（含全部 CSV 列）
    │            ├── 设置表格（可编辑）+ 启用 CRUD
    │            ├── m_currentPage = 1
    │            └── showPageData() 分页渲染
    │
    └── 多表 ──→ executeMultiTable()
                 ├── loadCsvFile() 加载所有表
                 ├── 构建带前缀组合表头（table.field）
                 ├── 展开 *  → 映射 SELECT 列
                 ├── 笛卡尔积（嵌套循环, 百万行保护）
                 ├── checkConditions() 过滤
                 ├── 提取 SELECT 列 → m_allData
                 ├── 保存 m_joinHeader / m_csvHeader
                 ├── 设置表格（只读）+ 禁用 CRUD
                 └── showPageData() 分页渲染

用户编辑 + 翻页 / 添加 / 删除 / 保存
    └── syncTableToData()      ← 仅单表模式
         │   将表格编辑写回 m_allData（通过 m_fieldIndexes → m_csvHeader 映射）
         │   while 补齐行长度确保不越界
         ├── 翻页: showPageData()
         ├── 添加: m_allData 追加空行 → showPageData()
         ├── 删除: 逆序移除 m_allData 元素 → showPageData()
         └── 保存: saveCsv() 将 m_csvHeader + m_allData 写回文件（QDir::mkpath 确保目录存在）
```
