#ifndef DATAPROCESSING_H
#define DATAPROCESSING_H

#include <QObject>
#include <QDateTime>
#include <QRegExp>
#include <iostream>
#include <set>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QtXml/QDomDocument>
#include <QTextStream>
#include <QPair>
#include "origin.h"
#include "station.h"
#include "simulationplanner.h"

class DataProcessing : public QObject
{
    Q_OBJECT

public:
    DataProcessing(bool simulationMode, QObject* parent = 0);
    void dumpOriginXml();
    void dumpStationXml();

signals:
    void stationsLoaded(std::set<Station> stations);
    void stationColorReceived(std::set<Station> changedStation);
    void originReceived(Origin origin);
    void eventReceived(Origin origin);

public slots:
    void init();
    void initSimulation(QDateTime simulationDateTime);

private slots:
    void fileChangedSlot(QString path);
    void processStationsFromFile(const QString &namefile);
    std::set<Station> processColorStationsFromFile(const QString &namefile);
    void processOriginFromFileLog(const QString &namefile);
    void processOriginFromFileXml(const QString &namefile);


private:
    bool simulationMode;
    Origin origin;
    std::set<Station> stations;
    QFileSystemWatcher watcher;
    QSettings *config;
    QDateTime bootDateTime;
    QDateTime simulationLastDateTime;
    QDateTime simulationFirstDateTime;
    qint64 simulationDuration;
    SimulationPlanner* simulationPlanner;


    std::vector<QStringList> findParameterStations(const QString &stationsString);

    QString findParameterOriginID(const QString &originString);
    QString findParameterOriginDate(const QString &originString);
    QString findParameterOriginTime(const QString &originString);
    QString findParameterOriginLatitude(const QString &originString);
    QString findParameterOriginLongitude(const QString &originString);
    QString findParameterOriginSystemDateTime(const QString &originString);

    QDateTime getDateTimeFromEvent(QDir eventFiles,QString eventName);
    QDateTime getDateTimeFirstPick(QDir eventFiles,QString eventName);
    QDateTime getLastDateTimeFromEvent(QDir eventFiles,QString eventName);
    QString getBlockPick(const QDateTime& firstdatetime, const QDateTime& lastdatetime);
    QString getBlockOrigin(const QDateTime& firstdatetime, const QDateTime& lastdatetime);
    int getPositionBegin(const QDateTime& firstdatetime, const QString& namefile);
    int getPositionEnd(const QDateTime& lastdatetime, const QString& namefile);
    std::set<QPair<QStringList,QDateTime> > getDateTimeBlocks(const QString& block);
    QList<QPair<QString,int> > getSecuence(const std::set<QPair<QStringList, QDateTime> > &blocks);

};

#endif // DATAPROCESSING_H
