#ifndef ORIGIN_H
#define ORIGIN_H

#include <ctime>
#include <set>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QFile>
#include <QtXml/QDomDocument>
#include "station.h"

class Origin{

    public:
    Origin(const std::string& id = "", const QDate& date = QDate(), 
    	   const QTime& time = QTime(), long double latitude = 0,
           long double longitude = 0, double magnitude = 0,
           const std::set<Station>& stations = std::set<Station>());
    
    /**GETTERS AND SETTERS**/
    std::string getOriginID() const;
    void setOriginID(const std::string &value);

    QTime getOriginTime() const;
    void setOriginTime(const QTime &value);

    QDate getOriginDate() const;
    void setOriginDate(const QDate &value);

    long double getLatitude() const;
    void setLatitude(long double value);

    long double getLongitude() const;
    void setLongitude(long double value);

    double getMagnitude() const;
    void setMagnitude(double value);

    std::set<Station> getStations() const;
    void setStations(const std::set<Station> &value);

    QDateTime getSystemDateTime() const;
    void setSystemDateTime(const QDateTime &value);

    /**To String**/
    std::string originToString() const;
    std::string toStringXml()const;
    void fromQDomNode(const QDomNode& xml);
    static std::set<Origin> originsFromQDomDocument(const QDomDocument& xml);

    /**OPERATORS**/
    friend bool operator < (const Origin& origin1, const Origin& origin2);
    friend bool operator == (const Origin& origin1, const Origin& origin2);
    friend std::ostream& operator << (std::ostream& os, const Origin& origin);

private:
    std::string originID;
    QDate originDate;
    QTime originTime;
    long double latitude;
    long double longitude;
    double magnitude;
    std::set<Station> stations;

    QDateTime systemDateTime;

};

#endif
