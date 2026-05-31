#include "widget.h"
#include "ui_widget.h"
#include<QPainter>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    //在这里画画
    QPainter painter(this);
    //画家偏移，搬动到某个坐标开始画画
    painter.translate(100,100);
    //创建一个画笔
    QPen pen;
    pen.setColor(QColor(255,0,0));

    //设置画笔风格
    pen.setStyle(Qt::DashLine);
    //设置宽度
    pen.setWidth(3);
    //需要填充使用画刷
    QBrush brush;
    brush.setColor(Qt::cyan);
    //设置画刷风格
    brush.setStyle(Qt::Dense3Pattern);
    //画家设置画框
    painter.setBrush(brush);


    painter.setPen(pen);
    //画条线
    painter.drawLine(0,0,100,100);

    //画矩形 左上角坐标，宽 高
    painter.drawRect(0,0,100,100);

    //画圆形用椭圆
    painter.drawEllipse(QPoint(100,100),100,100);

    //画文字
    painter.drawText(200,100,"好好学习，天天向上");
}

