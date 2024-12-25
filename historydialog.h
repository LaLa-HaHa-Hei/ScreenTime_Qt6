#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include "exeitemwidget.h"
#include "databasemanager.h"
#include <QDate>
#include <QApplication>

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButtonDelete_clicked();

private:
    Ui::HistoryDialog *ui;
    const QString _appDir = QCoreApplication::applicationDirPath();
    const QString _exeIconDir = ".\\user-data\\exe-icon";
    const QString _dataBasePath = ".\\user-data\\sqlite3.db";
    DatabaseManager *_dbManager;
    QStringList _tableNameList;
    QStringList _dateList;
    // 添加数据到列表
    void AddListWidgetItem(QString name, QString iconPath, QString time, int percentage);
    // 将总秒数转化为时间文本
    QString FormatSeconds(int totalSeconds);
};

#endif // HISTORYDIALOG_H
