#include "mypushbutton.h"
#include<QtDebug>
Mypushbutton::Mypushbutton(QWidget *parent) : QPushButton(parent)
{

}

Mypushbutton::~Mypushbutton()
{
    qDebug()<<"mypushbutton destroyed";
}
