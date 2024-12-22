#include "apptrayicon.h"

AppTrayIcon::AppTrayIcon(QMainWindow *parent)
    : QSystemTrayIcon(parent)
{
    _parent = parent;
    setIcon(QIcon(":/img/Assets/32.ico"));
    connect(this, &QSystemTrayIcon::activated, this, &AppTrayIcon::on_trayIcon_activated);

    _menu = new QMenu();
    QAction *actionShowWindow = new QAction("显示窗口");
    connect(actionShowWindow, &QAction::triggered, this, &AppTrayIcon::ShowParent);
    _menu->addAction(actionShowWindow);
    QAction *actionExitApp = new QAction("退出");
    connect(actionExitApp, &QAction::triggered, this, &QApplication::quit);
    _menu->addAction(actionExitApp);
    setContextMenu(_menu);
    show();
}

AppTrayIcon::~AppTrayIcon()
{}

void AppTrayIcon::on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
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
            _menu->move(pos.x(), pos.y() + offsetY);
            _menu->exec();
        }
        break;
    default:
        ;
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
