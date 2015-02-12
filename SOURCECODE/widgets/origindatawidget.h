#ifndef ORIGINDATAWIDGET_H
#define ORIGINDATAWIDGET_H

#include <QWidget>
#include <iostream>
#include "../classes/origin.h"

namespace Ui {
class OriginDataWidget;
}
class OriginDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OriginDataWidget(QWidget *parent = 0);
    ~OriginDataWidget();

public slots:
    void showOriginData(const Origin& origin);

private:
    Ui::OriginDataWidget *ui;
    Origin currentOrigin;
};

#endif // ORIGINDATAWIDGET_H
