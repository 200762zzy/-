#include "widget.h"
#include<QDebug>
#include<QPushButton>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    //lambda表达式
    //调用lambda的方式 使用函数指针

//    void (*p)()= [](){
//        qDebug()<<"hello world";
//    };
    //捕获局部变量有两种方式，一种值传递，一种引用
    //默认情况下值传递进来的变量就是const
//    int a=10;
//    int b=20;
//    [a,&b]()mutable//可以修改局部变量
//    {
//      qDebug()<<a<<b;
//    };
    int a=19;
    int b=12;
    //信号和槽使用lambda
    QPushButton *btn=new QPushButton("按钮1",this);
    connect(btn,&QPushButton::clicked,[=](){
         qDebug()<<a<<b;
    });
}

Widget::~Widget()
{
}

