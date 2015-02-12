#include <QSettings>
#include <QDir>
#include <QXmlStreamReader>
#include "dataprocessing.h"
#include "simulationplanner.h"

DataProcessing::DataProcessing(bool simulationMode, QObject* parent):
    QObject(parent),
    simulationMode(simulationMode),
    watcher(this),
    config(new QSettings(QDir::currentPath()+"/config/sorges.conf",QSettings::NativeFormat)),
    bootDateTime(QDateTime::currentDateTime())
    {

    /**watching different files depending on the execution mode*/
    if (simulationMode){

        std::string stationsFile = QDir::currentPath().toStdString()
                               +"/simulationFiles/stations_alertes.txt";
        config->setValue("simulationpaths/stations",QVariant(stationsFile.c_str()));

        if (!watcher.addPath(config->value("simulationpaths/stations").toString()))
            std::cerr << "Problem to find the file: "
                      << config->value("simulationpaths/stations").toString().toStdString()
                      << std::endl;

        std::string picksFile = QDir::currentPath().toStdString()
                               +"/simulationFiles/scalertes_picks.log";
        config->setValue("simulationpaths/picks",QVariant(picksFile.c_str()));

        if (!watcher.addPath(config->value("simulationpaths/picks").toString()))
            std::cerr << "Problem to find the file: "
                      << config->value("simulationpaths/picks").toString().toStdString()
                      << std::endl;

        std::string originsFile = QDir::currentPath().toStdString()
                               +"/simulationFiles/scalertes_origenes.log";
        config->setValue("simulationpaths/origins",QVariant(originsFile.c_str()));

        if (!watcher.addPath(config->value("simulationpaths/origins").toString()))
            std::cerr << "Problem to find the file: "
                      << config->value("simulationpaths/origins").toString().toStdString()
                      << std::endl;

        std::string eventFile = QDir::currentPath().toStdString()
                               +"/simulationFiles/event.last.xml";
        config->setValue("simulationpaths/event",QVariant(eventFile.c_str()));

        if (!watcher.addPath(config->value("simulationpaths/event").toString()))
            std::cerr << "Problem to find the file: "
                      << config->value("simulationpaths/event").toString().toStdString()
                      << std::endl;

    }

    else{ //realtime

        if (!watcher.addPath(config->value("filepaths/stations")
                                            .toString().replace("$HOME",QDir::homePath())))
            std::cerr << "Problem to find the file: "
                      << config->value("filepaths/stations").toString().toStdString()
                      << std::endl;

        if (!watcher.addPath(config->value("filepaths/picks")
                                            .toString().replace("$HOME",QDir::homePath())))
            std::cerr << "Problem to find the file: "
                      << config->value("filepaths/picks").toString().toStdString()
                      << std::endl;

        if (!watcher.addPath(config->value("filepaths/origins")
                                            .toString().replace("$HOME",QDir::homePath())))
            std::cerr << "Problem to find the file: "
                      << config->value("filepaths/origins").toString().toStdString()
                      << std::endl;
        /*
        if (!watcher.addPath(config->value("filepaths/events")
                                            .toString().replace("$HOME",QDir::homePath())))
            std::cerr << "Problem to find the file: "
                      << config->value("filepaths/events").toString().toStdString()
                      << std::endl;*/
    }



    /**slot connection for receiving file changes signals**/
    /**independent of the the mode**/
    connect(&watcher,SIGNAL(fileChanged(QString)),this,SLOT(fileChangedSlot(QString)));

}

void DataProcessing::init(){
    //List of stations loaded at the start of the system with dataProcessor.init()
    processStationsFromFile(config->value("filepaths/stations")
                            .toString().replace("$HOME",QDir::homePath()));
    if (!this->stations.empty())
        emit stationsLoaded(this->stations);
}

