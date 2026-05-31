#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 现代渲染：Fusion 风格跨平台统一 + 全局字体
    a.setStyle(QStyleFactory::create("Fusion"));
    a.setFont(QFont("Microsoft YaHei UI", 9));

    MainWindow w;
    w.show();
    return a.exec();
}
