#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QWidget>
#include<QPushButton>
class MyPushButton : public QPushButton
{
    Q_OBJECT
public:

    enum MyPushButtonstat{
        Normal,
        Presed
    };
    explicit MyPushButton(QString nomalImg,QString pressedImg,QWidget *parent = nullptr);

    //往上
    void moveUp();
    //往下
    void moveDown();
protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent* ev);
private:
    //正常状态的图片
    QString mNormalImg;
    QString mPressImg;
    MyPushButtonstat mStat;
signals:

};

#endif // MYPUSHBUTTON_H
