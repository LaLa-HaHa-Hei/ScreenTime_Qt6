#include "historydialog.h"
#include "ui_historydialog.h"
#include <QMessageBox>
#include <QFile>

HistoryDialog::HistoryDialog(QWidget *parent, AppSettings *settings)
    : QDialog(parent)
    , ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
    _settings = settings;
    // 饼图
    _chartLeft = new QChart();
    _chartLeft->legend()->hide();
    _chartLeft->setBackgroundVisible(false);
    ui->chartViewLeft->setChart(_chartLeft);
    _chartRight = new QChart();
    _chartRight->legend()->hide();
    _chartRight->setBackgroundVisible(false);
    ui->chartViewRight->setChart(_chartRight);
    // 数据库
    _dbManager = DatabaseManager::instance(_settings->DataBasePath);
    _dbManager->OpenDatabase();
    _tableNameList = _dbManager->GetTableNameList();
    // 使得今天的表名在[0]的位置
    std::sort(_tableNameList.begin(), _tableNameList.end(), [](const QString &a, const QString &b) {
        return a > b; // 按字典序降序排序, 让今天的在开头
    });
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
    QPieSeries *leftSeries = new QPieSeries();
    leftSeries->setPieSize(1);
    QPieSeries *rightSeries = new QPieSeries();
    rightSeries->setPieSize(1);
    // 今天的
    if (index == 0)
    {
        leftSeries->append("not used", 1);
        rightSeries->append("not used", 1);
        QPieSlice *lSlice = leftSeries->slices().at(0);
        QPieSlice *rSlice = rightSeries->slices().at(0);
        lSlice->setColor(QColor(200, 200, 200));
        rSlice->setColor(QColor(200, 200, 200));
        _chartLeft->addSeries(leftSeries);
        _chartRight->addSeries(rightSeries);
        return;
    }
    QString tableName = _tableNameList[index];
    int totalSeconds = _dbManager->GetTotalSeconds(tableName);
    QSqlQuery query = _dbManager->GetAppUsageData(tableName);
    while(query.next())
    {
        QString name = query.value(0).toString();
        int seconds = query.value(1).toInt();
        QString iconPath = QString("%1\\%2.png").arg(_settings->ExeIconFolder, name);
        AddListWidgetItem(name, iconPath, FormatSeconds(seconds), seconds * 100 / totalSeconds);
    }
    // 饼图
    QString filePath = QString("%1\\%2.dat").arg(_settings->UsageRecordsFolder, _dateList[index]);
    if (QFile::exists(filePath))
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly))
        {
            QDataStream in(&file);
            in.readRawData(reinterpret_cast<char*>(_first12hours), sizeof(_first12hours));
            in.readRawData(reinterpret_cast<char*>(_second12hours), sizeof(_second12hours));
            file.close();
            // 左面的饼图
            int slow = 0, fast = 1, i = 0;
            while (fast < 12 * 60)
            {
                if (_first12hours[fast] != _first12hours[slow])
                {
                    if (_first12hours[slow] == false) // 未使用
                    {
                        leftSeries->append("use", fast - slow);
                        QPieSlice *slice = leftSeries->slices().at(i++);
                        slice->setExplodeDistanceFactor(0);
                        slice->setColor(QColor(200, 200, 200));
                    }
                    else
                    {
                        leftSeries->append("not use", fast - slow);
                        QPieSlice *slice = leftSeries->slices().at(i++);
                        slice->setExplodeDistanceFactor(0);
                        slice->setColor(QColor(26, 148, 203));
                    }
                    slow = fast;
                }
                fast++;
            }
            if (_first12hours[slow] == false) // 未使用
            {
                leftSeries->append("use", fast - slow);
                QPieSlice *slice = leftSeries->slices().at(i++);
                slice->setExplodeDistanceFactor(0);
                slice->setColor(QColor(200, 200, 200));
            }
            else
            {
                leftSeries->append("not use", fast - slow);
                QPieSlice *slice = leftSeries->slices().at(i++);
                slice->setExplodeDistanceFactor(0);
                slice->setColor(QColor(26, 148, 203));
            }
            _chartLeft->addSeries(leftSeries);
            // 右面的饼图
            slow = 0;
            fast = 1;
            i = 0;
            while (fast < 12 * 60)
            {
                if (_second12hours[fast] != _second12hours[slow])
                {
                    if (_second12hours[slow] == false) // 未使用
                    {
                        rightSeries->append("not use", fast - slow);
                        QPieSlice *slice = rightSeries->slices().at(i++);
                        slice->setColor(QColor(200, 200, 200));
                    }
                    else
                    {
                        rightSeries->append("use", fast - slow);
                        QPieSlice *slice = rightSeries->slices().at(i++);
                        slice->setColor(QColor(26, 148, 203));
                    }
                    slow = fast;
                }
                fast++;
            }
            if (_second12hours[slow] == false) // 未使用
            {
                rightSeries->append("not use", fast - slow);
                QPieSlice *slice = rightSeries->slices().at(i++);
                slice->setColor(QColor(200, 200, 200));
            }
            else
            {
                rightSeries->append("use", fast - slow);
                QPieSlice *slice = rightSeries->slices().at(i++);
                slice->setColor(QColor(26, 148, 203));
            }
            _chartRight->addSeries(rightSeries);
        }
        else
        {
            qCritical() << "无法加载使用情况文件";
        }
    }
    else
    {
        leftSeries->append("not used", 1);
        rightSeries->append("not used", 1);
        QPieSlice *lSlice = leftSeries->slices().at(0);
        QPieSlice *rSlice = rightSeries->slices().at(0);
        lSlice->setColor(QColor(200, 200, 200));
        rSlice->setColor(QColor(200, 200, 200));
        _chartLeft->addSeries(leftSeries);
        _chartRight->addSeries(rightSeries);
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
        QString filePath = QString("%1\\%2.dat").arg(_settings->UsageRecordsFolder, _dateList[index]);
        if (QFile::exists(filePath))
        {
            QFile::remove(filePath);
        }
        _dbManager->DeleteTable(_tableNameList[index]);
        _tableNameList.removeAt(index);
        _dateList.removeAt(index);
        ui->comboBox->removeItem(index);
    }
}

