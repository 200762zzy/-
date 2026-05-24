#include "mylabel.h"
#include<QMouseEvent>
MyLabel::MyLabel(QWidget *parent) :QLabel(parent)
{
    this->setMouseTracking(true);
    //事件过滤器的使用
    //1.窗口调用installEventFilter来安装一个事件过滤器
    //2.参数是一个事件过滤器的对象Qobject，该对像的类要重写eventFilter的函数
    //事件过率的时候，事件会先达到事件过滤器的evenFilter函数
    //返回值：true表示拦截
    this->installEventFilter(this);
}

void MyLabel::mousePressEvent(QMouseEvent *ev)
{
    //输出鼠标事件一些信息
    //获取坐标
    int x= ev->x();
    int y=ev->y();
    //获取按键
    Qt::MouseButton btn=ev->button();
    QString strButton="";
    if(btn==Qt::LeftButton) strButton="leftbutton";
    if(btn==Qt::RightButton) strButton="Rightbutton";
    if(btn==Qt::MidButton) strButton="Midbutton";

    //label 可以显示html
    QString str=QString("<h1><center>press[%1,%2,%3]</center></h1>").arg(x).arg(y).arg(strButton);

    this->setText(str);
}

void MyLabel::mouseMoveEvent(QMouseEvent *ev)
{
    //输出鼠标事件一些信息
    //获取坐标
    int x= ev->x();
    int y=ev->y();
    //获取按键
    Qt::MouseButtons btns=ev->buttons();
    QString strButton="";
    if(btns & Qt::LeftButton) strButton+="leftbutton ";
    if(btns & Qt::RightButton) strButton+="Rightbutton ";
    if(btns & Qt::MidButton) strButton+="Midbutton ";

    //label 可以显示html
    QString str=QString("<h1><center>move[%1,%2,%3]</center></h1>").arg(x).arg(y).arg(strButton);

    this->setText(str);
}

bool MyLabel::event(QEvent *e)
{
    //返回值：true该时间得到处理，如果是flase，没被处理，事件会被继续传递到父窗口
    //Qevent就是所有Event的父类
    //判断类型
    if(e->type()==QEvent::MouseMove){
        this->mouseMoveEvent(static_cast<QMouseEvent*>(e));
        return true;
    }
    return QLabel::event(e);
}

bool MyLabel::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::MouseMove){
        //拦截
        return true;
    }
    return false;
}