void DataProcessing::fileChangedSlot(QString path)
  {
    //keep the watch on the file
    watcher.addPath(path);

    if (! simulationMode)
    {
        if (path ==
                 config->value(("filepaths/stations")).toString().replace("$HOME",QDir::homePath())) {
            processStationsFromFile(path);
            if (!this->stations.empty())
                emit stationsLoaded(this->stations);
        }

        else if (path ==
                      config->value(("filepaths/picks")).toString().replace("$HOME",QDir::homePath())){
            std::set<Station> changedStation = processColorStationsFromFile(path);
            if (!changedStation.empty()){
                emit stationColorReceived(changedStation);
                dumpStationXml();
            }
        }

        else if (path == config->value(("filepaths/origins")).toString().replace("$HOME",QDir::homePath())){
            processOriginFromFileLog(path);
            if(this->origin.getOriginID().length() > 0){
                emit originReceived(this->origin);
                dumpOriginXml();
            }
        }
        /*
        else if (path == config->value(("filepaths/events")).toString().replace("$HOME",QDir::homePath())){
            processOriginFromFileXml(path);
            if(this->origin.getOriginID().length() > 0){
                emit eventReceived(this->origin);
                dumpOriginXml();
            }
        }*/

        else std::cerr<<"Unrecognized file: "<<path.toStdString()<<std::endl;
    }

    else
    {
        if (path == config->value(("simulationpaths/stations")).toString()){
            processStationsFromFile(path);
            if (!this->stations.empty())
                emit stationsLoaded(this->stations);
        }

        else if (path == config->value(("simulationpaths/picks")).toString()){
            std::set<Station> changedStation = processColorStationsFromFile(path);
            if (!changedStation.empty()){
                emit stationColorReceived(changedStation);
                dumpStationXml();
            }
        }

        else if (path == config->value(("simulationpaths/origins")).toString()){
            processOriginFromFileLog(path);
            if(this->origin.getOriginID().length() > 0){
                emit originReceived(this->origin);
                dumpOriginXml();
            }
        }

        else if (path == config->value(("simulationpaths/event")).toString()){
            processOriginFromFileXml(path);
            if(this->origin.getOriginID().length() > 0){
                emit eventReceived(this->origin);
                dumpOriginXml();
            }
        }

        else std::cerr<<"Unrecognized file: "<<path.toStdString()<<std::endl;
    }

  }

void DataProcessing::processStationsFromFile(const QString &namefile){
    stations.clear();

    QString fileContent;
    QFile file(namefile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Problem to find the file: "
                  << namefile.toStdString() << std::endl;
    }
    fileContent = file.readAll();
    file.close();
    std::vector<QStringList> stationsParameters =
                                             findParameterStations(fileContent);

    std::string StationId;
    long double StationLatitude;
    long double StationLongitude;

    for(size_t i=0; i<stationsParameters.size(); i++){
        StationId = stationsParameters.at(i).at(stationsParameters.at(i)
                                                .size()-1).toStdString();
        std::string StationIdNetwork = "";
        StationLatitude = stationsParameters.at(i).at(1).toDouble();
        StationLongitude = stationsParameters.at(i).at(0).toDouble();
        stations.insert(Station(StationId,StationIdNetwork,
                                StationLatitude,StationLongitude));
    }
}

void DataProcessing::processOriginFromFileLog(const QString &namefile){
    QRegExp rxNewOrigen("NUEVO ORIGEN -");
    QString fileContent;
    QFile file(namefile);
    int pos=file.size();
    bool found = false;
    bool overflow = false;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Problem to find the file: "
                  << namefile.toStdString() << std::endl;
    }
    else{
        // Get Fragment with the same Date and Time (it contents some rubbish data).
        do{
            pos -= 4000;
            //it is not a empty file.
            if(pos < 0){
                overflow = true;
                pos = 0;
            }
            file.seek(pos);
            fileContent = file.readAll();

            if(rxNewOrigen.indexIn(fileContent) != -1)
                found = true;
        }while(found == false && overflow == false);

        // if is overflowed return.
        if(found == false){
            std::cerr<<"NO ORIGIN FOUND AT "
                       +QDateTime::currentDateTime().toString().toStdString()
                     <<std::endl;
        }
        else {
            QDateTime dateTimeOrigin = QDateTime::fromString(findParameterOriginDate(fileContent)+" "
                             +findParameterOriginTime(fileContent),"yyyy-MM-dd hh:mm:ss.z");
            if(simulationMode || dateTimeOrigin >= bootDateTime){
                //Look for parameters into log file.
                origin.setOriginID(findParameterOriginID(fileContent).toStdString());
                origin.setOriginDate(QDate::fromString(
                                     findParameterOriginDate(fileContent),"yyyy-MM-dd"));
                origin.setOriginTime(QTime::fromString(
                                     findParameterOriginTime(fileContent),"hh:mm:ss.z"));
                origin.setLatitude(findParameterOriginLatitude(fileContent).toDouble());
                origin.setLongitude(findParameterOriginLongitude(fileContent).toDouble());
                origin.setSystemDateTime(QDateTime::fromString(
                                         findParameterOriginSystemDateTime(fileContent),"yyyy-MM-dd hh:mm:ss.z"));
            }
        }
    }

}

