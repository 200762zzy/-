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

## 二、静态辅助函数 (Lines 17–75)

### `indexOfCI` (Lines 21–28)

**功能**：QStringList 大小写不敏感查找。`QStringList::indexOf` 不接受 `Qt::CaseSensitivity` 参数，只能手动遍历 + `QString::compare(CaseInsensitive)` 实现。

| 行号 | 语句 | 作用 |
|------|------|------|
| 21 | `// QStringList 大小写不敏感查找...` | 注释说明为什么需要独立函数 |
| 22–28 | `static int indexOfCI(...)` 实现 | `for` 遍历列表，`compare(value, CaseInsensitive)` 比较，找到返回索引，否则 `-1` |

### `stripQuotes` (Lines 30–35)

**功能**：去除字符串两端双引号。

| 行号 | 语句 | 作用 |
|------|------|------|
| 32–33 | `s.length() >= 2 && s.starts/endsWith('"')` | 检测是否为 `"..."` 包裹的字符串 |
| 33 | `return s.mid(1, s.length() - 2)` | 去掉首尾引号 |

### `splitConditions` (Lines 38–75)

**功能**：按 `and` 分割 WHERE 条件字符串，忽略引号内的 `and`。

| 行号 | 语句 | 作用 |
|------|------|------|
| 41–42 | `QString current; bool inQuote = false;` | 累积当前条件片段；引号标记 |
| 44–49 | `for` 循环，检测 `"` 时翻转 `inQuote` | 跟踪是否在引号字符串内 |
| 51–64 | 不在引号内且遇到 `and` 单词 | 检查前后是否为空白/边界 → 是则分割，`i += 2` 跳过 |
| 66 | `current += c` | 非分割点则追加字符到当前片段 |
| 69–71 | 收尾：最后一个条件加入结果列表 |

---

## 三、构造函数 `MainWindow::MainWindow` (Lines 81–215)

### 构造初始化 (Lines 81–84)
- **`QMainWindow(parent)`** — 调用基类构造函数
- **`ui(new Ui::MainWindow)`** — 创建 UI 实例

### 控件设置 (Lines 85–107)

| 行号 | 语句 | 作用 |
|------|------|------|
| 98 | `m_pageSize = 10;` | 每页 10 行 |
| 99 | `m_currentPage = 1;` | 初始页码为 1 |
| 100 | `m_isMultiTable = false;` | 初始化多表标记为 false |
| 103–107 | `QVBoxLayout* mainLayout = ...; setContentsMargins(24,24,24,24); setSpacing(18);` | 设置主布局边距间距 |

### QSS 样式表 (Lines 109–214)
与旧版完全相同，不做赘述。

---

## 四、析构函数 `~MainWindow` (Lines 217–220)

```cpp
MainWindow::~MainWindow() { delete ui; }
```

与旧版完全相同。

---

## 五、`validateAndParseSql` — SQL 解析与验证 (Lines 235–464)

**功能**：完整解析 `SELECT field FROM table [WHERE cond]`，检测 5 种语法错误，结果写入 `m_queryFields` / `m_queryTables` / `m_queryConditions` / `m_isMultiTable` / `m_joinHeader` / `m_fieldIndexes`。

### 函数头及重置 (Lines 235–244)

| 行号 | 语句 | 作用 |
|------|------|------|
| 237–244 | `m_queryFields.clear(); m_queryTables.clear(); m_queryConditions.clear(); m_isMultiTable = false; m_joinHeader.clear(); m_fieldIndexes.clear();` | 重置所有查询状态变量。全部使用 `indexOfCI()` 进行字段查找，**字段名大小写不敏感** |

### 步骤 ① — 规范化空白 + 检查 SELECT（Lines 245–259）

