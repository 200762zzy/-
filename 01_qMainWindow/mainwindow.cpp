#include "mainwindow.h"
#include<QMenuBar>
#include<QToolBar>
#include<QStatusBar>
#include<QLabel>
#include<QDockWidget>
#include<QTextEdit>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(800,600);
    //菜单栏,获取当前窗口的菜单栏，没有就会自动创建一个
    QMenuBar *mb=this->menuBar();
    //添加菜单
    QMenu *menuFile = mb->addMenu("文件");
    QMenu *menuEdit = mb->addMenu("编辑");
    //在菜单里边添加菜单项
    QAction* actionNew= menuFile->addAction("新建");
    QAction* actionOpen= menuFile->addAction("打开");

    //添加分割符
    menuFile->addSeparator();
    //添加二级菜单
    QMenu* menuRencent = menuFile->addMenu("最近打开文件");
    menuRencent->addAction("1.txt");

    //工具栏，可以有多个
    QToolBar* toolbar = this->addToolBar("");
    //工具栏添加工具
    toolbar->addAction(actionNew);
    toolbar->addAction(actionOpen);
    //默认停靠在左边
    this->addToolBar(Qt::LeftToolBarArea,toolbar);
    //只允许停靠在左边或者右边
    toolbar->setAllowedAreas(Qt::LeftToolBarArea|Qt::RightToolBarArea);

    //设置工具栏不可以浮动
    toolbar->setFloatable(false);
    //设置工具栏不可移动
    toolbar->setMovable(false);

    //状态栏,只有一个就可以直接获取
    QStatusBar* sb=this->statusBar();
    //往状态栏内添加信息
    //添加左侧信息
    QLabel* labelLeft=new QLabel("左侧信息",this);
    sb->addWidget(labelLeft);
    //添加右侧信息
    QLabel* labelRight=new QLabel("右侧信息",this);
    sb->addPermanentWidget(labelRight);

    //停靠部件可以有多个
    QDockWidget* dw=new QDockWidget("停靠部件",this);
    //默认情况下没有核心部件作为参照物，停靠部件会占完窗口
    this->addDockWidget(Qt::BottomDockWidgetArea,dw);
    //添加核心部件
    QTextEdit* textEdit=new QTextEdit(this);
    this->setCentralWidget(textEdit);

}

MainWindow::~MainWindow()
{
}