std::set<Station> DataProcessing::processColorStationsFromFile(const QString &namefile){
    std::set<Station> stationChanged;
    QRegExp rxLastDateTime("\n\\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d");
    QString fileContent;
    QString lastTime;
    QFile file(namefile);
    int pos=file.size();
    bool found = false;
    bool overflowed = false;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Problem to find the file: "
                  << namefile.toStdString() << std::endl;
    }

    else{
        // Get Eficiently the last DateTime in File.
        do{
            pos -= 100;
            //File empty.
            if(pos < 0){
                pos = 0;
                overflowed = true;
            }

            file.seek(pos);
            fileContent = file.readAll();
        }while(rxLastDateTime.lastIndexIn(fileContent) == -1 && overflowed == false);

        // if it is overflowed.
        if(rxLastDateTime.lastIndexIn(fileContent) == -1){
            std::cerr<<"NO PICK FOUND AT "
                       +QDateTime::currentDateTime().toString().toStdString()
                    <<std::endl;
        }
        else {
            lastTime = rxLastDateTime.cap(0);
            //avoid getting data from the past before app runs
            if (simulationMode ||
                                QDateTime::fromString(lastTime,"\nyyyy-MM-dd hh:mm:ss.z")
                                >= this->bootDateTime){

                found = false;
                overflowed = false;

                // Get Fragment with the same Date and Time (it contains some rubbish data).
                do{
                    pos -= 100;
                    if(pos < 0){
                        overflowed = true;
                        pos = 0;
                    }
                    file.seek(pos);
                    fileContent = file.readAll();
                    if(rxLastDateTime.indexIn(fileContent) != -1)
                        if(rxLastDateTime.cap() != lastTime)
                            found = true;
                }while(found == false && overflowed == false);

                // Look for the Station Alert line, and take the values.
                QRegExp rxLineAlert(lastTime
                                    +"\tEstaci\\S+ \\S+\tNivel de Alerta:\\d");
                rxLineAlert.lastIndexIn(fileContent);
                if(rxLineAlert.lastIndexIn(fileContent) != -1){
                    QRegExp rxStationRm("\n\\d\\d\\d\\d-\\d+-\\d+ \\d+:\\d+:\\d+.\\d\tEstaci\\S+ ");
                    QRegExp rxColourRm("Nivel de Alerta:");
                    QStringList parameters = rxLineAlert.cap()
                                             .remove(rxStationRm)
                                             .remove(rxColourRm).split("\t");
                    // Look for a station with the same ID and extract it
                    std::set<Station>::iterator it = stations
                                                    .find(Station(parameters.at(0)
                                                                  .toStdString()));
                    if(it != stations.end()){
                        Station st(*it);
                        // Change color and update this value.
                        st.setColor(parameters.at(1).toInt());
                        stations.erase(it);
                        stations.insert(st);

                        stationChanged.insert(st);
                    }
                }

            }
        }
    }
    return stationChanged;
}

void DataProcessing::processOriginFromFileXml(const QString &namefile){
    std::set<Station> mystations;
    QStringList networkID, stationID;
    QString originID;
    QString eventID;
    QString magnitudeID;
    long double originLatitude=0, originLongitude=0;
    double originMagnitude=0.0;
    QString dateTime;
    QDate originDate;
    QTime originTime;
    QRegExp rxDate("\\d+-\\d+-\\d+");
    QRegExp rxTime("\\d+:\\d+:\\d+.\\d\\d?\\d?");
    QDomElement element;

    QDomDocument doc(namefile);
    QFile file(namefile);
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();


    // Getting the pick values:
    QDomNodeList picks = doc.elementsByTagName("pick");
    for (int i = 0; i < picks.size(); i++) {
        QDomNode element = picks.item(i);
        QDomElement waveformID = element.firstChildElement("waveformID");
        if(waveformID.hasAttribute("stationCode"))
            stationID << waveformID.attribute("stationCode");
        if(waveformID.hasAttribute("networkCode"))
            networkID << waveformID.attribute("networkCode");
    }

    // Getting the Origin Value:
    QDomNodeList origins = doc.elementsByTagName("origin");
    for (int i = 0; i < origins.size(); i++) {
        QDomNode origin = origins.item(i);
        if(origin.isElement()){
            QDomElement element = origin.toElement();
            if(element.hasAttribute("publicID"))
            originID = element.attribute("publicID");
        }
        originLatitude = origin.firstChildElement("latitude")
                                   .firstChildElement("value").text().toDouble();
        originLongitude = origin.firstChildElement("longitude")
                                   .firstChildElement("value").text().toDouble();
    }

    //Get Event values:
    QDomNodeList events = doc.elementsByTagName("event");
    for (int i = 0; i < events.size(); i++) {
        QDomNode event = events.item(i);
        if(event.isElement()){
            element = event.toElement();
            if(element.hasAttribute("publicID")){
                eventID = element.attribute("publicID");
            }
            dateTime = event.firstChildElement("preferredOriginID").text();
            dateTime = dateTime.remove("Origin#");
            QStringList components = dateTime.split(".");
            QDateTime eventTime = QDateTime::fromString(components.at(0),"yyyyMMddhhmmss");
            eventTime = eventTime.addMSecs(components.at(1).at(0).digitValue()*100);
            originDate = eventTime.date ();
            originTime = eventTime.time ();

            magnitudeID = event.firstChildElement("preferredMagnitudeID").text();
        }
    }

    // Getting the magnitudes values
    QDomNodeList magnitudes = doc.elementsByTagName("magnitude");
    for (int i = 0; i < magnitudes.size(); i++) {
        QDomNode magnitude = magnitudes.item(i);
        if(magnitude.hasAttributes() && magnitude.isElement()){
            element = magnitude.toElement();
            if(element.hasAttribute("publicID"))
                if(element.attribute("publicID") == magnitudeID)
                    if(!magnitude.firstChildElement("magnitude").hasAttributes())
                        originMagnitude = magnitude.firstChildElement("magnitude")
                                          .firstChildElement("value").text().toDouble();
        }
    }

    // PRINTING AND SETTING ORIGIN AND STATIONS.

        origin.setOriginID(eventID.toStdString()+"\t"+originID.toStdString());
        origin.setLatitude(originLatitude);
        origin.setLongitude(originLongitude);
        origin.setOriginDate(originDate);
        origin.setOriginTime(originTime);
        for(int i=0; i<stationID.size(); i++){
            std::set<Station>::iterator it = stations.find(Station(stationID.at(i).toStdString()));
            if(it != stations.end()){
                Station st(*it);
                st.setNetworkID(networkID.at(i).toStdString());
                stations.erase(it);
                stations.insert(st);
                mystations.insert(st);
            }
        }
        origin.setStations(mystations);
        origin.setMagnitude(originMagnitude);

}


