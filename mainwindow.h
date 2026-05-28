#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 条件结构体 — 存储单条 WHERE 条件
struct WhereCondition {
    QString leftTable;      // 左侧表名（无表前缀时为空）
    QString leftField;      // 左侧字段名
    QString op;             // 操作符（=、>、<、>=、<=、<>）
    QString rightTable;     // 右侧表名（字段比较时有效）
    QString rightField;     // 右侧字段名（字段比较时有效）
    QString rightValue;     // 右侧值（值比较时有效）
    bool    isFieldCompare; // false: 字段与值比较; true: 字段与字段比较
};

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

    // 当前查询状态
    QStringList m_queryFields;                 // SELECT 字段列表
    QStringList m_queryTables;                 // FROM 表名列表
    QList<WhereCondition> m_queryConditions;   // WHERE 条件列表
    bool        m_isMultiTable;                // 是否多表查询
    QStringList m_joinHeader;                  // 多表时完整带前缀表头
    QList<int>  m_fieldIndexes;                // 单表时字段到 CSV 列索引映射

    // 数据缓存
    QList<QStringList> m_allData;              // 全部数据行
    QStringList m_csvHeader;                   // CSV 完整表头（用于写回）
    int m_currentPage;
    int m_pageSize;

    // SQL 解析与验证（解析结果写入上述成员变量）
    bool validateAndParseSql(const QString &sql, QString &errorMsg);

    // 通用 CSV 加载工具
    bool loadCsvFile(const QString &tableName, QStringList &header,
                     QList<QStringList> &data);

    // 查询执行
    bool executeSingleTable();
    bool executeMultiTable();

    // 值比较与条件匹配
    bool compareValues(const QString &l, const QString &op, const QString &r);
    bool checkConditions(const QStringList &row, const QStringList &header);

    // CRUD 辅助
    bool saveCsv(const QString &tableName);
    void syncTableToData();
};

#endif // MAINWINDOW_H
