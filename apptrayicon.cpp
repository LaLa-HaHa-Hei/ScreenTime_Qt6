#include "apptrayicon.h"
#include <QProcess>

AppTrayIcon::AppTrayIcon(QMainWindow *parent)
    : QSystemTrayIcon(parent)
{
    _parent = parent;
    setIcon(QIcon(":/img/Assets/32.ico"));
    setToolTip("屏幕使用时间");
    connect(this, &QSystemTrayIcon::activated, this, &AppTrayIcon::on_trayIcon_activated);

    _menu = new QMenu();
    QAction *actionOpenAppDir = new QAction("打开安装目录");
    connect(actionOpenAppDir, &QAction::triggered, this, &AppTrayIcon::on_actionOpenAppDir_triggered);
    _menu->addAction(actionOpenAppDir);
    QAction *actionShowWindow = new QAction("显示窗口");
    connect(actionShowWindow, &QAction::triggered, this, &AppTrayIcon::ShowParent);
    _menu->addAction(actionShowWindow);
    QAction *actionExitApp = new QAction("退出程序");
    connect(actionExitApp, &QAction::triggered, this, &QApplication::quit);
    _menu->addAction(actionExitApp);
    setContextMenu(_menu);
    show();
}

AppTrayIcon::~AppTrayIcon()
{
}

void AppTrayIcon::on_actionOpenAppDir_triggered()
{
    QString exeDir = QCoreApplication::applicationDirPath();
    exeDir.replace("/", "\\");
    QProcess::startDetached("explorer", QStringList() << exeDir);
}

void AppTrayIcon::on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::MiddleClick:
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if (_parent->isVisible())
        {
            HideParent();
        }
        else
        {
            ShowParent();
        }
        break;
    case QSystemTrayIcon::Context:
        {
            QPoint pos = QCursor::pos();
            int offsetY = -_menu->sizeHint().height();
            _menu->popup(QPoint(pos.x(), pos.y() + offsetY));
        }
        break;
    case QSystemTrayIcon::Unknown:
        break;
    }
}

void AppTrayIcon::ShowParent()
{
    _parent->show();
    _parent->activateWindow();
}
void AppTrayIcon::HideParent()
{
    _parent->hide();
}
