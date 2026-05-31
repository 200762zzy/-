QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    deepseekapi.cpp \
    docxhelper.cpp \
    workerthread.cpp

HEADERS += \
    mainwindow.h \
    deepseekapi.h \
    docxhelper.h \
    workerthread.h

# ==============================================
# Windows 专用：编译后自动复制缺少的 DLL
# libcrypto-1_1/libssl-1_1: OpenSSL 1.1 DLL（Qt 5.14.2 MinGW 要求 OpenSSL 1.1.x）
# libgcc_s_dw2 / libstdc++-6: Qt5Network 的 MinGW 运行时依赖
# ==============================================
win32 {
    # OpenSSL 1.1 DLL 所在目录（Qt Creator 自带，与 Qt 版本匹配）
    SSL_DIR = $$clean_path($$[QT_INSTALL_PREFIX]/../../Tools/QtCreator/bin)
    # Qt 的 MinGW bin 目录（libgcc_s_dw2 / libstdc++ 所在）
    MINGW_BIN = $$[QT_INSTALL_PREFIX]/bin

    # 根据构建配置选择目标目录（只复制到当前配置的目录）
    CONFIG(debug, debug|release) {
        TARGET_DIR = $$OUT_PWD/debug/
    } else {
        TARGET_DIR = $$OUT_PWD/release/
    }

    # MinGW 的 cp 接受 Windows 路径（D:/...），不接受 MSYS 路径（/D/...）
    # 所以不用 $$shell_path()，直接用原始的正斜杠路径
    QMAKE_POST_LINK += $${QMAKE_COPY} $$SSL_DIR/libcrypto-1_1.dll $$TARGET_DIR $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${QMAKE_COPY} $$SSL_DIR/libssl-1_1.dll $$TARGET_DIR $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${QMAKE_COPY} $$MINGW_BIN/libgcc_s_dw2-1.dll $$TARGET_DIR $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${QMAKE_COPY} $$MINGW_BIN/libstdc++-6.dll $$TARGET_DIR
}


