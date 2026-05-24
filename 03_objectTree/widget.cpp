#include "widget.h"
#include<QPushButton>
#include"mypushbutton.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{   //局部变量在函数退出时就自行释放
//    QPushButton btn("按钮1",this);
//    btn.show();
    //解决 让按钮的生命周期长一点
    //1.static（不推荐）
    //2.类成员变量
    //3.new一个，动态分配对象
    Mypushbutton* btn1=new Mypushbutton(this);
    btn1->setText("按钮1");
    //没有delete

    //测试析构
    //再继承一个按钮类，在字类的析构里面打log
}

Widget::~Widget()
{
}