| 行号 | 语句 | 作用 |
|------|------|------|
| 246 | `QString s = sql.simplified().trimmed();` | 合并连续空格 → 去除首尾空白（容错：多余空格） |
| 247–250 | `if (s.isEmpty())` | SQL 为空则报错 |
| 253–256 | `if (!s.startsWith("select", Qt::CaseInsensitive))` | **不区分大小写**检查 SELECT 关键字，错误类型 1 |
| 259 | `s = s.mid(6).trimmed();` | 去掉 "SELECT" 前缀 |

### 步骤 ② — 查找 FROM（Lines 261–271）

| 行号 | 语句 | 作用 |
|------|------|------|
| 265–266 | `QString lower = s.toLower(); fromIdx = lower.indexOf("from");` | 在小写版本中定位 "from" |
| 267–270 | `if (fromIdx < 0)` | 找不到 → 报错 |

### 步骤 ③ — 提取字段列表（Lines 273–295）

| 行号 | 语句 | 作用 |
|------|------|------|
| 274 | `QString fieldPart = s.left(fromIdx).trimmed();` | 截取字段部分 |
| 277–280 | `if (fieldPart.endsWith(","))` | 末尾逗号 → 错误类型 3 |
| 283–288 | `split(",", Qt::SkipEmptyParts)` → 逐项 `trimmed()` | 分割字段列表 |
| 290–292 | `if (m_queryFields.isEmpty())` | 空字段列表报错 |
| 295 | `bool isStar = (m_queryFields.size() == 1 && m_queryFields[0] == "*");` | 标记 `SELECT *` |

### 步骤 ④ — 提取 FROM 后内容（Lines 297–314）

| 行号 | 语句 | 作用 |
|------|------|------|
| 298–299 | `QString afterFrom = s.mid(fromIdx + 4).trimmed();` | 获取 "from" 后面的部分 |
| 302 | `int whereIdx = afterFromLower.indexOf("where");` | 查找 WHERE 关键字 |
| 305–310 | 分割 `tablePart` 和 `wherePart` | 有 where → 分割；无 where → 全部为表名 |
| 313–314 | 去除末尾分号 | 兼容 `;` 结尾 |

### 步骤 ⑤ — 解析表名（Lines 316–331）

| 行号 | 语句 | 作用 |
|------|------|------|
| 317 | `split(",", Qt::SkipEmptyParts)` | 逗号分割多个表 |
| 318–326 | 遍历，去除别名（空格后内容） | 支持 `FROM tab_class c` 语法 |
| 328–331 | `if (m_queryTables.isEmpty())` | 空表名报错（错误类型 2） |

### 步骤 ⑥ — 检查表文件存在（Lines 333–342）

| 行号 | 语句 | 作用 |
|------|------|------|
| 335–339 | `QFile file("D:/qt/Object/" + t + ".csv"); if (!file.exists())` | 检查磁盘文件是否存在，错误类型 2 |
| 342 | `m_isMultiTable = (m_queryTables.size() > 1);` | 标记单表/多表 |

### 步骤 ⑦ — 解析 WHERE 条件（Lines 344–408）

| 行号 | 语句 | 作用 |
|------|------|------|
| 346 | `QStringList condStrs = splitConditions(wherePart);` | 按 `and` 分割条件 |
| 352–361 | 确定操作符位置 | `>=`/`<=`/`<>` 优先于单字符 `=`/`>`/`<` |
| 368–370 | `left` / `op` / `right` 三部分 | 按操作符位置拆分 |
| 372–379 | 解析左侧 | 含 `.` 则分割为 `leftTable` + `leftField` |
| 381–405 | 解析右侧 | 引号 → 值；含 `.` → 字段引用；数值 → 数值；否则 → 错误类型 5 |
| 407 | `m_queryConditions << cond;` | 添加条件到列表 |

### 步骤 ⑧ — 检查字段名存在（Lines 411–461）

| 行号 | 语句 | 作用 |
|------|------|------|
| 413–415 | `if (isStar)` | `*` 跳过检查，执行时展开 |
| 416–448 | 多表：字段必须 `table.field` 格式 | 先验证表名在 FROM 列表中（L428–438），再用 `indexOfCI()` 验证字段在表头中存在（L443），**大小写兼容** |
| 449–460 | 单表：直接用 `indexOfCI()` 检查字段在表头中 | 错误类型 4，**大小写兼容** |