std::vector<QStringList> DataProcessing::findParameterStations(const QString &stationsString){
    std::vector<QStringList> StationParameters;
    QStringList eachStationsFile = stationsString.split("\n");
    QString oneStation;

    for (int i = 0; i < eachStationsFile.size(); ++i){
        oneStation = QString(eachStationsFile.at(i));
        if(!oneStation.isEmpty())
            StationParameters.push_back(oneStation.split("\t"));
    }
    return StationParameters;
}

QString DataProcessing::findParameterOriginID(const QString &originString){
     QRegExp rx("Origin#\\S+");
     if(rx.indexIn(originString) != -1)
        return rx.cap(0);
     return QString();
}

QString DataProcessing::findParameterOriginDate(const QString &originString){
    QRegExp rx("CreationTime= \\d+-\\d+-\\d+");
    QRegExp rx1("CreationTime= ");
    if(rx.indexIn(originString) != -1)
        return rx.cap(0).remove(rx1);
    return QString();
}

QString DataProcessing::findParameterOriginTime(const QString &originString){
    QRegExp rx("CreationTime= \\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d");
    QRegExp rx1("CreationTime= \\d+-\\d+-\\d+ ");
    if(rx.indexIn(originString) != -1)
        return rx.cap(0).remove(rx1);
    return QString();
}

QString DataProcessing::findParameterOriginLatitude(const QString &originString){
    QRegExp rx("origen->latitud = \\S+[(]");
    QRegExp rx2("origen->latitud = ");
    if(rx.lastIndexIn(originString) != -1){
        return rx.cap(0).remove(rx2).remove("(");
    }
    return QString();
}

QString DataProcessing::findParameterOriginLongitude(const QString &originString){
    QRegExp rx("origen->longitug = \\S+[(]");
    QRegExp rx2("origen->longitug = ");
    if(rx.lastIndexIn(originString) != -1)
        return rx.cap(0).remove(rx2).remove("(");
    return QString();
}

QString DataProcessing::findParameterOriginSystemDateTime(const QString &originString){
    QRegExp rx("\n\\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d+");
    if(rx.lastIndexIn(originString) != -1)
        return rx.cap(0).remove("\n");
    return QString();
}

void DataProcessing::dumpOriginXml(){

    QDir current(QDir::currentPath());
    current.mkdir("backup");

    QFile file(
          QString::fromStdString(QDir::currentPath().toStdString ()
                                 +"/backup/"+origin.getOriginID()+".xml"));
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text)){
        std::cerr << "Problem to create origin backup file: " <<  "xml" << std::endl;
        return;
    }

    QTextStream out(&file);
    out << QString::fromStdString("<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"+origin.toStringXml());
    file.close();

}

