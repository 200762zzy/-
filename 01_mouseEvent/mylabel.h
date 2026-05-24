#ifndef MYLABEL_H
#define MYLABEL_H

#include <QWidget>
#include<QLabel>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = nullptr);

signals:

protected:
    //重写鼠标按键处理函数
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    bool event(QEvent *e) override;
    bool eventFilter(QObject* watched,QEvent* event)override;
};

#endif // MYLABEL_H
