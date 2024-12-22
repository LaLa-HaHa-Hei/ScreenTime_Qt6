#include "exeitemwidget.h"
#include "ui_exeitemwidget.h"

ExeItemWidget::ExeItemWidget(QWidget *parent, QString name, QString iconPath, QString time, int percentage)
    : QWidget(parent)
    , ui(new Ui::ExeItemWidget)
{
    ui->setupUi(this);
    ui->labelName->setText(name);
    ui->labelTime->setText(time);
    ui->progressBar->setValue(percentage);
    QPixmap pixmap(iconPath);
    if (pixmap.isNull())
    {
        QPixmap pixmap2(":/img/Assets/unknowfile.png");
        ui->labelIcon->setPixmap(pixmap2);
    }
    else
    {
        ui->labelIcon->setPixmap(pixmap);
    }
}

ExeItemWidget::~ExeItemWidget()
{
    delete ui;
}
