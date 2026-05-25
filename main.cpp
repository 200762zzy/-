// =============================================================================
// main.cpp — 程序入口
// =============================================================================
// 职责：
//   1. 创建 QApplication（管理整个 GUI 程序的生命周期）
//   2. 设置全局字体（中文优化）
//   3. 创建并显示主窗口
//   4. 进入事件循环（等待用户操作）
// =============================================================================

#include "mainwindow.h"    // 主窗口类的头文件

#include <QApplication>    // Qt 应用程序类
#include <QFont>           // 字体设置

int main(int argc, char *argv[])
{
    // 第一步：创建 QApplication 对象
    // argc/argv 来自命令行参数。每个 Qt GUI 程序有且只有一个 QApplication。
    QApplication a(argc, argv);

    // 第二步：全局字体设置
    // 默认字体对中文渲染不好看，手动切换为 Microsoft YaHei UI
    // 这个字体在 Windows 上对中文字符有更好的显示效果
    QFont font = a.font();                      // 先拿到系统默认字体
    font.setFamily("Microsoft YaHei UI");       // 替换字体族
    font.setPointSize(9);                       // 设置字号（9pt）
    font.setStyleStrategy(QFont::PreferAntialias); // 开启抗锯齿，文字更平滑
    a.setFont(font);                            // 应用到整个应用程序

    // 第三步：创建主窗口
    // MainWindow 继承自 QMainWindow，会在构造时自动加载 mainwindow.ui
    MainWindow w;
    w.show();        // 显示窗口（默认是隐藏的，必须调用 show()）

    // 第四步：进入事件循环
    // a.exec() 阻塞运行，等待用户操作（鼠标点击、键盘输入等）
    // 用户关闭窗口后 exec() 返回，程序结束
    return a.exec();
}
