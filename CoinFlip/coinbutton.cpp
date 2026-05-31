#include "coinbutton.h"
#include<QPainter>
#include<QTimer>
CoinButton::CoinButton(QWidget *parent) : QPushButton(parent)
{
    this->setState(0);
    //去边框
    this->setStyleSheet("QPushButton{border::0px;}");

    connect(&this->mTimer,&QTimer::timeout,[=](){
        if(this->mState){
             this->mFrame--;
        }
        else{
             this->mFrame++;
        }

        QString frameName=QString(":/res/Coin000%1.png").arg(this->mFrame);
        this->setIcon(QIcon(frameName));
        if(this->mFrame==8||this->mFrame==1){
            this->mTimer.stop();
        }
    });
}

int CoinButton::state() const
{
    return mState;
}

void CoinButton::setState(int state)
{
    mState = state;
    if(this->mState){
        this->setIcon(QIcon(":/res/Coin0001.png"));
    }
    else{
        this->setIcon(QIcon(":/res/Coin0008.png"));
    }
    this->setIconSize(this->size());
}

void CoinButton::flip()
{
    this->setStateWithAnimation(!this->state());
}

void CoinButton::setStateWithAnimation(int state)
{
    //g->s
    this->mState=state;
    if(this->mState){
        this->mFrame=8;

    }
    else{
        this->mFrame=1;

    }
    this->mTimer.start(30);
}

void CoinButton::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    QPixmap pix;
    pix.load(":/res/BoardNode.png");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);
    QPushButton::paintEvent(ev);
}

