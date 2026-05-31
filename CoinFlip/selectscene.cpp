#include "selectscene.h"
#include<QPushButton>
#include<QPainter>
#include<QMenuBar>
#include"mypushbutton.h"
#include"playscene.h"
#include<QSound>
SelectScene::SelectScene(QWidget *parent) : MyMainWindow(parent)
{   this->setWindowTitle("选择关卡");
    MyPushButton* btnBack=new MyPushButton(":/res/BackButton.png",":/res/BackButtonSelected.png",this);
    btnBack->resize(72,32);
    connect(btnBack,&QPushButton::clicked,this,&SelectScene::backBtnclicked);

    btnBack->move(this->width()-btnBack->width(),this->height()-btnBack->height());
    //创建20个按钮
    const int colWidth=70;
    const int rowHeight=70;
    const int xOffset=25;
    const int yOffset=130;
    for(int i=0;i<20;i++){
        MyPushButton* btn=new MyPushButton(":/res/LevelIcon.png",
                                           ":/res/LevelIcon.png",
                                           this);
        btn->setText(QString::number(i+1));
        //排序
        //x=列数*列宽 y=行数*行高
        int row=i/4;
        int col=i%4;
        int x=col*colWidth;
        int y=row* rowHeight;
        btn->resize(57,57);
        btn->move(x+xOffset,y+yOffset);
        connect(btn,&MyPushButton::clicked,[=](){
            QSound::play(":/res/TapButtonSound.wav");

            //点击按钮打开新的场景
            PlayScene* ps=new PlayScene(i+1);
            ps->move(this->pos());
            ps->setAttribute(Qt::WA_DeleteOnClose);
            ps->show();
            this->hide();
            connect(ps,&PlayScene::backBtnclicked,[=](){
                QSound::play(":/res/BackButtonSound.wav");
                this->move(ps->pos());
                this->show();
                ps->close();
            });
        });

    }
}

void SelectScene::paintEvent(QPaintEvent *event)
{
    //绘制背景图片
    QPainter painter(this);
    painter.translate(0,this->menuBar()->height());
    QPixmap pix(":/res/OtherSceneBg.png");
    painter.drawPixmap(0,0,this->width(),this->height(),pix);
    pix.load(":/res/Title.png");
    painter.drawPixmap(0,0,pix);
}
