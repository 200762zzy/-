QT += core gui network xml testlib widgets

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = test_classObject2

INCLUDEPATH += ..

SOURCES += \
    test_main.cpp \
    test_docxhelper.cpp \
    test_deepseekapi.cpp \
    ../deepseekapi.cpp \
    ../docxhelper.cpp \
    ../workerthread.cpp

HEADERS += \
    test_docxhelper.h \
    test_deepseekapi.h \
    ../deepseekapi.h \
    ../docxhelper.h \
    ../workerthread.h

# Windows 专用：编译后自动复制 OpenSSL 1.1 DLL
win32 {
    SSL_DIR = $$clean_path($$[QT_INSTALL_PREFIX]/../../Tools/QtCreator/bin)
    MINGW_BIN = $$[QT_INSTALL_PREFIX]/bin

    CONFIG(debug, debug|release) {
        TARGET_DIR = $$OUT_PWD/debug/
    } else {
        TARGET_DIR = $$OUT_PWD/release/
    }

    QMAKE_POST_LINK += $${QMAKE_COPY} $$SSL_DIR/libcrypto-1_1.dll $$TARGET_DIR $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${QMAKE_COPY} $$SSL_DIR/libssl-1_1.dll $$TARGET_DIR
}