---

## 六、`loadCsvFile` — 通用 CSV 加载 (Lines 470–497)

**功能**：读取 `D:/qt/Object/<tableName>.csv`，返回表头和数据。

| 行号 | 语句 | 作用 |
|------|------|------|
| 473–475 | `QFile file(...) → open()` | 以只读文本模式打开文件 |
| 477–478 | `QTextStream in(&file); in.setCodec("GBK");` | 创建文本流，编码 GBK |
| 480 | `QString headerLine = in.readLine().trimmed();` | 读取第一行（表头） |
| 481–484 | `if (headerLine.isEmpty())` | 空表头报错 |
| 486 | `header = headerLine.split(",", Qt::SkipEmptyParts);` | 按逗号拆分表头 |
| 489–492 | `while (!in.atEnd())` 逐行读取 | 跳过空行，按逗号拆分为 `QStringList` 追加到 `data`（`Qt::KeepEmptyParts` 保留空字段） |
| 495 | `file.close();` | 释放文件句柄 |

---

## 七、`compareValues` — 值比较 (Lines 503–533)

**功能**：比较两个值（优先数值比较，回退字符串比较）。

| 行号 | 语句 | 作用 |
|------|------|------|
| 509–511 | `l.toDouble(&lOk); r.toDouble(&rOk);` | 尝试转换为数值 |
| 513–520 | 两者均为数值 | 使用直接比较运算符（`==`/`!=`/`>`/`<`/`>=`/`<=`）进行浮点数值比较 |
| 523–530 | 字符串比较 | 使用 `QString::compare(Qt::CaseInsensitive)` |

---

## 八、`checkConditions` — 条件匹配 (Lines 540–591)

**功能**：对单行数据检查所有 WHERE 条件是否满足。

| 行号 | 语句 | 作用 |
|------|------|------|
| 547–549 | 构建左侧查询键 | 有 `leftTable` → `table.field`；无 → 直接 `field` |
| 552–558 | **手动遍历** header 数组，逐元素大小写不敏感比较 | 替代可能失效的 `indexOf`，在表头中定位列索引 |
| 562 | `QString leftVal = row[leftIdx].trimmed();` | 获取左侧值并去除空白 |
| 564–587 | 分支：字段间比较 / 字段-值比较 | 字段间 → 再次手动遍历查找右侧索引再比较；值 → 直接对比 |

---

## 九、`executeSingleTable` — 单表查询执行 (Lines 598–655)

**功能**：加载单表 CSV → WHERE 过滤 → 显示表格。

| 行号 | 语句 | 作用 |
|------|------|------|
| 603–608 | `loadCsvFile(tableName, header, rawData)` | 加载完整 CSV |
| 611 | `m_csvHeader = header;` | 保存完整表头（写回用） |
| 614–616 | `if (isStar) m_queryFields = header;` | `*` 展开为所有列 |
| 618–622 | 构建 `m_fieldIndexes`（使用 `indexOfCI()`） | 查询字段 → CSV 列索引映射，**大小写兼容** |
| 625–630 | `for (row : rawData) if (checkConditions(row, header)) m_allData << row;` | WHERE 过滤，存储**完整行** |
| 632–642 | 设置表格属性 | 可编辑、可多选、交替行色、列宽拉伸 |
| 645–647 | `btn_add/delete/save->setEnabled(true)` | 启用 CRUD |
| 649 | `m_currentPage = 1;` | 重置为第一页 |
| 650 | `showPageData();` | 分页渲染第一页 |
| 652–653 | 状态栏信息 | 显示表名和行数 |

---

## 十、`executeMultiTable` — 多表连接查询执行 (Lines 661–767)

**功能**：多表笛卡尔积 → WHERE 过滤 → 提取 SELECT 列 → 只读显示。

