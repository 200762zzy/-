#ifndef COINBUTTON_H
#define COINBUTTON_H

#include <QWidget>
#include<QPushButton>
#include<QTimer>
class CoinButton : public QPushButton
{
    Q_OBJECT
public:
    explicit CoinButton(QWidget *parent = nullptr);

    int state() const;
    void setState(int state);
    void flip();
    void setStateWithAnimation(int state);
signals:
protected:
    void paintEvent(QPaintEvent *ev) override;
private:
    //0表示银币
    int mState;
    int mFrame;
    QTimer mTimer;
};

#endif // COINBUTTON_H