void DataProcessing::dumpStationXml(){

    QDir current(QDir::currentPath());
    current.mkdir("backup");

    QFile file(
          QString::fromStdString(QDir::currentPath().toStdString()
                                 +"/backup/Stations#"+QDateTime::currentDateTime()
                                 .toString("yyyyMMddhhmmss.zzz")
                                 .toStdString()+".xml"));
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text)){
        std::cerr << "Problem to create station backup file: " <<  "xml" << std::endl;
        return;
    }

    QTextStream out(&file);
    out << QString::fromStdString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Stations>\n"
                                  +Station::stationsToStringXml(stations,"\t"))+"</Stations>";
    file.close();

}




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// SIMULATION METHODS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DataProcessing::initSimulation(QDateTime simulationDateTime){

    /*First, search for the closest event to the required DateTime*/
    /*Path from /home/USERNAME/.seiscomp3/log/events*/
    //QDir eventsDir(QDir::homePath()+"/.seiscomp3/log/events");
    QString path = config->value("filepaths/events").toString();
    if (path.startsWith("$HOME")){
        path.replace("$HOME",QDir::homePath());
    }
    QDir eventsDir(path);

    if (! eventsDir.exists ()){
        std::cerr<<"NOT FOUND directory path ~/.seiscomp3/log/events"<<std::endl;
        std::cerr<<"EVENT FILES CANNOT BE FOUND"<<std::endl;
        exit(-1);
    }
    else{

        QString year = QString::number(simulationDateTime.date().year());

        QString month = QString::number(simulationDateTime.date().month());
        if (simulationDateTime.date().month()<10) month = "0"+month;

        QString day = QString::number(simulationDateTime.date().day());
        if (simulationDateTime.date().day()<10) day = "0"+day;

        /*Events organized in hierarchy yyyy/MM/dd/eventName */

        QString requiredEvent("");
        QDateTime requiredEventDateTime;
        double timeDiff = std::numeric_limits<double>::max();

        QDir eventsForDateTime(eventsDir.absolutePath ()+"/"+year+"/"+month+"/"+day);
        if (eventsForDateTime.exists()){
            /*Searching all the events in the same day month and year*/
            foreach(QString eventName,eventsForDateTime.entryList()){
                //avoiding unix directories elements . and ..
                if (eventName.toStdString() != "."
                                                && eventName.toStdString() != "..")
                {
                    QDir eventFiles(eventsForDateTime.absolutePath()+"/"+eventName);
                    QDateTime eventDateTime = getDateTimeFromEvent(eventFiles,eventName);

                    double range = abs(eventDateTime.msecsTo(simulationDateTime));
                    if (range<timeDiff){
                        timeDiff = range;
                        requiredEvent = eventName;
                        requiredEventDateTime  = eventDateTime;
                    }
                }
            }
        }

        else {
            std::cout<<"No events stored for the requested date"<<std::endl;
            std::cout<<"Searching previous and next day"<<std::endl;
        }

        /*In case of less difference with the nex ot previous days events*/
        QDateTime prevDayDateTime;
        prevDayDateTime.setDate(simulationDateTime.date().addDays(-1));
        prevDayDateTime.setTime(QTime(23,59,59));
        QDateTime nextDayDateTime;
        nextDayDateTime.setDate(simulationDateTime.date().addDays(1));
        nextDayDateTime.setTime(QTime(0,0,0));

        double timeDiffPrevDay = abs(simulationDateTime.msecsTo(prevDayDateTime));
        double timeDiffNextDay = abs(simulationDateTime.msecsTo(nextDayDateTime));

        if (timeDiffPrevDay < timeDiffNextDay){
            /*Searching in the prev day*/
            QString prevDayYear = QString::number(prevDayDateTime.date().year());

            QString prevDayMonth = QString::number(prevDayDateTime.date().month());
            if (prevDayDateTime.date().month()<10) prevDayMonth = "0"+prevDayMonth;

            QString prevDay = QString::number(prevDayDateTime.date().day());
            if (prevDayDateTime.date().day()<10) prevDay = "0"+prevDay;

            QDir eventsForPrevDayDateTime(eventsDir.absolutePath()
                                          +"/"+prevDayYear+"/"+prevDayMonth+"/"+prevDay);
            if (eventsForPrevDayDateTime.exists()){
                /*Searching all the events in the same day month and year*/
                foreach(QString eventName,eventsForPrevDayDateTime.entryList()){
                    //avoiding unix directories elements . and ..
                    if (eventName.toStdString() != "."
                                                    && eventName.toStdString() != "..")
                    {
                        QDir eventFiles(eventsForPrevDayDateTime.absolutePath()+"/"+eventName);
                        QDateTime eventDateTime = getDateTimeFromEvent(eventFiles,eventName);

                        double range = abs(eventDateTime.msecsTo(simulationDateTime));
                        if (range<timeDiff){
                            timeDiff = range;
                            requiredEvent = eventName;
                            requiredEventDateTime  = eventDateTime;
                        }
                    }
                }
            }
        }
        else {
            /*Searching in the next day*/
            QString nextDayYear = QString::number(nextDayDateTime.date().year());

            QString nextDayMonth = QString::number(nextDayDateTime.date().month());
            if (nextDayDateTime.date().month()<10) nextDayMonth = "0"+nextDayMonth;

            QString nextDay = QString::number(nextDayDateTime.date().day());
            if (nextDayDateTime.date().day()<10) nextDay = "0"+nextDay;

            QDir eventsForNextDayDateTime(eventsDir.absolutePath()
                                          +"/"+nextDayYear+"/"+nextDayMonth+"/"+nextDay);
            if (eventsForNextDayDateTime.exists()){
                /*Searching all the events in the same day month and year*/
                foreach(QString eventName,eventsForNextDayDateTime.entryList()){
                    //avoiding unix directories elements . and ..
                    if (eventName.toStdString() != "."
                                                    && eventName.toStdString() != "..")
                    {
                        QDir eventFiles(eventsForNextDayDateTime.absolutePath()+"/"+eventName);
                        QDateTime eventDateTime = getDateTimeFromEvent(eventFiles,eventName);

                        double range = abs(eventDateTime.msecsTo(simulationDateTime));
                        if (range<timeDiff){
                            timeDiff = range;
                            requiredEvent = eventName;
                            requiredEventDateTime  = eventDateTime;
                        }
                    }
                }
            }
        }

        /*Event found on required, previous or next day or not found in them*/
        if (requiredEvent == ""){
            std::cerr<<"NO EVENT FOUND CLOSE TO THE REQUESTED TIMESTAMP OR NEXT/PREVIOUS DAY"<<std::endl;
            exit(-1);
        }
        else{
            std::cout<<"Starting simulation of EVENT "
                       +requiredEvent.toStdString()<<std::endl;
            std::cout<<"Timestamp of the event: "
                       +requiredEventDateTime.toString("yyyy-MM-dd hh:mm:ss.z")
                       .toStdString ()<<std::endl;
        }

        QString requiredYear = QString::number(requiredEventDateTime.date().year());

        QString requiredMonth = QString::number(requiredEventDateTime.date().month());
        if (requiredEventDateTime.date().month()<10) requiredMonth = "0"+requiredMonth;

        QString requiredDay = QString::number(requiredEventDateTime.date().day());
        if (requiredEventDateTime.date().day()<10) requiredDay = "0"+requiredDay;

        QDir requiredEventDir(eventsDir.absolutePath ()
                              +"/"+requiredYear
                              +"/"+requiredMonth
                              +"/"+requiredDay
                              +"/"+requiredEvent);

        QDateTime firstDateTime = getDateTimeFirstPick(requiredEventDir,requiredEvent);
        QDateTime lastDateTime = getLastDateTimeFromEvent(requiredEventDir,requiredEvent);
        //in order to avoid losing data due to miliseconds accuracy in logs end lines
        lastDateTime = lastDateTime.addMSecs(500);

        std::cout<<"First pick of the simulation at "
                   +firstDateTime.toString("yyyy-MM-dd hh:mm:ss.z")
                   .toStdString()<<std::endl;

        std::cout<<"End of gathering simulation data at "
                   +lastDateTime.toString("yyyy-MM-dd hh:mm:ss.z")
                   .toStdString()<<std::endl;

        simulationFirstDateTime = firstDateTime;
        simulationLastDateTime = lastDateTime;
        simulationDuration = abs(firstDateTime.msecsTo(lastDateTime));

        /*Getting the string blocks from the logs in the datetimes range*/
        QString picksString = getBlockPick (firstDateTime,lastDateTime);
        QString originsString = getBlockOrigin (firstDateTime,lastDateTime);
        QString allString = picksString+"\n"+originsString;
        std::cout<<"Reading data blocks.."<<std::endl;

        /*Split them in blocks with the same datetime*/
        std::set<QPair<QStringList, QDateTime> > picksByDate = getDateTimeBlocks(picksString);
        std::set<QPair<QStringList, QDateTime> > originsByDate = getDateTimeBlocks(originsString);
        std::set<QPair<QStringList, QDateTime> > allByDate = getDateTimeBlocks(allString);

        /*And now, once sorted the blocks, so as to plan the simulation we get
         * the blocks together with the difference of miliseconds between them in the logs*/
        QList<QPair<QString, int> > picksBlocks = getSecuence(picksByDate);
        QList<QPair<QString, int> > originsBlocks = getSecuence(originsByDate);
        QList<QPair<QString, int> > allBlocks = getSecuence(allByDate);
        std::cout<<"Calculating time ranges"<<std::endl;

        /*And let the planner take over of the simulation*/
        simulationPlanner = new SimulationPlanner(requiredEvent,requiredEventDir,
                                                  allBlocks,picksBlocks,originsBlocks,simulationDuration);

    }
}

