#ifndef EXEITEMWIDGET_H
#define EXEITEMWIDGET_H

#include <QWidget>

namespace Ui {
class ExeItemWidget;
}

class ExeItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExeItemWidget(QWidget *parent = nullptr, QString name = "Name", QString iconPath = ":/img/Assets/unknowfile.png", QString time = "0秒", int percentage = 0);
    ~ExeItemWidget();

private:
    Ui::ExeItemWidget *ui;
};

#endif // EXEITEMWIDGET_H
