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
#include <QPixmap>
#include <QImage>
#include <QFileDialog>
#include "databasemanager.h"
#include "appsettings.h"
#include <QLabel>
#include "apptrayicon.h"
// #include <QChart>
// #include <QChartView>
#include <QtCharts>

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
    // 打开按照目录
    void on_actionOpenAppDir_triggered();
    // 打开关于窗口
    void on_actionOpenAboutDialog_triggered();

    void on_actionOpenHistoryDialog_triggered();

protected:
    // 窗口显示事件
    void showEvent(QShowEvent *event) override
    {
        SaveDataToDB();
        _timerRefreshListWidget.start();
        RefreshListWidget();
        QMainWindow::showEvent(event);
    }
    void hideEvent(QHideEvent *event) override
    {
        _timerRefreshListWidget.stop();
        QMainWindow::hideEvent(event);
    }
    // 窗口关闭事件
    void closeEvent(QCloseEvent* event) override
    {
        hide();
        event->ignore();
    }

private:
    Ui::MainWindow *ui;
    // 饼图
    QChart *_chartLeft;
    QChart *_chartRight;
    bool _first12hours[12 * 60] = {false};
    bool _second12hours[12 * 60] = {false};
    // 托盘
    AppTrayIcon *_appTrayIcon;
    // 每分钟检查是否程序在运行
    QTimer _timerCheckUsing;
    // 状态栏label
    QLabel *_statusbarLabel;
    // 设置
    AppSettings *_settings;
    // 临时的应用使用数据列表
    QMap<QString, int> _exeUsageList;
    // 程序exe所在的目录
    const QString _appDir = QCoreApplication::applicationDirPath();
    // 今天的日期
    const QDate _today = QDate::currentDate();
    // 数据库
    DatabaseManager *_dbManager;
    QString _tableName = "During_";
    // 获取顶层窗口的次数
    int _getTopWindowCount = 0;
    // 获取顶层窗口的计时器
    QTimer _timerGetTopWindow;
    // 刷新窗口
    QTimer _timerRefreshListWidget;
    // 截图
    QTimer _timerCaptureFullScreen;
    // 向列表中添加项
    void AddListWidgetItem(QString name, QString iconPath, QString time, int percentage);
    // 检测顶层窗口
    void GetTopWindow();
    // 刷新列表
    void RefreshListWidget();
    // 将总秒数转化为时间文本
    QString FormatSeconds(int totalSeconds);
    // 截图
    void CaptureFullScreen();
    // 保存数据到数据库
    void SaveDataToDB();
    // 检查是否在使用
    void CheckUsing();
};
#endif // MAINWINDOW_H
