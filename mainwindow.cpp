#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "exeitemwidget.h"
#include <QProcess>
#include <QDir>
#include <aboutdialog.h>
#include <QFileIconProvider>
#include <QMessageBox>
#include "historydialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 饼图
    _chartLeft = new QChart();
    _chartLeft->legend()->hide();
    ui->chartViewLeft->setChart(_chartLeft);
    _chartRight = new QChart();
    _chartRight->legend()->hide();
    ui->chartViewRight->setChart(_chartRight);
    CheckUsing();
    connect(&_timerCheckUsing, &QTimer::timeout, this, &MainWindow::CheckUsing);
    _timerCheckUsing.setInterval(50 * 1000);
    _timerCheckUsing.start();
    // 状态栏
    _statusbarLabel = new QLabel("总时间：", this);
    ui->statusbar->addWidget(_statusbarLabel);                     // 在状态栏左面
    ui->statusbar->addPermanentWidget(new QLabel("V1.0.0", this)); // 在状态栏右面
    // 托盘
    _appTrayIcon =  new AppTrayIcon(this);
    // 读取设置
    _settings = new AppSettings("config.ini");
    _settings->CheckAndCreateDirectories();
    // 数据库
    _dbManager = DatabaseManager::instance(_settings->DataBasePath);
    _dbManager->OpenDatabase();
    _tableName += _today.toString("yyyyMMdd");
    _dbManager->CreateTable(_tableName);
    // 每隔GetTopWindowInterval_s秒获取顶层窗口
    // GetTopWindow();
    connect(&_timerGetTopWindow, &QTimer::timeout, this, &MainWindow::GetTopWindow);
    _timerGetTopWindow.setInterval(_settings->GetTopWindowInterval_s * 1000);
    _timerGetTopWindow.setTimerType(Qt::PreciseTimer);
    _timerGetTopWindow.start();
    // 每个RefreshListWidgetInterval_s分钟刷新列表
    connect(&_timerRefreshListWidget, &QTimer::timeout, this, &MainWindow::RefreshListWidget);
    _timerRefreshListWidget.setInterval(_settings->RefreshListWidgetInterval_s * 1000);
    // 每ScreenshotInterval_min分钟截图
    if (_settings->EnableScreenshot)
    {
        connect(&_timerCaptureFullScreen, &QTimer::timeout, this, &MainWindow::CaptureFullScreen);
        _timerCaptureFullScreen.start(_settings->ScreenshotInterval_min * 60 * 1000);
    }
    // 是否在运行程序时显示窗口
    if (_settings->ShowWindowOnStart)
    {
        show();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CheckUsing()
{
    if (QTime::currentTime().hour() >= 12)
    {
        _second12hours[(QTime::currentTime().hour() - 12) * 60 + QTime::currentTime().minute()] = true;
    }
    else
    {
        _first12hours[QTime::currentTime().hour() * 60 + QTime::currentTime().minute()] = true;
    }
}

void MainWindow::AddListWidgetItem(QString name, QString iconPath, QString time, int percentage)
{
    ExeItemWidget *widget = new ExeItemWidget(ui->listWidget, name, iconPath, time, percentage);
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(widget->sizeHint());
    ui->listWidget->setItemWidget(item, widget);
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
    if (_today < QDateTime::currentDateTime().date()) // 已经第二天
    {
        ShellExecuteW(NULL, L"open", reinterpret_cast<const wchar_t *>(QCoreApplication::applicationFilePath().replace("/", "\\").utf16()), NULL, NULL, SW_SHOW);
        qInfo() << "第二天";
        QApplication::quit();
    }
    HWND activeWindowHandle = GetForegroundWindow();
    DWORD processId = 0;
    GetWindowThreadProcessId(activeWindowHandle, &processId);
    if (processId == 0)
    {
        qWarning() << QString("无法获取processId（%1）, HWND: ").arg(GetLastError()) << activeWindowHandle;
        _exeUsageList["Unknown"] += _settings->GetTopWindowInterval_s;
        return;
    }
    // 打开进程
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess)
    {
        qWarning() << QString("无法获取打开进程（%1）HWND, pid: ").arg(GetLastError()) << activeWindowHandle << ", " << processId;
        _exeUsageList["Unknown"] += _settings->GetTopWindowInterval_s;
        return;
    }
    // 获取路径
    WCHAR processPath[MAX_PATH];
    if (GetModuleFileNameExW(hProcess, NULL, processPath, MAX_PATH) == 0)
    {
        CloseHandle(hProcess);
        qWarning() << QString("无法获取应用名（%1）。HWND, pid: ").arg(GetLastError()) << activeWindowHandle << ", " << processId;
        _exeUsageList["Unknown"] += _settings->GetTopWindowInterval_s;
        return;
    }
    CloseHandle(hProcess);
    QString exePath = QString::fromWCharArray(processPath);
    QFileInfo fileInfo(exePath);
    QString exeName = fileInfo.completeBaseName();
    // 改程序的使用时间 + 间隔
    _exeUsageList[exeName] += _settings->GetTopWindowInterval_s;
    // 图标
    QString iconPath = QString("%1\\%2.png").arg(_settings->ExeIconDir, exeName);
    QFile file(iconPath);
    if (!file.exists()) // 如果不存在图片就创建
    {
        QFileIconProvider iconProvider;
        QIcon icon = iconProvider.icon(fileInfo);
        if (!icon.isNull())
        {
            QPixmap pixmap = icon.pixmap(40, 40);
            pixmap.save(iconPath);
        }
    }
    _getTopWindowCount++;
    if (_getTopWindowCount == _settings->SaveTriggerCount_times)
    {
        SaveDataToDB();
        _getTopWindowCount = 0;
    }
}