QDateTime DataProcessing::getDateTimeFromEvent(QDir eventFiles,QString eventName){

    QString namefile = eventFiles.absoluteFilePath(eventName+".last.xml");
    QDomDocument xml(namefile);
    QFile file(namefile);
    if (!file.open(QIODevice::ReadOnly)){
        std::cerr << "Problem to find the file: "+eventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached, simulation aborted"<<std::endl;
        exit(-1);
    }
    if (!xml.setContent(&file)) {
        std::cerr << "Problem to read the content: "+eventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached, simulation aborted"<<std::endl;
        file.close();
        exit(-1);
    }
    file.close();

    QDomNodeList eventTagList = xml.elementsByTagName("event");
    QDomNode eventTag = eventTagList.item(0);
    QString originID = eventTag.firstChildElement("preferredOriginID").text();

    QString eventTimeString = originID.remove("Origin#");
    QStringList components = eventTimeString.split(".");
    QDateTime eventTime = QDateTime::fromString(components.at(0),"yyyyMMddhhmmss");
    eventTime = eventTime.addMSecs(components.at(1).at(0).digitValue()*100);
    return eventTime;
}

QDateTime DataProcessing::getDateTimeFirstPick(QDir eventFiles,QString eventName){

    QString namefile = eventFiles.absoluteFilePath(eventName+".last.xml");
    QDomDocument xml(namefile);
    QFile file(namefile);
    if (!file.open(QIODevice::ReadOnly)){
        std::cerr << "Problem to find the file: "+eventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached, simulation aborted"<<std::endl;
        exit(-1);
    }
    if (!xml.setContent(&file)) {
        std::cerr << "Problem to read the content: "+eventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached, simulation aborted"<<std::endl;
        file.close();
        exit(-1);
    }
    file.close();

    QDomNodeList eventParameters = xml.elementsByTagName("EventParameters");
    QDomNode eventParameterTag = eventParameters.item(0);
    QString pickFirstTimeString = eventParameterTag.firstChildElement("pick")
                                           .firstChildElement("time")
                                           .firstChildElement("value").text();
    pickFirstTimeString.replace(QChar('T'),QChar(' '));
    pickFirstTimeString.remove(QChar('Z'));
    QStringList components = pickFirstTimeString.split(".");
    QDateTime pickFirstTime = QDateTime::fromString(components.at(0),
                                                    "yyyy-MM-dd hh:mm:ss");
    pickFirstTime = pickFirstTime.addMSecs(components.at(1).at(0).digitValue()*100);

    return pickFirstTime;
}

