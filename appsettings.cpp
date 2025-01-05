#include "appsettings.h"
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>

AppSettings::AppSettings(const QString& filePath)
    : _settings(ToAbsolutePath(filePath), QSettings::IniFormat)
{
    QString absoluteFilePath = ToAbsolutePath(filePath);
    if(QFile::exists(absoluteFilePath))
    {
        UserDataFolder = _settings.value("UserDataFolder", UserDataFolder).toString();
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
        _settings.setValue("UserDataFolder", UserDataFolder);
        _settings.setValue("EnableScreenshot", EnableScreenshot);
        _settings.setValue("ScreenshotInterval_min", ScreenshotInterval_min);
        _settings.setValue("JpegQuality", JpegQuality);
        _settings.setValue("GetTopInterval_s", GetTopWindowInterval_s);
        _settings.setValue("SaveTriggerCount_times", SaveTriggerCount_times);
        _settings.setValue("RefreshListWidgetInterval_s", RefreshListWidgetInterval_s);
        _settings.setValue("ShowWindowOnStart", ShowWindowOnStart);
        _settings.sync();
    }

    UserDataFolder = ToAbsolutePath(UserDataFolder);
    QDir userDir(UserDataFolder);
    ExeIconFolder = userDir.filePath("exe-icon");
    DataBasePath = userDir.filePath("sqlite3.db");
    ScreenshotDir = userDir.filePath("screenshot");
    UsageRecordsFolder = userDir.filePath("usage-records");
    UsageRecordsFile = UsageRecordsFolder + QDir::separator() + QDate::currentDate().toString("yyyy-MM-dd") + ".dat";
    TodayScreenshotFolder = userDir.filePath(QString("screenshot") + QDir::separator() + QDate::currentDate().toString("yyyy-MM-dd"));
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
    if (!dir.exists(UserDataFolder))
    {
        if (!dir.mkpath(UserDataFolder)) {
            qCritical() << "无法创建用户数据文件夹：" << UserDataFolder;
            // QMessageBox::critical(this, "错误", "无法创建用户数据文件夹");
        }
    }
    if (!dir.exists(ExeIconFolder))
    {
        if (!dir.mkpath(ExeIconFolder)) {
            qCritical() << "无法创建 exe-icon 文件夹：" << ExeIconFolder;
            // QMessageBox::critical(this, "错误", "无法创建图标存储文件夹");
        }
    }
    if (!dir.exists(ScreenshotDir))
    {
        if (!dir.mkpath(ScreenshotDir)) {
            qCritical() << "无法创建截图文件夹：" << ScreenshotDir;
            // QMessageBox::critical(this, "错误", "无法创建截屏文件夹");
        }
    }
    if (!dir.exists(UsageRecordsFolder))
    {
        if (!dir.mkpath(UsageRecordsFolder)) {
            qCritical() << "无法创建使用记录文件夹：" << ScreenshotDir;
            // QMessageBox::critical(this, "错误", "无法创建截屏文件夹");
        }
    }
    if (!dir.exists(TodayScreenshotFolder))
    {
        if (!dir.mkpath(TodayScreenshotFolder)) {
            qCritical() << "无法创建今天的截屏文件夹：" << TodayScreenshotFolder;
            // QMessageBox::critical(this, "错误", "无法创建今天的截屏文件夹：");
        }
    }
}
