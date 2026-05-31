#include "widget.h"
#include<QString>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    //创建对象并挂入对象树
    pTeacher =new Teacher(this);
    pStudent =new student(this);
    //连接



    //hungry多一个参数
    //treat多一个参数
    //因为函数发生了重载所以取不到地址
    //1.使用函数重载赋值，让编译器选择符合类型的函数
    //2.使用static_cast强制转换，原理同上
    void(Teacher::*teacher_qString)(QString)=&Teacher::hungry;
    void(student::*student_qString)(QString)=&student::treat;
    connect(pTeacher,teacher_qString,pStudent,student_qString);

    connect(pTeacher,
            static_cast<void(Teacher::*)()>(&Teacher::hungry)
            ,pStudent,
            static_cast<void(student::*)()>(&student::treat)
            ) ;

    this->classIsOver();
}
void Widget::classIsOver(){
    //触发信号
    emit pTeacher->hungry();
    emit pTeacher->hungry("黄焖鸡");

}

Widget::~Widget()
{
}

