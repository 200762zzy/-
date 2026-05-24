#include "widget.h"
#include<QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *btn=new QPushButton("关闭窗口",this);

    //关闭 this-》close
    //信号发出者
    //信号
    //信号接收者
    //槽：处理动作
    //通过connect(信号发出者,信号,信号接收者,槽)建立连接

    //保留&符号
    //提升代码可读性
    //自动提示
    connect(btn,&QPushButton::clicked,this,&Widget::close);
}

Widget::~Widget()
{
}