QDateTime DataProcessing::getLastDateTimeFromEvent(QDir eventFiles,QString eventName){

    QString namefile = eventFiles.absoluteFilePath(eventName+".last.xml");
    QDomDocument xml(namefile);
    QFile file(namefile);
    if (!file.open(QIODevice::ReadOnly)){
        std::cerr << "Problem to find the file: "+eventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached, simulation aborted"<<std::endl;
        exit(-1);
    }
    if (!xml.setContent(&file)) {
        std::cerr << "Problem to read the content: "+eventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached, simulation aborted"<<std::endl;
        file.close();
        exit(-1);
    }
    file.close();

    QDomNodeList eventTagList = xml.elementsByTagName("event");
    QDomNode eventTag = eventTagList.item(0);
    QString modificationTimeString = eventTag.firstChildElement("creationInfo")
                                       .firstChildElement("modificationTime").text();

    modificationTimeString.replace(QChar('T'),QChar(' '));
    modificationTimeString.remove(QChar('Z'));
    QStringList components = modificationTimeString.split(".");
    QDateTime modificationTime = QDateTime::fromString(components.at(0),
                                                    "yyyy-MM-dd hh:mm:ss");
    modificationTime = modificationTime.addMSecs(components.at(1).at(0).digitValue()*100);

    return modificationTime;
}

QString DataProcessing::getBlockPick(const QDateTime& firstdatetime, const QDateTime& lastdatetime){
    int posBegin, posEnd;
    QString blockPick;
    posBegin = getPositionBegin(firstdatetime,
                                config->value("filepaths/picks").toString()
                                .replace("$HOME",QDir::homePath()));
    posEnd = getPositionEnd(lastdatetime,
                            config->value("filepaths/picks").toString()
                            .replace("$HOME",QDir::homePath()));
    QFile file(config->value("filepaths/picks").toString().replace("$HOME",QDir::homePath()));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Problem to find the file: "
                  <<config->value("filepaths/picks").toString().toStdString()
                  << std::endl;
    }
    if(posBegin < posEnd){
        file.seek(posBegin);
        blockPick = file.read(posEnd - posBegin);
    }
    return blockPick;

}

