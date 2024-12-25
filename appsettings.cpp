#include "appsettings.h"
#include <QFile>
#include <QDir>
#include <QCoreApplication>

AppSettings::AppSettings(const QString& filePath)
    : _settings(ToAbsolutePath(filePath), QSettings::IniFormat)
{
    QString absoluteFilePath = ToAbsolutePath(filePath);
    if(QFile::exists(absoluteFilePath))
    {
        UserDataDir = _settings.value("UserDataDir", UserDataDir).toString();
        EnableScreenshot = _settings.value("EnableScreenshot", EnableScreenshot).toBool();
        ScreenshotInterval_min = _settings.value("ScreenshotInterval_min", ScreenshotInterval_min).toInt();
        JpegQuality = _settings.value("JpegQuality", JpegQuality).toInt();
        if (JpegQuality < 0 || JpegQuality > 100) // 确保JPEG质量在0-100范围内
        {
            JpegQuality = 9; // 设置为默认值
        }
        GetTopWindowInterval_s = _settings.value("GetTopInterval_s", GetTopWindowInterval_s).toInt();
        SaveTriggerCount_times = _settings.value("SaveTriggerCount_times", SaveTriggerCount_times).toInt();
        RefreshListWidgetInterval_s = _settings.value("RefreshListWidgetInterval_s", RefreshListWidgetInterval_s).toInt();
        ShowWindowOnStart = _settings.value("ShowWindowOnStart", ShowWindowOnStart).toBool();
    }
    else
    {
        _settings.setValue("UserDataDir", UserDataDir);
        _settings.setValue("EnableScreenshot", EnableScreenshot);
        _settings.setValue("ScreenshotInterval_min", ScreenshotInterval_min);
        _settings.setValue("JpegQuality", JpegQuality);
        _settings.setValue("GetTopInterval_s", GetTopWindowInterval_s);
        _settings.setValue("SaveTriggerCount_times", SaveTriggerCount_times);
        _settings.setValue("RefreshListWidgetInterval_s", RefreshListWidgetInterval_s);
        _settings.setValue("ShowWindowOnStart", ShowWindowOnStart);
        _settings.sync();
    }

    UserDataDir = ToAbsolutePath(UserDataDir);
    QDir userDir(UserDataDir);
    ExeIconDir = userDir.filePath("exe-icon");
    DataBasePath = userDir.filePath("sqlite3.db");
    ScreenshotDir = userDir.filePath("screenshot");
    TodayScreenshotDir = userDir.filePath(QString("screenshot") + QDir::separator() + QDate::currentDate().toString("yyyy-MM-dd"));
}

QString AppSettings::ToAbsolutePath(const QString& path) const
{
    return QDir::isAbsolutePath(path) ? path : QDir(QCoreApplication::applicationDirPath()).filePath(path);
}

void AppSettings::sync()
{
    _settings.sync();
}


void AppSettings::CheckAndCreateDirectories()
{
    // 必要的文件夹
    QDir dir;
    // 检查并创建用户数据文件夹
    if (!dir.exists(UserDataDir))
    {
        if (!dir.mkpath(UserDataDir)) {
            qCritical() << "无法创建用户数据文件夹：" << UserDataDir;
            // QMessageBox::critical(this, "错误", "无法创建用户数据文件夹");
        }
    }
    // 检查并创建 exe-icon 文件夹
    if (!dir.exists(ExeIconDir))
    {
        if (!dir.mkpath(ExeIconDir)) {
            qCritical() << "无法创建 exe-icon 文件夹：" << ExeIconDir;
            // QMessageBox::critical(this, "错误", "无法创建图标存储文件夹");
        }
    }
    // 检查并创建 screenshot 文件夹
    if (!dir.exists(ScreenshotDir))
    {
        if (!dir.mkpath(ScreenshotDir)) {
            qCritical() << "无法创建截图文件夹：" << ScreenshotDir;
            // QMessageBox::critical(this, "错误", "无法创建截屏文件夹");
        }
    }
    // 检查并创建 今天的screenshot 文件夹
    if (!dir.exists(TodayScreenshotDir))
    {
        if (!dir.mkpath(TodayScreenshotDir)) {
            qCritical() << "无法创建今天的截屏文件夹：" << TodayScreenshotDir;
            // QMessageBox::critical(this, "错误", "无法创建今天的截屏文件夹：");
        }
    }
}
