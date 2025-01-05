#include "mainwindow.h"

#include <QApplication>
#include <QLocale>

static QFile logFile;
static QMutex logMutex;

void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&logMutex);

    if (!logFile.isOpen()) {
        return; // 日志文件未打开，直接返回
    }

    QTextStream out(&logFile);

    // 获取时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 根据消息类型生成日志前缀
    QString logLevel;
    switch (type) {
    case QtDebugMsg:
        logLevel = " [DEBUG] ";
        break;
    case QtInfoMsg:
        logLevel = " [INFO] ";
        break;
    case QtWarningMsg:
        logLevel = " [WARNING] ";
        break;
    case QtCriticalMsg:
        logLevel = " [CRITICAL] ";
        break;
    case QtFatalMsg:
        logLevel = " [FATAL] ";
        break;
    }

    // 将日志消息写入文件
    out << timestamp << logLevel << " - "<< msg << "\n";
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 使用标准的数字格式
    QLocale::setDefault(QLocale::C);

    // 配置日志
    QString logFilePath = QDir(QCoreApplication::applicationDirPath()).filePath("log.txt");
    logFile.setFileName(logFilePath);
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "无法打开日志文件:" << logFilePath;
        return -1; // 无法打开日志文件，直接退出
    }
    qInstallMessageHandler(logToFile);

    qInfo() << "开始";
    // qInfo() << "字体：" << QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
    // qInfo() << "语言：" << QLocale::system().name();

    MainWindow w;
    // 程序退出时保存数据
    QObject::connect(&a, &QCoreApplication::aboutToQuit, &w, &MainWindow::OnAppQuit);

    int val = a.exec();
    qInfo() << "退出 (" << val << ")";
    return val;
}
