#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QPushButton>
#include"newwidget.h"
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);

    ~Widget();
private:
    QPushButton* btnToNext;
    NewWidget*  w2;

};
#endif // WIDGET_H
