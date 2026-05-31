#ifndef TEACHER_H
#define TEACHER_H

#include <QObject>
#include<QString>
class Teacher : public QObject
{
    Q_OBJECT
public:
    explicit Teacher(QObject *parent = nullptr);

signals:
    //信号
    void hungry();
    void hungry(QString what);

};

#endif // TEACHER_H
