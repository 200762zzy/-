#include "mypushbutton.h"
#include<QPainter>
#include<QPropertyAnimation>
MyPushButton::MyPushButton(QString nomalImg,QString pressedImg,QWidget *parent) : QPushButton(parent)
  ,mNormalImg(nomalImg)
  ,mPressImg(pressedImg)
{
    mStat=Normal;
}

void MyPushButton::moveUp()
{
    QPropertyAnimation* animation=new QPropertyAnimation(this,"geometry",this);
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(this->x(),this->y()-10,this->width(),this->height()));
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

}

void MyPushButton::moveDown()
{
    QPropertyAnimation* animation=new QPropertyAnimation(this,"geometry",this);
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(this->x(),this->y()+10,this->width(),this->height()));
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MyPushButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap pix;
    if(mStat==Normal)
        pix.load(mNormalImg);
    if(mStat==Presed)
        pix.load(mPressImg);
    painter.drawPixmap(0,0,this->width(),this->height(),pix);
    painter.drawText(0,0,this->width(),this->height(),
                   Qt::AlignHCenter|Qt::AlignVCenter,this->text());
}

void MyPushButton::mousePressEvent(QMouseEvent *ev)
{
    this->mStat=Presed;
    update();
    QPushButton::mousePressEvent(ev);
}

void MyPushButton::mouseReleaseEvent(QMouseEvent *ev)
{
    this->mStat=Normal;
    update();
    QPushButton::mouseReleaseEvent(ev);
}


