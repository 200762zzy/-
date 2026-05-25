# =============================================================================
# classObject.pro — qmake 项目文件
# =============================================================================
# 这个文件告诉 qmake 如何构建项目：
#   - 使用了哪些 Qt 模块
#   - 哪些源代码需要编译
#   - 哪些 .ui 表单需要生成
# 运行方式: qmake classObject.pro  →  make
# =============================================================================

# --- Qt 模块声明 ------------------------------------------------------------
# core gui = QtCore + QtGui（所有 Qt 项目都需要）
QT       += core gui

# Qt5+ 之后 widgets 从 QtGui 独立为 QtWidgets 模块
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# --- C++ 标准 ---------------------------------------------------------------
CONFIG += c++11

# --- 弃用 API 警告 ----------------------------------------------------------
# 如果用了标记为弃用的 Qt API，编译时会弹出警告
DEFINES += QT_DEPRECATED_WARNINGS

# === 源代码文件清单 =========================================================

# .cpp 源文件
SOURCES += \
    main.cpp \
    mainwindow.cpp

# .h 头文件（qmake 会扫描依赖，帮助增量编译）
HEADERS += \
    mainwindow.h

# .ui 表单文件（Qt Designer 绘制，运行 qmake 时自动生成 ui_*.h）
FORMS += \
    mainwindow.ui

# === 部署规则（可忽略，仅嵌入式/移动端平台需要）===========================
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