### 加载所有表 (Lines 667–676)

| 行号 | 语句 | 作用 |
|------|------|------|
| 667–676 | 遍历 `m_queryTables`，依次 `loadCsvFile()` | 加载全部表入内存 |

### 构建组合表头 (Lines 678–684)

| 行号 | 语句 | 作用 |
|------|------|------|
| 680–683 | 双层循环：表 → 字段 | 组合表头格式：`表名.字段名`（如 `tab_class.ID`） |

### 展开 + 索引映射 (Lines 686–700)

| 行号 | 语句 | 作用 |
|------|------|------|
| 687–689 | `*` 展开 | 替换为所有组合字段 |
| 691–700 | 映射 SELECT 字段 → 组合表头索引（使用 `indexOfCI()`） | 找不到则报错，**大小写兼容** |

### 笛卡尔积 (Lines 702–728)

| 行号 | 语句 | 作用 |
|------|------|------|
| 706–709 | 从第一张表初始化 `product` | 所有行为原始行 |
| 712–728 | 依次交叉连接其余表 | 双层循环：现有行 × 新表行 → `combined << newRow` |
| 724–727 | 内存保护 | 超过 100 万行时终止并提示 |

### 过滤 + 提取 (Lines 730–739)

| 行号 | 语句 | 作用 |
|------|------|------|
| 731–739 | 逐行检查 `checkConditions(row, combinedHeader)` | 满足条件的提取 SELECT 列加入 `m_allData` |

### 保存状态 + 设置表格（只读）(Lines 741–767)

| 行号 | 语句 | 作用 |
|------|------|------|
| 742–743 | `m_joinHeader = combinedHeader; m_csvHeader = m_queryFields;` | 保存组合表头和查询字段（供 `showPageData` 使用） |
| 750 | `setEditTriggers(NoEditTriggers)` | 表格只读 |
| 757–759 | `btn_add/delete/save->setEnabled(false)` | 禁用 CRUD |
| 761–762 | `m_currentPage = 1; showPageData();` | 重置页码并渲染 |
| 764–765 | 状态栏信息 | 显示多表查询行数 |

---

## 十一、`on_pushButton_clicked` — 确定按钮 (Lines 773–787)

**功能**：入口函数，验证 → 分发。

| 行号 | 语句 | 作用 |
|------|------|------|
| 775 | `QString sql = ui->lineEdit->text();` | 获取输入框 SQL |
| 778–781 | `if (!validateAndParseSql(sql, errorMsg))` | 校验失败 → 弹窗报错并 return |
| 783–786 | `if (m_isMultiTable) executeMultiTable() else executeSingleTable()` | 按单表/多表分发执行 |

---

## 十二、`showPageData` — 分页渲染 (Lines 793–844)

**功能**：根据当前页码和页大小，从 `m_allData` 中截取数据填充到表格。

| 行号 | 语句 | 作用 |
|------|------|------|
| 795–800 | `if (m_queryFields.isEmpty())` | 无查询字段时显示"无数据"并禁用翻页 |
| 802–806 | 重置表格列结构 | 清空 → 设置列数 → 设表头 → 列宽拉伸 |
| 808–813 | `if (m_allData.isEmpty())` | 空数据显示"第 1 页 / 共 0 页" |
| 815–816 | `start = (page-1)*pageSize; end = qMin(...)` | 计算当前页起止索引 |
| 818–837 | 双层循环填充表格 | **多表分支**（L823–827）：`dataRow` 已为 SELECT 列，直接显示；**单表分支**（L828–835）：通过 `m_fieldIndexes` 从完整行提取显示列 |
| 839–843 | 页码标签 + 按钮状态 | 同旧版 |

---

## 十三、`syncTableToData` — 编辑同步 (Lines 850–875)

**功能**：将表格编辑写回 `m_allData`（仅单表模式有效）。

