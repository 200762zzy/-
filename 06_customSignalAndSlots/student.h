#ifndef STUDENT_H
#define STUDENT_H

#include <QObject>
#include<QString>
class student : public QObject
{
    Q_OBJECT
public:
    explicit student(QObject *parent = nullptr);
    //槽
    void treat();
    void treat(QString what);
signals:

};

#endif // STUDENT_H
