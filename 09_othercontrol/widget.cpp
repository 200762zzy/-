#include "widget.h"
#include "ui_widget.h"
#include<QLabel>
#include<QMovie>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //设置label播放gif
    QMovie* movie=new QMovie("");
    ui->label->setMovie
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_8_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Widget::on_pushButton_9_clicked()
{
   ui->stackedWidget->setCurrentIndex(1);
}
