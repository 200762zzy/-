# classObject

Qt 5.14.2 qmake project (MinGW 32-bit), C++11, single-window widget app.

## Build

```
qmake classObject.pro
make
```

Build directory: `D:/qt/build-classObject-Desktop_Qt_5_14_2_MinGW_32_bit-Debug/`

## Non-obvious gotchas

- **Hardcoded data path** — CSV files are loaded from `C:/ClassObject/<tableName>.csv`. The app will fail silently if this directory or files don't exist.
- **Simulated SQL** — The `SELECT field1,field2 FROM tablename` syntax is parsed locally; it just reads a matching CSV and filters columns by header name. No real database.
- **No tests, no CI, no lint/typecheck** — nothing beyond building.
- **UI generated** — `mainwindow.ui` is a Qt Designer form; editing requires re-running qmake to regenerate `ui_mainwindow.h`.
- **`QTextStream::setCodec`** — Qt5-specific API call in `mainwindow.cpp`. No equivalent in Qt6 (removed).
- **CRUD toolbar is in .ui** — The 添加行/删除行/保存 buttons (`btn_add`, `btn_delete`, `btn_save`) are defined in `mainwindow.ui` and auto-connected to `on_btn_*_clicked()` slots via Qt's auto-connect mechanism.
- **Edits lost on new query** — Clicking "确定" (new SQL query) reloads data from scratch and discards unsaved in-memory changes. Always save before switching tables.

## Project structure

| File | Role |
|------|------|
| `main.cpp` | App entrypoint; global font set to `Microsoft YaHei UI` |
| `mainwindow.h/.cpp` | Main window, SQL parser, CSV CRUD, pagination, modern QSS theme |
| `mainwindow.ui` | Qt Designer UI form (welcome label, table, pagination bar, SQL input bar) |
| `classObject.pro` | qmake project file |

## 角色

你是一个资深的qt工程师，精通ui界面的编辑 信号和槽

## 代码规范

1.写出代码都有注释

2.函数，按照

```
void print_Array(){

}
```

这样的风格