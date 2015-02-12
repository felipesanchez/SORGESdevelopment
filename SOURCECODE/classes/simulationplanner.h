#ifndef SIMULATIONPLANNER_H
#define SIMULATIONPLANNER_H

#include <QObject>
#include <QList>
#include <set>
#include <QPair>
#include <QString>
#include <QDir>
#include <QTimer>
#include <QSettings>

class SimulationPlanner : public QObject
{
    Q_OBJECT
public:
    SimulationPlanner();
    SimulationPlanner(QString event, QDir eventDir, QList<QPair<QString, int> > all,
                      QList<QPair<QString,int> > picks,QList<QPair<QString,int> > origins,
                      int duration, QObject *parent = 0);

signals:
    void pickTurn(QString pickBlock);
    void originTurn(QString originBlock);

public slots:
    void dispatch();
    void sendPick(QString pickBlock);
    void sendOrigin(QString originBlock);
    void sendEvent();

private:
    QString requiredEventName;
    QDir requiredEventDir;
    QList<QPair<QString,int> > allBlocks;
    QList<QPair<QString,int> > picksBlocks;
    QList<QPair<QString,int> > originsBlocks;
    int simulationDuration;
    QTimer* eventTimer;
    QTimer* blocksTimer;
    int blocksCounter;
    int originsCounter;
    int picksCounter;
    QSettings *config;

    QDateTime getBlockTime(QString block);

};

#endif // SIMULATIONPLANNER_H

