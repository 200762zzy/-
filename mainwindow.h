#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_btn_pre_clicked();
    void on_btn_next_clicked();
    void showPageData();
    void on_btn_add_clicked();
    void on_btn_delete_clicked();
    void on_btn_save_clicked();

private:
    Ui::MainWindow *ui;
    QList<QStringList> m_allData;
    QStringList m_csvHeader;              // CSV 文件表头，用于写回
    int m_currentPage;
    int m_pageSize;

    QStringList parseSql(QString sql, QString &tableName);
    bool loadCsvToTable(QString tableName, QStringList fields);
    bool saveCsv(QString tableName);      // 将 m_allData 写回 CSV
    void syncTableToData();               // 将表格编辑同步到 m_allData
};

#endif // MAINWINDOW_H
