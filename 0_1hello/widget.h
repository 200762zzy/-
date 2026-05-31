#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{//宏：引入qt信号和槽
    Q_OBJECT

public:
    //parent窗口指针，父窗口对象指针
    //如果parent为0或者为空 表示当前窗口是个顶层窗口
    //顶层窗口就是可以在任务栏可以找到的
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
