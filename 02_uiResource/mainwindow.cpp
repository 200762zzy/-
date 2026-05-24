#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QLabel>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

   //往状态栏添加信息
    ui->statusbar->addWidget(new QLabel("左侧信息",this));
    //使用图骗资源
    //使用绝对路径
    //ui->actionnew->setIcon(QIcon("D:Qt\\day03\\03_resources\\Image\\Luffy.png"));
    //使用资源文件
    //使用资源路径的形式
    // 冒号+前缀+目录文件名
    ui->actionnew->setIcon(QIcon(":/new/prefix1/Image/Luffy.png"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

