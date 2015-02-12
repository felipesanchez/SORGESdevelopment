#include "stationsdatawidget.h"
#include "ui_stationsdatawidget.h"

StationsDataWidget::StationsDataWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StationsDataWidget)
{
    ui->setupUi(this);
}

StationsDataWidget::~StationsDataWidget()
{
    delete ui;
}

void StationsDataWidget::showStationsData(const std::set<Station>& stations){
    currentStations = stations;
    ui->textEdit_stations->clear();
    for(std::set<Station>::iterator it=currentStations.begin();
                                    it!=currentStations.end();++it){
        ui->textEdit_stations->append((*it).stationToString().c_str());
    }
}

void StationsDataWidget::changeStationsData(const std::set<Station>& stations){
    foreach(Station station,stations){
        currentStations.erase(currentStations.find(station));
        currentStations.insert(station);
        showStationsData(currentStations);
    }
}

