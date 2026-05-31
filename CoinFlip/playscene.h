#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>
#include"mymainwindow.h"
#include"coinbutton.h"
class PlayScene : public MyMainWindow
{
    Q_OBJECT
public:
    explicit PlayScene(int level,QWidget *parent = nullptr);
    void flip(int row,int col);
    void judgeWin();
signals:
    void backBtnclicked();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    CoinButton* mCoins[4][4];
    bool mHashWin=false;
};

#endif // PLAYSCENE_H
