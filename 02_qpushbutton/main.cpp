#include "widget.h"
#include<QPushButton>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    //添加按钮
//    QPushButton btn;
//    btn.setText("按钮1");
//    btn.show();
    //默认情况下没有建立父子关系的，显示都是顶层窗口
    //要建立父子关系
    //1.setParent函数
    QPushButton btn;
    btn.setText("按钮1");
    btn.setParent(&w);
    //2.构造函数传参
    QPushButton btn2("按钮2",&w);
    // 移动以下按钮位置
    btn2.move(100,100);

    btn2.resize(400,400);

    //按钮3和按钮2建立父子关系
    QPushButton btn3("按钮3",&btn2);

    //移动按钮/窗口位置
    btn3.move(100,100);
    //设置按钮大小
    btn3.resize(100,100);
    //设置窗口标题
    w.setWindowTitle("hello world");
    //设置窗口固定大小
    w.setFixedSize(800,600);
    //同时设置窗口的位置和大小
    w.setGeometry(400,400,800,600);
    //展示窗口
    w.show();
    return a.exec();
}
