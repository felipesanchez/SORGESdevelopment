#ifndef STATIONSDATAWIDGET_H
#define STATIONSDATAWIDGET_H

#include <QWidget>
#include <set>
#include "../classes/station.h"

namespace Ui {
class StationsDataWidget;
}

class StationsDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StationsDataWidget(QWidget *parent = 0);
    ~StationsDataWidget();

public slots:
    void showStationsData(const std::set<Station>& stations);
    void changeStationsData(const std::set<Station>& stations);

private:
    Ui::StationsDataWidget *ui;
    std::set<Station> currentStations;
};

#endif // STATIONSDATAWIDGET_H
