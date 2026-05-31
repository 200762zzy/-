#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include"student.h"
#include"teacher.h"
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    //触发函数
    void classIsOver();

private:
    Teacher* pTeacher;
    student* pStudent;
};
#endif // WIDGET_H
