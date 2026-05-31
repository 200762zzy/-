#include "student.h"
#include<QDebug>
student::student(QObject *parent) : QObject(parent)
{

}

void student::treat()
{
    qDebug()<<"student treat teacher";
}

void student::treat(QString what)
{
      qDebug()<<"student treat teacher with"<<what;
}
