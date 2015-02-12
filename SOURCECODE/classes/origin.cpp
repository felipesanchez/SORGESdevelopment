#include<iostream>
#include<sstream>
#include "origin.h"

Origin::Origin(const std::string& id, const QDate& date, const QTime& time, 
			   long double latitude,long double longitude, 
			   double magnitude, const std::set<Station>& stations):
               originID(id),
               originDate(date),
               originTime(time),
   			   latitude(latitude),
   			   longitude(longitude),
   			   magnitude(magnitude),
               stations(stations),
               systemDateTime(QDateTime::currentDateTime())
               {}


/**GETTERS AND SETTERS**/

std::string Origin::getOriginID() const
{
    return originID;
}
void Origin::setOriginID(const std::string &value)
{
    originID = value;
}

std::set<Station> Origin::getStations() const
{
    return stations;
}
void Origin::setStations(const std::set<Station> &value)
{
    stations = value;
}

double Origin::getMagnitude() const
{
    return magnitude;
}
void Origin::setMagnitude(double value)
{
    magnitude = value;
}

long double Origin::getLongitude() const
{
    return longitude;
}
void Origin::setLongitude(long double value)
{
    longitude = value;
}

long double Origin::getLatitude() const
{
    return latitude;
}
void Origin::setLatitude(long double value)
{
    latitude = value;
}

QTime Origin::getOriginTime() const
{
    return originTime;
}
void Origin::setOriginTime(const QTime &value)
{
    originTime = value;
}

QDate Origin::getOriginDate() const
{
    return originDate;
}
void Origin::setOriginDate(const QDate &value)
{
    originDate = value;
}

QDateTime Origin::getSystemDateTime() const
{
    return systemDateTime;
}
void Origin::setSystemDateTime(const QDateTime &value)
{
    systemDateTime = value;
}


/**OPERATORS**/

bool operator < (const Origin& origin1, const Origin& origin2){
    QDateTime origin1DateTime, origin2DateTime;
    origin1DateTime.setDate(origin1.getOriginDate());
    origin2DateTime.setDate(origin2.getOriginDate());
    origin1DateTime.setTime(origin1.getOriginTime());
    origin2DateTime.setTime(origin2.getOriginTime());
    return origin1DateTime < origin2DateTime;
}

bool operator == (const Origin& origin1, const Origin& origin2){		
	return origin1.originID == origin2.originID;		
}

std::ostream& operator << (std::ostream& os, const Origin& origin){
    os << "<Origin>\n";
    os << "\t<OriginID>" << origin.getOriginID().c_str() << "</OriginID>\n";
    os << "\t<OriginDate>"
       << origin.getOriginDate().toString("yyyy-MM-dd").toStdString().c_str()
       << "</OriginDate>\n";
    os << "\t<OriginTime>"
       << origin.getOriginTime().toString("hh:mm:ss.z").toStdString().c_str()
       << "</OriginTime>\n";
    os << "\t<OriginSystemDateTime>"
       << origin.getSystemDateTime().toString("yyyy-MM-dd hh:mm:ss.z").toStdString().c_str()
       << "</OriginSystemDateTime>\n";
    os << "\t<OriginMagnitude>" << origin.getMagnitude()
       << "</OriginMagnitude>\n";
    os << "\t<OriginLatitude>" << (double)origin.getLatitude()
       << "</OriginLatitude>\n";
    os << "\t<OriginLongitude>" << (double)origin.getLongitude()
       << "</OriginLongitude>\n";
    os << "\t<AssociatedStations>\n";
    os << "\t\t<StationsNumber>"<< origin.stations.size()
       << "</StationsNumber>\n";
    for(std::set<Station>::iterator it=origin.stations.begin();
                                    it!=origin.stations.end(); ++it)
        os << *it ;
    os << "\t</AssociatedStations>\n";
    os << "</Origin>\n";
    return os;
}

void Origin::fromQDomNode(const QDomNode& originNode){
    originID = originNode.firstChildElement("OriginID").text().toStdString();
    originDate = QDate::fromString(originNode.firstChildElement("OriginDate").text(),"yyyy-MM-dd");
    originTime = QTime::fromString(originNode.firstChildElement("OriginTime").text(),"hh:mm:ss.zzz");
    latitude = originNode.firstChildElement("OriginLatitude").text().toDouble();
    longitude = originNode.firstChildElement("OriginLongitude").text().toDouble();
    magnitude = originNode.firstChildElement("OriginMagnitude").text().toDouble();
    stations = Station::stationsFromQDomElement(originNode.firstChildElement("AssociatedStations"));
}

std::set<Origin> Origin::originsFromQDomDocument(const QDomDocument& xml){
    std::set<Origin> myOrigins;
    QDomNodeList origins = xml.elementsByTagName("Origin");
    for (int i = 0; i < origins.size(); i++) {
        Origin myOrigin;
        myOrigin.fromQDomNode(origins.item(i));
        myOrigins.insert(myOrigin);
    }
    return myOrigins;
}

/**To String method*/
/**uses the output operator format to create a string**/
std::string Origin::originToString() const{
    std::ostringstream convert;

    convert << "Origin ID: " << this->originID << "\n";
    convert << "Timestamp: " << this->originDate.toString().toStdString()
                             << this->originDate.toString().toStdString()<<"\n";
    convert << "Latitude: "<< this->latitude
            << "\nLongitude: " << this->longitude << "\n";
    if (this->magnitude != 0) convert << "Magnitude: " << this->magnitude << "\n";
    if (!(this->stations.empty())){
        convert << "Related Stations:\n";
        foreach(Station st,this->stations){
            convert << st.getStationID() << "\n";
        }
    }
    return convert.str();
}

std::string Origin::toStringXml ()const{
    std::string os;
    os += "<Origin>\n";
    os +=  "\t<OriginID>" + getOriginID()+ "</OriginID>\n";
    os += "\t<OriginDate>" + getOriginDate().toString("yyyy-MM-dd").toStdString() + "</OriginDate>\n";
    os += "\t<OriginTime>" + getOriginTime().toString("hh:mm:ss.z").toStdString() + "</OriginTime>\n";
    os += "\t<OriginSystemDateTime>"
            + getSystemDateTime().toString("yyyy-MM-ddThh:mm:ss.z").toStdString()
            + "</OriginSystemDateTime>\n";
    os += "\t<OriginMagnitude>" +  QString::number((double)getMagnitude()).toStdString() +  "</OriginMagnitude>\n";
    os += "\t<OriginLatitude>" +  QString::number((double)getLatitude()).toStdString() + "</OriginLatitude>\n";
    os += "\t<OriginLongitude>" + QString::number((double)getLongitude()).toStdString() +  "</OriginLongitude>\n";
    os += "\t<AssociatedStations>\n";
    os += "\t\t<StationsNumber>"+ QString::number(stations.size()).toStdString() + "</StationsNumber>\n";
    for(std::set<Station>::iterator it=stations.begin(); it!=stations.end(); ++it){
            os += (*it).toStringXml("\t\t");
    }
    os += "\t</AssociatedStations>\n";
    os += "</Origin>\n";
    return os;
}
