#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
protected:
    void timerEvent(QTimerEvent* event);

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_start_clicked();

    void on_stop_clicked();


    void on_reset_clicked();

    void on_start_2_clicked();

    void on_stop_2_clicked();

private:
    Ui::Widget *ui;

    int timerId;
};
#endif // WIDGET_H
