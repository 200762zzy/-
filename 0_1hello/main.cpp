#include "widget.h"
#include<iostream>
#include <QApplication>

int main(int argc, char *argv[])
{   //创建一个应用程序对象
    //维护qt应用程序的一个对象,每个qt有且只有一个的对象

    QApplication a(argc, argv);
    //窗口类的一个对象
    Widget w;
    //把窗口显示出来
    w.show();

    std::cout<<"before exec"<<std::endl;
    a.exec();
    //死循环让程序一直运行，生命循环，消息循环
    //    while(1){
    //        if(点击x按钮)//生命循环
    //            break;
    //        if(点击最小化按钮){//消息循环
    //            最小化动作;
    //            ...;(其他操作);
    //        }
    //    }


    std::cout<<"after exec"<<std::endl;
    return 0;
    //return a.exec();
}
