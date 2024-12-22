#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "exeitemwidget.h"
#include <QProcess>
#include <QDir>
#include <aboutdialog.h>
#include "apptrayicon.h"
#include <QFileIconProvider>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 托盘
    new AppTrayIcon(this);
    // 必要的文件夹
    QDir dir(QString("%1\\%2").arg(_appDir, _exeIconDir));
    if (!dir.exists())
    {
        bool success = dir.mkpath(".");
        if (!success)
        {
            QMessageBox::critical(this, "错误", QString("文件夹'%1'创建失败").arg(_exeIconDir));
            return;
        }
    }
    // 数据库
    _tableName += _today.toString("yyyyMMdd");
    _db.setDatabaseName(QString("%1\\%2").arg(_appDir, _dataBasePath));
    if (!_db.open())
    {
        QMessageBox::critical(this, "错误", "无法打开数据库：sqlite3.db\n请重启程序试试");
        return;
    }
    QString createTableQuery = QString("CREATE TABLE IF NOT EXISTS %1 ("
                                       "name TEXT PRIMARY KEY, "
                                       "seconds INTEGER)").arg(_tableName);
    _query = new QSqlQuery(_db);
    if (!_query->exec(createTableQuery))
    {
        QMessageBox::critical(this, "错误", QString("无法创建表，错误信息：%1\n请重启程序试试").arg(_query->lastError().text()));
        return;
    }
    // 初始内容
    AddListWidgetItem("每隔1分钟刷新一次列表", ":/img/Assets/unknownfile.png", "", 10);
    // 每隔一秒获取顶层窗口
    connect(&_timerGetTopWindow, &QTimer::timeout, this, &MainWindow::GetTopWindow);
    _timerGetTopWindow.setInterval(1000);
    _timerGetTopWindow.setTimerType(Qt::PreciseTimer);
    _timerGetTopWindow.start();
    // 每个1分钟刷新列表
    connect(&_timerRefreshListWidget, &QTimer::timeout, this, &MainWindow::RefreshListWidget);
    _timerRefreshListWidget.setInterval(60 * 1000);
    _timerRefreshListWidget.start();
}

MainWindow::~MainWindow()
{
    _db.close();
    delete _query;
    delete ui;
}

void MainWindow::AddListWidgetItem(QString name, QString iconPath, QString time, int percentage)
{
    ExeItemWidget *widget = new ExeItemWidget(ui->listWidget, name, iconPath, time, percentage);
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(widget->sizeHint());
    ui->listWidget->setItemWidget(item, widget);
}

void MainWindow::on_actionOpenWorkingDir_triggered()
{
    QString workingDir = QDir::currentPath();
    workingDir.replace("/", "\\");
    QProcess::startDetached("explorer", QStringList() << workingDir);
}


void MainWindow::on_actionOpenAppDir_triggered()
{
    QString exeDir = _appDir;
    exeDir.replace("/", "\\");
    QProcess::startDetached("explorer", QStringList() << exeDir);
}


void MainWindow::on_actionOpenAboutDialog_triggered()
{
    AboutDialog *w = new AboutDialog();
    w->show();
}

void MainWindow::GetTopWindow()
{
    if (_today < QDateTime::currentDateTime().date())//已经第二天
    {
        ShellExecuteW(NULL, L"open", reinterpret_cast<const wchar_t *>(QCoreApplication::applicationFilePath().replace("/", "\\").utf16()), NULL, NULL, SW_SHOW);
        QApplication::quit();
    }
    HWND activeWindowHandle = GetForegroundWindow();
    DWORD processId  = 0;
    GetWindowThreadProcessId(activeWindowHandle, &processId );
    if (processId  == 0)
    {return;}
    // 打开进程
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess)
        return;
    // 获取路径
    WCHAR processPath[MAX_PATH];
    if (GetModuleFileNameExW(hProcess, NULL, processPath, MAX_PATH) == 0)
    {
        CloseHandle(hProcess);
        return;
    }
    CloseHandle(hProcess);
    QString exePath = QString::fromWCharArray(processPath);
    QFileInfo fileInfo(exePath);
    QString exeName = fileInfo.completeBaseName();
    // 查找是否记录过
    QString updateQuery = QString("INSERT INTO %1 (name, seconds) VALUES ('%2', 1) ON CONFLICT(name) DO UPDATE SET seconds = seconds + 1;").arg(_tableName, exeName);
    if (!_query->exec(updateQuery))
    {
        QMessageBox::critical(this, "错误", QString("无法增加使用时间，错误信息：%1").arg(_query->lastError().text()));
    }
    QString iconPath = QString("%1\\%2\\%3.png").arg(_appDir, _exeIconDir, exeName);
    QFile file(iconPath);
    if (!file.exists()) //如果不存在图片就创建
    {
        QFileIconProvider iconProvider;
        QIcon icon = iconProvider.icon(fileInfo);
        if (!icon.isNull())
        {
            QPixmap pixmap = icon.pixmap(40, 40);
            pixmap.save(iconPath);
        }
    }
}

void MainWindow::RefreshListWidget()
{
    // 求总时间和
    int totalSeconds ;
    QString totalSecondsQuery = QString("SELECT SUM(seconds) FROM %1").arg(_tableName);
    if(!_query->exec(totalSecondsQuery))
    {
        QMessageBox::critical(this, "错误", QString("无法获取时间总和，错误信息：%1").arg(_query->lastError().text()));
        return;
    }
    if (_query->next())
    {
        totalSeconds = _query->value(0).toInt();
    }
    else
    {
        QMessageBox::critical(this, "错误", "没有找到时间总和");
        return;
    }
    // 获取每个应用项信息
    QString getQuery = QString("SELECT * FROM %1 ORDER BY seconds DESC").arg(_tableName);
    if (!_query->exec(getQuery))
    {
        QMessageBox::critical(this, "错误", QString("无法获取每个应用信息，错误信息：%1").arg(_query->lastError().text()));
        return;
    }
    ui->listWidget->clear();
    while(_query->next())
    {
        QString name = _query->value(0).toString();
        int seconds = _query->value(1).toInt();
        QString iconPath = QString("%1\\%2\\%3.png").arg(_appDir, _exeIconDir, name);
        AddListWidgetItem(name, iconPath, FormatSeconds(seconds), seconds * 100 / totalSeconds);
    }
}

QString MainWindow::FormatSeconds(int totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    QString result;
    if (hours > 0) {
        result += QString("%1小时").arg(hours);
    }
    if (minutes > 0 || hours > 0) { // 如果有小时，也显示分钟部分
        result += QString("%1分").arg(minutes);
    }
    result += QString("%1秒").arg(seconds);

    return result;
}
