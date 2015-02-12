#include "simulationplanner.h"
#include <iostream>
#include <QTime>
#include <QTextStream>
#include <QFile>
/*
 * AQUI FELIPE
 *
 * IBAS A PONER TMB UN QSETTINGS
 *
 *
 */
SimulationPlanner::SimulationPlanner(){}

SimulationPlanner::SimulationPlanner(QString event, QDir eventDir,
                                     QList<QPair<QString,int> > all,
                                     QList<QPair<QString,int> > picks,
                                     QList<QPair<QString,int> > origins,
                                     int duration,
                                     QObject *parent) :
    QObject(parent),
    requiredEventName(event),
    requiredEventDir(eventDir),
    allBlocks(all),
    picksBlocks(picks),
    originsBlocks(origins),
    simulationDuration(duration),
    eventTimer(new QTimer(this)),
    blocksTimer(new QTimer(this)),
    blocksCounter(0),
    originsCounter(0),
    picksCounter(0),
    config(new QSettings(QDir::currentPath()+"/config/sorges.conf",QSettings::NativeFormat))
{
    //to start the simulation with a bit of delay in order to set everything
    int waitTime=2000;
    std::cout<<"Starting planner in 2 seconds..."<<std::endl;

    connect (blocksTimer,SIGNAL(timeout()),this,SLOT(dispatch()));

    connect (this,SIGNAL(pickTurn(QString)),this,SLOT(sendPick(QString)));
    connect (this,SIGNAL(originTurn(QString)),this,SLOT(sendOrigin(QString)));

    std::cout<<"Starting time triggers for data..."<<std::endl;
    blocksTimer->start(waitTime);
}

void SimulationPlanner::dispatch(){

    QString generalBlock = allBlocks[blocksCounter].first;
    QString pickBlock;
    QString originBlock;

    if (picksCounter < picksBlocks.size())
        pickBlock = picksBlocks[picksCounter].first;

    if (originsCounter < originsBlocks.size())
        originBlock = originsBlocks[originsCounter].first;

    if (generalBlock == pickBlock){
        emit pickTurn (pickBlock);
        picksCounter++;
    }
    else if(generalBlock == originBlock){
        emit originTurn (originBlock);
        originsCounter++;
    }
    else{
        QDateTime pickTime = getBlockTime(pickBlock);
        QDateTime originTime = getBlockTime(originBlock);
        if (pickTime == originTime){
           emit pickTurn (pickBlock);
           picksCounter++;
           emit originTurn (originBlock);
           originsCounter++;
        }
        else std::cout<<"unlinked block"<<std::endl;
    }

    blocksCounter++;
    if (blocksCounter == allBlocks.size())
        blocksTimer->stop();
    else
        blocksTimer->start(allBlocks[blocksCounter].second);
}

void SimulationPlanner::sendPick(QString pickBlock){
    std::cout<<"Sending: pick..."<<std::endl;
    QFile file(config->value("simulationpaths/picks").toString());
    if(picksCounter==0){
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            std::cerr << "Problem to find the picks file"<<std::endl;
        }
    }
    else{
        if(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
            std::cerr << "Problem to find the picks file"<<std::endl;
        }
    }
    QTextStream out(&file);
    out << endl << pickBlock;
    file.close();
}

void SimulationPlanner::sendOrigin(QString originBlock){
    std::cout<<"Sending: origin..."<<std::endl;
    QFile file(config->value("simulationpaths/origins").toString());
    if(originsCounter==0){
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            std::cerr << "Problem to find the origins file"<<std::endl;
        }
    }
    else{
        if(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
            std::cerr << "Problem to find the origins file"<<std::endl;
        }
    }
    QTextStream out(&file);
    out << endl << originBlock;
    file.close();

    //last origin sends event instead of let a timer do that (avoid delay)
    if(originsCounter == originsBlocks.size()-1){
        QTimer::singleShot (150,this,SLOT(sendEvent ()));
    }
}

void SimulationPlanner::sendEvent (){
    std::cout<<"Sending: event..."<<requiredEventName.toStdString()<<std::endl;

    eventTimer->stop();

    QString eventfilename = requiredEventDir
                            .absoluteFilePath(requiredEventName+".last.xml");
    QFile eventFile(eventfilename);
    if (!eventFile.open(QIODevice::ReadOnly)){
        std::cerr << "Problem to find the file: "+requiredEventName.toStdString()+".last.xml"<<std::endl;
        std::cerr << "No event information can be reached."<<std::endl;
    }
    QString xmlContent = eventFile.readAll();
    eventFile.close();

    QFile file(config->value("simulationpaths/event").toString());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        std::cerr << "Problem to find the event file"<<std::endl;
    }
    QTextStream out(&file);
    out << xmlContent;
    file.close();
}

QDateTime SimulationPlanner::getBlockTime(QString block){
    QDateTime datetime;
    QRegExp rxDateBlock("\\d+-\\d+-\\d+ \\d+:\\d+:\\d+.\\d");
    if(rxDateBlock.indexIn(block) != -1)
        datetime = QDateTime::fromString(rxDateBlock.cap(0),"yyyy-MM-dd hh:mm:ss.z");
    return datetime;
}
