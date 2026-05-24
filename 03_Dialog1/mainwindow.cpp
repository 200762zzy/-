#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDialog>
#include<QtDebug>
#include<QMessageBox>
#include<QFileDialog>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionModal,&QAction::triggered,[=](){
        //创建一个模态对话框
        QDialog dlg(this);
        dlg.exec();

        qDebug()<<"hello dialog";
    });
    connect(ui->actionNonModal,&QAction::triggered,[=](){
        //创建一个非模态对话框
        //因为show是非堵塞行为出了作用域就会被销毁所以要用new
        QDialog* dlg=new QDialog(this);
        //释放问题，只有父对象释放的时候子对象才释放
        //通过设置窗口的属性
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();

    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actioncritical_triggered()
{
    QMessageBox::critical(this,"错误","critical");
}

void MainWindow::on_actionWarming_triggered()
{
    QMessageBox::warning(this,"警告","warning");
}

void MainWindow::on_actionInfo_triggered()
{
    QMessageBox::information(this,"信息","information");

}

void MainWindow::on_actionQuestion_triggered()
{   //点击了OK就打印
    if(QMessageBox::Ok==QMessageBox::question(this,"问你个事","你有对象吗",
                                              QMessageBox::Ok|QMessageBox::No))
    {
        qDebug()<<"点击了ok";
    }
    else{
        qDebug()<<"点击了cancle";
    }
}

void MainWindow::on_actionFileDialog_triggered()
{
    //打开一个文件对话框(this 窗口名字 路径，文件过滤器)
    QFileDialog::getOpenFileName(this,"打开一个文件","D:\\",
                                 "PNG*(.png);;JPG(*jpg);;GIF(*gif);;all(*.*)");
}
