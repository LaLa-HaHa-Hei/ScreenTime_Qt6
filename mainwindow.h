#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <windows.h>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <Psapi.h>
#include <QDate>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 打开工作目录
    void on_actionOpenWorkingDir_triggered();
    // 打开按照目录
    void on_actionOpenAppDir_triggered();
    // 打开关于窗口
    void on_actionOpenAboutDialog_triggered();

protected:
    // 窗口显示事件
    void showEvent(QShowEvent *event) override
    {
        QMainWindow::showEvent(event);
        RefreshListWidget();
    }
    // 窗口关闭事件
    void closeEvent(QCloseEvent* event) override
    {
        hide();
        event->ignore();
    }

private:
    Ui::MainWindow *ui;
    const QString _exeIconDir = ".\\user-data\\exe-icon";
    const QString _dataBasePath = ".\\user-data\\sqlite3.db";
    const QString _appDir = QCoreApplication::applicationDirPath();
    // 今天的日期
    const QDate _today = QDate::currentDate();
    // 数据库
    QString _tableName = "During_";
    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE");
    QSqlQuery *_query = nullptr;
    // 获取顶层窗口的计时器
    QTimer _timerGetTopWindow;
    // 刷新窗口
    QTimer _timerRefreshListWidget;
    // 向列表中添加项
    void AddListWidgetItem(QString name, QString iconPath, QString time, int percentage);
    // 检测顶层窗口
    void GetTopWindow();
    // 刷新列表
    void RefreshListWidget();
    // 将总秒数转化为时间文本
    QString FormatSeconds(int totalSeconds);
    // // 隐藏
    // void HideMainWindow();
    // // 显示
    // void ShowMainWindow();
    // // 退出
    // void ExitApp();
};
#endif // MAINWINDOW_H
