#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include "exeitemwidget.h"
#include "databasemanager.h"
#include <QDate>
#include <QApplication>
#include <QtCharts>
#include "appsettings.h"

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr, AppSettings *settings = nullptr);
    ~HistoryDialog();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButtonDelete_clicked();

private:
    Ui::HistoryDialog *ui;
    AppSettings *_settings;
    DatabaseManager *_dbManager;
    QStringList _tableNameList;
    QStringList _dateList = {" "};
    // 饼图
    QChart *_chartLeft;
    QChart *_chartRight;
    bool _first12hours[12 * 60] = {false};
    bool _second12hours[12 * 60] = {false};
    // 添加数据到列表
    void AddListWidgetItem(QString name, QString iconPath, QString time, int percentage);
    // 将总秒数转化为时间文本
    QString FormatSeconds(int totalSeconds);
};

#endif // HISTORYDIALOG_H
