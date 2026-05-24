#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    w2=new NewWidget;
    btnToNext=new QPushButton("按钮1",this);
    w2->setWindowTitle("窗口2");
    connect(btnToNext,&QPushButton::clicked,[=](){
       w2->show();
       this->close();
    });
    QPushButton* btn2=new QPushButton("按钮2",w2);
    connect(btn2,&QPushButton::clicked,[=](){
        this->show();
        w2->close();
    });

}

Widget::~Widget()
{
}

