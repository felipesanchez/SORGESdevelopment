#include <QApplication>
#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <QDateTime>
#include "classes/station.h"
#include "classes/origin.h"
#include "classes/dataprocessing.h"
#include "widgets/mapwidget.h"
#include "widgets/origindatawidget.h"
#include "widgets/stationsdatawidget.h"


/**Checking arguments when running the program
 * sorges
 * sorges realtime
 * sorges simulation yyyy-MM-dd hh:mm:ss
 */
std::string checkArguments(int argc, char *argv[]){

    if (argc == 1 || (argc == 2 && QString(argv[1]) == "realtime")){
        return "realtime";
    }
    else if (argc == 4 && QString(argv[1]) == "simulation"){
        QDateTime datetime =
                QDateTime::fromString(QString(argv[2])+" "+QString(argv[3]),
                                      "yyyy-MM-dd hh:mm:ss");

        if (!datetime.isValid ()){
            std::cout<<"wrong parameters.\nCorrect use:\nsorges\nsorges realtime\n";
            std::cout<<"sorges simulation yyyy-MM-dd hh:mm:ss"<<std::endl;
            exit(-1);
        }
        else
            return "simulation";
    }
    else {
        std::cout<<"wrong parameters.\nCorrect use:\nsorges\nsorges realtime\n";
        std::cout<<"sorges simulation yyyy-MM-dd hh:mm:ss"<<std::endl;
        exit(-1);
    }

}

int main(int argc, char *argv[])
{
    std::string startMode = checkArguments (argc,argv);

    QApplication a(argc, argv);

    bool simulationMode=false;
    if (startMode=="simulation")
        simulationMode = true;

    DataProcessing dataProcessor(simulationMode);
    MapWidget mapW;
    OriginDataWidget originDataW;
    StationsDataWidget stationsDataW;

    /**SIGNAL -> SLOT mapping**/

    QObject::connect (&dataProcessor, SIGNAL(stationsLoaded(std::set<Station>)),
                      &stationsDataW, SLOT(showStationsData(std::set<Station>)));

    QObject::connect (&dataProcessor, SIGNAL(stationsLoaded(std::set<Station>)),
                      &mapW, SLOT(paintStations(std::set<Station>)));

    QObject::connect (&dataProcessor, SIGNAL(originReceived(Origin)),
                      &originDataW, SLOT(showOriginData(Origin)));

    QObject::connect (&dataProcessor, SIGNAL(originReceived(Origin)),
                      &mapW, SLOT(paintOrigin(Origin)));

    QObject::connect (&dataProcessor, SIGNAL(stationColorReceived(std::set<Station>)),
                      &stationsDataW, SLOT(changeStationsData(std::set<Station>)));

    QObject::connect (&dataProcessor, SIGNAL(stationColorReceived(std::set<Station>)),
                      &mapW, SLOT(changeStationsColors(std::set<Station>)));

    QObject::connect (&dataProcessor, SIGNAL(eventReceived(Origin)),
                      &originDataW, SLOT(showOriginData(Origin)));

    QObject::connect (&dataProcessor, SIGNAL(eventReceived(Origin)),
                      &mapW, SLOT(paintOrigin(Origin)));

    mapW.show();
    originDataW.show();
    stationsDataW.show();

    if (startMode == "realtime"){
        dataProcessor.init();
    }
    else if (startMode == "simulation"){
        QDateTime datetime =QDateTime::fromString(QString(argv[2])+" "+QString(argv[3]),
                                                  "yyyy-MM-dd hh:mm:ss");
        dataProcessor.init();
        dataProcessor.initSimulation(datetime);
    }

    return a.exec();
}
