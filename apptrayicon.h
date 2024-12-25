#ifndef APPTRAYICON_H
#define APPTRAYICON_H

#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QMenu>
#include <QApplication>

class AppTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    AppTrayIcon(QMainWindow *parent = nullptr);
    ~AppTrayIcon();

private slots:
    void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
    void on_actionOpenAppDir_triggered();

private:
    QMainWindow *_parent;
    QMenu *_menu;
    void ShowParent();
    void HideParent();

};

#endif // APPTRAYICON_H
