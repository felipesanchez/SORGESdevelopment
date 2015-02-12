#ifndef STATION_H
#define STATION_H
#include <string>
#include <map>
#include <QTextStream>
#include <QDomDocument>
#include <set>
 
class Station{
	
	public:
    Station(const std::string& id ="", const std::string& netId ="", 
    		long double latitude = 0, long double longitude = 0, int color = -1);

   	/**ON-SITE ALERT
	* Color Scale.
	* Static attribute for all the station objects
	* Map with alert code and its color in hexadecimal form.
	*/
    static std::map<int,const char*> onSiteAlert;
	const char* getCurrentOnSiteAlert() const;
	
    /**GETTERS AND SETTERS**/
    std::string getStationID() const;
    void setStationID(const std::string &value);

    std::string getNetworkID() const;
    void setNetworkID(const std::string &value);

    long double getLatitude() const;
    void setLatitude(long double value);

    long double getLongitude() const;
    void setLongitude(long double value);

    int getColor() const;
    void setColor(int value);
    const char* getCurrentOnSiteAlert();

    /**To String**/
    std::string stationToString() const;
    std::string toStringXml(const std::string& tab = "")const;
    static std::set<Station> stationsFromQDomElement(const QDomElement &xml);
    static std::string stationsToStringXml(const std::set<Station> &xml, const std::string& tab="");
    void fromQDomNode(const QDomNode& stationNode);

    /**OPERATORS**/
    friend bool operator < (const Station& station1, const Station& station2);
    friend bool operator == (const Station& station1, const Station& station2);
    friend std::ostream& operator << (std::ostream& os, const Station& station);

private:
    std::string stationID;
    std::string networkID;
    long double latitude;
    long double longitude;
	int color;
};

#endif