QString DataProcessing::getBlockOrigin(const QDateTime& firstdatetime, const QDateTime& lastdatetime){
    int posBegin, posEnd;
    QString blockOrigin;
    posBegin = getPositionBegin(firstdatetime,
                                config->value("filepaths/origins").toString()
                                .replace("$HOME",QDir::homePath()));
    posEnd = getPositionEnd(lastdatetime,
                            config->value("filepaths/origins").toString()
                            .replace("$HOME",QDir::homePath()));
    QFile file(config->value("filepaths/origins").toString().replace("$HOME",QDir::homePath()));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Problem to find the file: "
                  <<config->value("filepaths/origins").toString().toStdString()
                  << std::endl;
    }
    if(posBegin < posEnd){
        file.seek(posBegin);
        blockOrigin = file.read(posEnd - posBegin);
    }
    return blockOrigin;

}



int DataProcessing::getPositionBegin(const QDateTime& firstdatetime, const QString& namefile){
    QRegExp rxDateBlock("\\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d");
    QString fileContent;
    QFile file(namefile);
    bool found = false;
    int pos = 0;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    while(!file.atEnd() && !found){
        pos = file.pos();
        fileContent = file.readLine();
        if(rxDateBlock.indexIn(fileContent) != -1 ){
            if(QDateTime::fromString(rxDateBlock.cap(0).remove("\n"),"yyyy-MM-dd hh:mm:ss.z")
               >= firstdatetime)
                found = true;
        }
    }
    if (found)
        return pos;
    else
        return file.size ();

}



int DataProcessing::getPositionEnd(const QDateTime &lastdatetime, const QString& namefile){
    QRegExp rxDateBlock("\\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d");
    QString fileContent;
    QFile file(namefile);
    bool found = false;
    int pos = 0;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }
    while(!file.atEnd() && !found){
        pos = file.pos();
        fileContent = file.readLine();
        if(rxDateBlock.indexIn(fileContent) != -1 ){
            if(QDateTime::fromString(rxDateBlock.cap(0),"yyyy-MM-dd hh:mm:ss.z")
            > lastdatetime){
                file.seek(pos);
                found = true;
            }
        }
    }
    if (found)
        return pos;
    else
        return file.size ();

}


std::set<QPair<QStringList, QDateTime> > DataProcessing::getDateTimeBlocks(const QString &block){
    QRegExp rxDateBlock("\\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d");
    std::set<QPair<QStringList,QDateTime> > dataBlocks;
    QStringList blocks = block.split("\n");

    for(int i = 0; i<blocks.size(); i++){
        if (blocks.size() > 0){
            if(rxDateBlock.indexIn(blocks[i]) != -1){
                QPair<QStringList,QDateTime>
                        myPair(QStringList()<<blocks[i],
                                              QDateTime::fromString(rxDateBlock.cap(0),
                                                                    "yyyy-MM-dd hh:mm:ss.z"));

                std::set<QPair<QStringList,QDateTime> >::iterator it = dataBlocks.find(myPair);
                if (it != dataBlocks.end()){
                    QPair<QStringList,QDateTime>  temp(*it);
                    temp.first = temp.first << myPair.first;
                    dataBlocks.erase(it);
                    dataBlocks.insert(temp);

                }else
                    dataBlocks.insert(myPair);
            }
        }
    }
    return dataBlocks;

}

bool operator < (const QPair<QStringList,QDateTime> & block1, const QPair<QStringList,QDateTime> & block2){
    return block1.second < block2.second;
}


QList<QPair<QString, int> >
DataProcessing::getSecuence(const std::set<QPair<QStringList,QDateTime> >& blocks){
    QString blockString;
    QList<QPair<QString,int> > animationBlock;
    std::set<QPair<QStringList,QDateTime> >::iterator it2;
    QPair<QString,int> mySecuence;
    for(std::set<QPair<QStringList,QDateTime> >::iterator it = blocks.begin(); it != blocks.end(); ++it){
        blockString.clear();
        for(int i=0; i<it->first.size(); i++){
            blockString += "\n" + it->first.at(i);
        }

        it2 = it;
        if(it != blocks.begin())
            --it2;

        mySecuence = QPair<QString,int>(blockString,it2->second.msecsTo(it->second));
        animationBlock.push_back(mySecuence);
    }
    it2 = blocks.end();
    --it2;

    return animationBlock;
}
