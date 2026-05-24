#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w1;
    w1.setWindowTitle("窗口1");
    w1.setFixedSize(1999,1344);

    w1.show();

    return a.exec();
}
