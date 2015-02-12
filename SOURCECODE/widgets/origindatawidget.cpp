#include<sstream>
#include "origindatawidget.h"
#include "ui_origindatawidget.h"

OriginDataWidget::OriginDataWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OriginDataWidget)
{
    ui->setupUi(this);
}

OriginDataWidget::~OriginDataWidget()
{
    delete ui;
}

std::string ldoubleToString(const long double& number){
    std::ostringstream convert;
    convert << number;
    return convert.str();
}

void OriginDataWidget::showOriginData(const Origin& origin){
    currentOrigin = origin;

    ui->textEdit_originid->setPlainText(QString(origin.getOriginID().c_str()));

    ui->textEdit_timestamp->setPlainText(QString(origin.getOriginDate()
                                                 .toString("dd-MM-yyyy"))
                                         +" "+QString(origin.getOriginTime()
                                                      .toString("hh:mm:ss.zzz t")));
    ui->textEdit_latitude->setPlainText(
                           QString(ldoubleToString(origin.getLatitude()).c_str()));
    ui->textEdit_longitude->setPlainText(
                           QString(ldoubleToString(origin.getLongitude ()).c_str()));
    ui->textEdit_magnitude->setPlainText(
                           QString(ldoubleToString(origin.getMagnitude ()).c_str()));

    std::set<Station> stations = origin.getStations();
    for(std::set<Station>::iterator it=stations.begin();it!=stations.end(); ++it){
        ui->textEdit_stations->append((*it).stationToString().c_str());
    }

}

