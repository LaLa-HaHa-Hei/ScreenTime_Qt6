#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>

class AppSettings
{
public:
    explicit AppSettings(const QString& filePath = "config.ini");
    QString UserDataDir = ".\\user-data";
    QString ExeIconDir;
    QString DataBasePath;
    QString ScreenshotDir;
    QString TodayScreenshotDir;
    bool EnableScreenshot = false;
    int ScreenshotInterval_min = 10;
    int JpegQuality = 9;
    int GetTopWindowInterval_s = 1;
    int SaveTriggerCount_times = 60;
    int RefreshListWidgetInterval_s = 60;
    bool ShowWindowOnStart = true;
    void sync();
    void CheckAndCreateDirectories();

private:
    QSettings _settings;
    // 路径转换函数：将相对路径转换为绝对路径
    QString ToAbsolutePath(const QString& path) const ;
};

#endif // APPSETTINGS_H

