#include "startscene.h"
#include<QPushButton>
#include"mypushbutton.h"
#include<QTimer>
#include<QSound>

StartScene::StartScene(QWidget *parent) : MyMainWindow(parent)
{
    this->setWindowTitle("开始窗口");
    MyPushButton* btnStart= new MyPushButton(":/res/MenuSceneStartButton.png"
                                             ,":/res/MenuSceneStartButton.png"
                                             ,this);
    btnStart->resize(114,114);
    //按钮水平居中垂方向在四分之三位置

    btnStart->move((this->width()-btnStart->width())/2,
                   this->height()*3/4-btnStart->height()/2);
    connect(&this->mSelectScene,&SelectScene::backBtnclicked,[=](){
        QSound::play(":/res/BackButtonSound.wav");
        this->show();
        this->mSelectScene.hide();
        this->move(this->mSelectScene.pos());
    });

    connect(btnStart,&MyPushButton::clicked,[=](){
        QSound::play(":/res/TapButtonSound.wav");
        btnStart->setEnabled(false);
        btnStart->moveDown();
        QTimer::singleShot(150,[=](){
            btnStart->moveUp();
        });
        QTimer::singleShot(300,[=](){
            btnStart->setEnabled(true);
            //场景转换
            //隐藏当前窗口
            this->hide();
            //显示第二个窗口
            this->mSelectScene.show();
            this->mSelectScene.move(this->pos());
        });
    });
}
