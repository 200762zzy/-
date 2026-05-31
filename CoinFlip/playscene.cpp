#include "playscene.h"
#include"mypushbutton.h"
#include<QPainter>
#include<QPixmap>
#include<QMenuBar>
#include<QLabel>
#include"coinbutton.h"
#include"dataconfig.h"
#include<QTimer>
#include<QMessageBox>
#include<QLabel>
#include<QPropertyAnimation>
#include<QSound>
PlayScene::PlayScene(int level,QWidget *parent) : MyMainWindow(parent)
{
    MyPushButton* btnBack=new MyPushButton(":/res/BackButton.png",":/res/BackButtonSelected.png",this);
    btnBack->resize(72,32);
    connect(btnBack,&QPushButton::clicked,this,&PlayScene::backBtnclicked);

    btnBack->move(this->width()-btnBack->width(),this->height()-btnBack->height());

    QLabel* label=new QLabel(this);
    label->setText(QString("level: %1").arg(level));
    label->resize(150,50);
    label->setFont(QFont("华文新魏",20));
    label->move(30,this->height()-label->height());


    //16个按钮
    const int colWidth=50;
    const int rowHeight=50;
    const int xOffset=57;
    const int yOffset=200;


    dataConfig data;
    QVector<QVector<int>> dataMap=data.mData[level];
    for(int row=0;row<4;++row){
        for(int col=0;col<4;col++){
            CoinButton* btn=new CoinButton(this);
            mCoins[row][col]=btn;
            int x=col* colWidth+xOffset;
            int y=row* rowHeight+yOffset;
            btn->setGeometry(x,y,50,50);
            btn->setState(dataMap[row][col]);
            connect(btn,&CoinButton::clicked,[=](){
                //btn->flip();
                this->flip(row,col);
            });
        }
    }
}

void PlayScene::flip(int row, int col)
{   if(mHashWin){
        return;}

    this->mCoins[row][col]->flip();
    QSound::play(":/res/ConFlipSound.wav");
    QTimer::singleShot(240,[=](){
        if(row-1>=0)
            this->mCoins[row-1][col]->flip();
        if(row+1<4)
            this->mCoins[row+1][col]->flip();
        if(col-1>=0)
            this->mCoins[row][col-1]->flip();
        if(col+1<4)
            this->mCoins[row][col+1]->flip();
        this->judgeWin();
    });

}

void PlayScene::judgeWin()
{
    for(int row=0;row<4;++row){
        for(int col=0;col<4;col++){
           if(!this->mCoins[row][col]->state()){
               return;
           }
        }
    }
    mHashWin=true;
    QSound::play(":/res/LevelWinSound.wav");
    //QMessageBox::information(this,"恭喜","你已经胜利了");
    QLabel* labelWin=new QLabel(this);
    QPixmap pix=QPixmap(":/res/LevelCompletedDialogBg.png");

    labelWin->setPixmap(pix);
    labelWin->resize(pix.size());
    labelWin->show();
    labelWin->move((this->width()-labelWin->width())/2,-labelWin->height());
    QPropertyAnimation* animation=new QPropertyAnimation(labelWin,"geometry",this);
    animation->setStartValue(labelWin->geometry());
    animation->setEndValue(QRect(labelWin->x(),labelWin->y()+100,labelWin->width(),labelWin->height()));
    animation->setDuration(1000);
    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PlayScene::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.translate(0,this->menuBar()->height());
    QPixmap pix(":/res/PlayLevelSceneBg.png");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);
    pix.load(":/res/Title.png");
    pix=pix.scaled(pix.width()/2,pix.height()/2);
    painter.drawPixmap(0,0,pix);
}
