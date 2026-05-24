#include "widget.h"

#include <QApplication>
//点击按钮关闭窗口
//按钮
//被点击
//窗口
//关闭

//建立四者关系
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setFixedSize(400,1110);

    w.show();
    return a.exec();
}
