#include "historydialog.h"
#include "ui_historydialog.h"
#include <QMessageBox>

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
    // 数据库
    _dbManager = DatabaseManager::instance(QString("%1\\%2").arg(_appDir, _dataBasePath));
    _dbManager->OpenDatabase();
    _tableNameList = _dbManager->GetTableNameList();
    // 使得今天的表名在[0]的位置
    std::sort(_tableNameList.begin(), _tableNameList.end(), [](const QString &a, const QString &b) {
        return a > b; // 按字典序降序排序, 让今天的在开头
    });
    _dateList = {" "};
    for (int i = 1; i < _tableNameList.size(); i++)
    {
        QString year = _tableNameList[i].mid(7, 4);
        QString month = _tableNameList[i].mid(11, 2);
        QString day = _tableNameList[i].mid(13, 2);
        _dateList.append(QString("%1-%2-%3").arg(year, month, day));
    }
    ui->comboBox->addItems(_dateList);
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::AddListWidgetItem(QString name, QString iconPath, QString time, int percentage)
{
    ExeItemWidget *widget = new ExeItemWidget(ui->listWidget, name, iconPath, time, percentage);
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(widget->sizeHint());
    ui->listWidget->setItemWidget(item, widget);
}

void HistoryDialog::on_comboBox_currentIndexChanged(int index)
{
    ui->listWidget->clear();
    // 今天的
    if (index == 0)
        return;
    QString tableName = _tableNameList[index];
    int totalSeconds = _dbManager->GetTotalSeconds(tableName);
    QSqlQuery query = _dbManager->GetAppUsageData(tableName);
    while(query.next())
    {
        QString name = query.value(0).toString();
        int seconds = query.value(1).toInt();
        QString iconPath = QString("%1\\%2\\%3.png").arg(_appDir, _exeIconDir, name);
        AddListWidgetItem(name, iconPath, FormatSeconds(seconds), seconds * 100 / totalSeconds);
    }
}

QString HistoryDialog::FormatSeconds(int totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    QString result;
    if (hours > 0) {
        result += QString("%1小时").arg(hours);
    }
    if (minutes > 0 || hours > 0) { // 如果有小时，也显示分钟部分
        result += QString("%1分").arg(minutes);
    }
    result += QString("%1秒").arg(seconds);

    return result;
}

void HistoryDialog::on_pushButtonDelete_clicked()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("确认删除");
    msgBox.setText("你确定要删除这天的记录吗？");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes)
    {
        int index = ui->comboBox->currentIndex();
        if (index == 0)
        {
            return;
        }
        _dbManager->DeleteTable(_tableNameList[index]);
        _tableNameList.remove(index);
        _dateList.remove(index);
        ui->comboBox->removeItem(index);
    }
}