| 行号 | 语句 | 作用 |
|------|------|------|
| 853 | `if (m_isMultiTable || m_queryFields.isEmpty() || m_csvHeader.isEmpty()) return;` | 多表查询跳过同步（只读） |
| 856–860 | 构建 `fieldToCsv` 映射（使用 `indexOfCI()`） | 查询字段 → CSV 全列索引，**大小写兼容** |
| 862–874 | 双层循环写回 | 遍历可见行 → 计算全局行号 → 映射列 → **补齐行长度（`while` 添加空串）** → 写回 `m_allData` |

---

## 十四、`on_btn_pre_clicked` — 上一页 (Lines 881–888)

| 行号 | 语句 | 作用 |
|------|------|------|
| 883–887 | `syncTableToData()` → `if (m_currentPage > 1) m_currentPage--` → `showPageData()` | 先同步编辑，再翻页 |

---

## 十五、`on_btn_next_clicked` — 下一页 (Lines 894–902)

| 行号 | 语句 | 作用 |
|------|------|------|
| 896–901 | `syncTableToData()` → 重新计算 `totalPage` → `if (m_currentPage < totalPage) m_currentPage++` → `showPageData()` | 先同步编辑，再翻页（防止最后一页越界） |

---

## 十六、`on_btn_add_clicked` — 添加行 (Lines 908–927)

| 行号 | 语句 | 作用 |
|------|------|------|
| 910–912 | `if (m_isMultiTable)` | 多表模式禁止编辑 |
| 914–917 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示 |
| 918 | `syncTableToData()` | 先同步当前编辑 |
| 920–923 | 构造全空行追加到 `m_allData` | 长度 = CSV 列数 |
| 925 | `m_currentPage = ...` | 跳转到最后一页（确保用户能看到新行） |
| 926 | `showPageData()` | 重新渲染 |

---

## 十七、`on_btn_delete_clicked` — 删除行 (Lines 933–966)

| 行号 | 语句 | 作用 |
|------|------|------|
| 935–937 | `if (m_isMultiTable)` | 多表模式禁止编辑 |
| 939–942 | `if (m_csvHeader.isEmpty())` | 未加载数据表时提示 |
| 943 | `syncTableToData()` | 同步编辑 |
| 945–948 | `selectedRows()` → 空则警告 | 获取选中行 |
| 951–952 | 确认对话框 | 防误删除 |
| 954–960 | 逆序删除 | `globalRow = (page-1)*pageSize + row` → `std::sort` 逆序 → `removeAt()` |
| 962–965 | 调整页码 → 刷新 | 防止空页 |

---

## 十八、`on_btn_save_clicked` — 保存 CSV (Lines 972–989)

| 行号 | 语句 | 作用 |
|------|------|------|
| 974–976 | `if (m_isMultiTable)` | 多表模式禁止保存 |
| 978–981 | `if (m_csvHeader.isEmpty())` | 无数据时提示 |
| 982 | `syncTableToData()` | 同步编辑 |
| 984 | `QString tableName = m_queryTables.isEmpty() ? "" : m_queryTables[0];` | 从存储状态取表名（不再重新解析 SQL） |
| 985–988 | `if (saveCsv(tableName))` | 调用保存函数，成功则更新状态栏显示"保存成功！"并弹 `QMessageBox` 提示 |

---

## 十九、`saveCsv` — 写入 CSV 文件 (Lines 995–1017)

| 行号 | 语句 | 作用 |
|------|------|------|
| 999–1000 | `QDir dir("D:/qt/Object/"); if (!dir.exists()) dir.mkpath(".");` | 确保目标目录存在 |
| 1002–1005 | `QFile(...) → open(WriteOnly)` | 以只写文本模式打开文件 |
| 1008–1009 | `QTextStream out(&file); out.setCodec("GBK");` | 创建文本流，编码 GBK |
| 1010 | `out << m_csvHeader.join(",") << "\n";` | 写入表头行 |
| 1012–1013 | `for (row : m_allData) out << row.join(",") << "\n";` | 逐行写入数据 |
| 1015 | `file.close();` | 释放文件句柄 |

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
