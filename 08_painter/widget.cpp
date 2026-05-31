#include "widget.h"
#include "ui_widget.h"
#include<QPainter>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    mPoX=0;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    //绘制已经存在的图片
     QPainter painter(this);
     QPixmap pixmap("://Image/butterfly1.png");
     painter.drawPixmap(mPoX,0,pixmap);
}


void Widget::on_pushButton_clicked()
{
    //每点击一次就向右移动图片
    mPoX+=10;
    //手动触发事件
    //两种方式
    //1.repaint
    //2.update
    //this->repaint();
    this->update();
}