void MainWindow::RefreshListWidget()
{
    // 求总时间和
    int totalSeconds = _dbManager->GetTotalSeconds(_tableName);
    _statusbarLabel->setText(QString("总时间：%1").arg(FormatSeconds(totalSeconds)));
    if (totalSeconds == 0)
        return;
    // 获取每个应用项信息
    ui->listWidget->clear();
    QSqlQuery query = _dbManager->GetAppUsageData(_tableName);
    while (query.next())
    {
        QString name = query.value(0).toString();
        int seconds = query.value(1).toInt();
        QString iconPath = QString("%1\\%2.png").arg(_settings->ExeIconDir, name);
        AddListWidgetItem(name, iconPath, FormatSeconds(seconds), seconds * 100 / totalSeconds);
    }
    // 左面的饼图
    int slow = 0, fast = 1, index = 0;
    QPieSeries *leftSeries = new QPieSeries();
    leftSeries->setPieSize(1);
    while (fast < 12 * 60)
    {
        if (_first12hours[fast] != _first12hours[slow])
        {
            if (_first12hours[slow] == false) // 未使用
            {
                leftSeries->append("use", fast - slow);
                QPieSlice *slice = leftSeries->slices().at(index++);
                slice->setExplodeDistanceFactor(0);
                slice->setColor(QColor(200, 200, 200));
            }
            else
            {
                leftSeries->append("not use", fast - slow);
                QPieSlice *slice = leftSeries->slices().at(index++);
                slice->setExplodeDistanceFactor(0);
                slice->setColor(QColor(26, 148, 203));
            }
            slow = fast;
        }
        fast++;
    }
    if (_first12hours[slow] == false) // 未使用
    {
        leftSeries->append("use", fast - slow);
        QPieSlice *slice = leftSeries->slices().at(index++);
        slice->setExplodeDistanceFactor(0);
        slice->setColor(QColor(200, 200, 200));
    }
    else
    {
        leftSeries->append("not use", fast - slow);
        QPieSlice *slice = leftSeries->slices().at(index++);
        slice->setExplodeDistanceFactor(0);
        slice->setColor(QColor(26, 148, 203));
    }
    _chartLeft->addSeries(leftSeries);
    // 右面的饼图
    slow = 0;
    fast = 1;
    index = 0;
    QPieSeries *rightSeries = new QPieSeries();
    rightSeries->setPieSize(1);
    while (fast < 12 * 60)
    {
        if (_second12hours[fast] != _second12hours[slow])
        {
            if (_second12hours[slow] == false) // 未使用
            {
                rightSeries->append("use", fast - slow);
                QPieSlice *slice = rightSeries->slices().at(index++);
                slice->setColor(QColor(200, 200, 200));
            }
            else
            {
                rightSeries->append("not use", fast - slow);
                QPieSlice *slice = rightSeries->slices().at(index++);
                slice->setColor(QColor(26, 148, 203));
            }
            slow = fast;
        }
        fast++;
    }
    if (_second12hours[slow] == false) // 未使用
    {
        rightSeries->append("use", fast - slow);
        QPieSlice *slice = rightSeries->slices().at(index++);
        slice->setColor(QColor(200, 200, 200));
    }
    else
    {
        rightSeries->append("not use", fast - slow);
        QPieSlice *slice = rightSeries->slices().at(index++);
        slice->setColor(QColor(26, 148, 203));
    }
    _chartRight->addSeries(rightSeries);
}

QString MainWindow::FormatSeconds(int totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    QString result;
    if (hours > 0)
    {
        result += QString("%1小时").arg(hours);
    }
    if (minutes > 0 || hours > 0)
    { // 如果有小时，也显示分钟部分
        result += QString("%1分").arg(minutes);
    }
    result += QString("%1秒").arg(seconds);

    return result;
}

void MainWindow::on_actionOpenHistoryDialog_triggered()
{
    HistoryDialog *w = new HistoryDialog();
    w->show();
}

void MainWindow::CaptureFullScreen()
{
    // 获取当前活动窗口的屏幕
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
    {
        qWarning() << "无法获取屏幕";
        QMessageBox::about(this, "错误", "无法获取屏幕");
        return;
    }

    // 截取全屏
    QPixmap pixmap = screen->grabWindow(0);

    // 检查截图是否成功
    if (pixmap.isNull())
    {
        qWarning() << "截图失败";
        QMessageBox::about(this, "错误", "截图失败");
        return;
    }

    // 将截图保存为JPEG格式
    QString filePath = QString("%1\\%2.jpeg").arg(_settings->TodayScreenshotDir, QTime::currentTime().toString("hh-mm-ss"));
    if (!filePath.isEmpty())
    {
        // 将QPixmap转换为QImage以支持质量设置
        QImage image = pixmap.toImage();
        if (!image.save(filePath, "JPEG", _settings->JpegQuality))
        {
            qWarning() << "报错截图失败";
            QMessageBox::about(this, "错误", "保存截图失败");
        }
    }
}

void MainWindow::SaveDataToDB()
{
    for (auto it = _exeUsageList.begin(); it != _exeUsageList.end(); ++it)
    {
        _dbManager->InsertOrUpdateAppTime(_tableName, it.key(), it.value());
    }
    _exeUsageList.clear();
}
