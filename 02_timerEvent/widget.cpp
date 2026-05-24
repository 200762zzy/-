#include "widget.h"
#include "ui_widget.h"
#include<QDebug>
#include<QTimer>
static int num=1;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //创建定时器
    //startTimer(1000);

}
void Widget::timerEvent(QTimerEvent *event)
{

    //qDebug()<<num++;
    this->ui->lcdNumber->display(num++);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_start_clicked()
{   //一秒响一次
    this->timerId=startTimer(1000);

}

void Widget::on_stop_clicked()
{
    //停止计算器
    killTimer(this->timerId);
}

void Widget::on_reset_clicked()
{
    num=0;
    this->ui->lcdNumber->display(num);

}

void Widget::on_start_2_clicked()
{
    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,[=](){
        static int num1=1;
        this->ui->lcdNumber_2->display(num1++);
    });
    timer->start(10);
}

void Widget::on_stop_2_clicked()
{

}
