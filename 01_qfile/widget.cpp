#include "widget.h"
#include "ui_widget.h"
#include<QFile>
#include<QFileDialog>
#include<QTextCodec>
#include<QFileInfo>
#include<QDebug>
#include<QDateTime>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QFile file("D:\\比特就业课\\课件\\第3阶段-C++核心编程与桌面应用开发\\Qt\\day03\\03_resources\\text.txt");
    file.open(QIODevice::WriteOnly|QIODevice::Append);
    file.write("hello world");
    file.close();

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString fileName=QFileDialog::getOpenFileName(this,"打开一个txt",
                                                  "D:\\比特就业课\\课件\\第3阶段-C++核心编程与桌面应用开发\\Qt\\day03\\03_resources","TXT (*.txt)");
    //对filename做判断，如果没有选择文件，那么就是一个空串
    if(fileName.isEmpty()){
        return;
    }
    //不为空，选择某个文件名将其显示到lineedit上
    this->ui->lineEdit->setText(fileName);

    //使用qfile读取文件
    QFile file(fileName);
    //打开文件
    file.open(QIODevice::ReadOnly);
    //读取文件内容
    //将所有内容读取出来
    //QByteArray array=file.readAll();

    //单行读取
    QByteArray array;
    do{

        array=file.readLine();
    }while(!file.atEnd());
    //使用其他编码
    QTextCodec *codec=QTextCodec::codecForName("gbk");
    QString content=codec->toUnicode(array);

//  QString content=QString(array);
    this->ui->plainTextEdit->setPlainText(content);
    //关闭文件

    //获取文件信息
    QFileInfo info(fileName);
    qDebug()<<info.fileName();
    qDebug()<<info.baseName();
    qDebug()<<info.created().toString("");
    qDebug()<<info.lastModified();

    file.close();
}
