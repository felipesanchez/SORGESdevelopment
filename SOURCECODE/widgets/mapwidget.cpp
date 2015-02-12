#include <math.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <QTimer>
#include <QGraphicsItem>
#include <algorithm>
#include <QToolTip>
#include <QWhatsThis>
#include <QDateTime>
#include "mapwidget.h"
#include "../config/mapdefinition.h"
#include "ui_mapwidget.h"

MapWidget::MapWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::MapWidget), mapScene(new QGraphicsScene(this)),
                                            circlesTimer(new QTimer(this))
{    
    //Setup for the user interface.
    ui->setupUi(this);

    //Image load from resource
    mapImage.load(MAP_IMAGE_PATH);

    //add image to scene
    QPixmap mapPixmap = QPixmap::fromImage(mapImage);
    QGraphicsItem *mapItem = mapScene.addPixmap(mapPixmap);
    //and set the name of the item
    mapItem->setData (0,"map");

    //and fit rectangle to image limits
    mapScene.setSceneRect(mapPixmap.rect());

    //User interface contains the GraphicsView named mapView (see the ui form)
    ui->mapView->setScene(&mapScene);

    //connect the timeout of the timer to the event to paint the concentric circles
    connect(circlesTimer, SIGNAL(timeout()), this, SLOT(paintCircles()));

    //showing item info when selected
    connect(&mapScene, SIGNAL(selectionChanged()),this,SLOT(showInformation()));

}

MapWidget::~MapWidget()
{
    delete ui;
}

void MapWidget::showInformation()
{
    foreach(QGraphicsItem * item,mapScene.selectedItems ())
    {
        if (item->data(1).toString() == "station"){
            Station st(*(stations
                         .find(Station(item->data (0).toString().toStdString()))));
            long double x,y;
            coordinatesToPixels(x,y,st.getLatitude(),st.getLongitude());
            QPoint pos(this->pos().x()+x-80,this->pos().y()+y-50);
            std::string s = "<span style=\"color:black;\"> Station ID: "
                            +st.getStationID ()
                            +"<br>Net ID: "+st.getNetworkID ()
                            +"<br>Latitude: "
                            +QString::number((double)st.getLatitude()).toStdString ()
                            +"<br>Longitude: "
                            +QString::number((double)st.getLongitude ()).toStdString ()
                            +"<br>On-Site Alert: ";
            if(st.getColor()>-1) s+=QString::number(st.getColor ()).toStdString ();
                            s+="</span>";
            QWhatsThis::showText(pos,s.c_str(),ui->mapView);
        }
        else if (item->data(0).toString() == "epicenter"){
            long double x,y;
            coordinatesToPixels(x,y,
                                currentOrigin.getLatitude(),
                                currentOrigin.getLongitude());
            QPoint pos(this->pos().x()+x-100,this->pos().y()+y-110);
            std::string s = "<span style=\"color:black;\"> Origin ID:<br>"
                            +currentOrigin.getOriginID()
                            +"<br>Timestamp: ";
                        s+= currentOrigin.getOriginDate()
                                .toString("dd-MM-yyyy")
                                .toStdString ()+" ";
                        s+= currentOrigin.getOriginTime()
                                .toString("hh:mm:ss.z")
                                .toStdString ();
                        s+="<br>Latitude: "
                            +QString::number((double)
                                             currentOrigin.getLatitude()).toStdString()
                            +"<br>Longitude: "
                            +QString::number((double)
                                             currentOrigin.getLongitude()).toStdString ();
            if(currentOrigin.getMagnitude() > 0)
                            s+=QString::number(currentOrigin.getMagnitude()).toStdString ();
                            s+="</span>";
            QWhatsThis::showText(pos,s.c_str(),ui->mapView);
        }
    }
}


/**CONVERTER FUNCTIONS FROM COORDINATES TO PIXELS**/

/**Converter from degrees-minutes-seconds to decimal degrees*/
long double MapWidget::convertToDecimalDegrees(long double degrees,
                                               long double minutes,
                                               long double seconds)
{

    return degrees + (long double)(minutes/60) + (long double)(seconds/3600);
}

/*
 * Interpolation function which transform received coordinates to map image pixels
 * (version receiving degrees-minutes-seconds
*/
void MapWidget::coordinatesToPixels(long double &pixelX,long double &pixelY,
                                    long double degreesTargetLat,
                                    long double minutesTargetLat,
                                    long double secondsTargetLat,
                                    long double degreesTargetLon,
                                    long double minutesTargetLon,
                                    long double secondsTargetLon)
{
    long double minLon = MAP_MIN_LONGITUDE;
    long double minLat = MAP_MIN_LATITUDE;
    long double maxLon = MAP_MAX_LONGITUDE;
    long double maxLat = MAP_MAX_LATITUDE;
    long double targetLon = convertToDecimalDegrees(degreesTargetLon,
                                                    minutesTargetLon,
                                                    secondsTargetLon);
    long double targetLat = convertToDecimalDegrees(degreesTargetLat,
                                                    minutesTargetLat,
                                                    secondsTargetLat);
    long double maxXpixel = mapScene.width();
    long double maxYpixel = mapScene.height();
    long double minXpixel = 0;
    long double minYpixel = 0;

    pixelX = ((targetLon - minLon) / (maxLon - minLon)) * (maxXpixel - minXpixel);
    pixelY = ((targetLat - minLat) / (maxLat - minLat)) * (maxYpixel - minYpixel);
}

/*
 * Interpolation function which transform received coordinates to map image pixels
 * (version receiving decimal degrees
*/
void MapWidget::coordinatesToPixels(long double &pixelX,long double &pixelY,
                                    long double targetLat,long double targetLon)
{
    long double minLon = MAP_MIN_LONGITUDE;
    long double minLat = MAP_MIN_LATITUDE;
    long double maxLon = MAP_MAX_LONGITUDE;
    long double maxLat = MAP_MAX_LATITUDE;
    long double maxXpixel = mapScene.width();
    long double maxYpixel = mapScene.height();
    long double minXpixel = 0;
    long double minYpixel = 0;

    pixelX = ((targetLon - minLon) / (maxLon - minLon)) * (maxXpixel - minXpixel);
    pixelY = ((targetLat - minLat) / (maxLat - minLat)) * (maxYpixel - minYpixel);
}



/**STATION FUNCTIONS**/

void MapWidget::paintStations(const std::set<Station>& stationsList)
{
    this->stations.clear();
    clearStations();
    this->stations = stationsList;
    for(std::set<Station>::iterator it=stations.begin(); it!=stations.end(); ++it)
        drawStation(*it);
}

void MapWidget::clearStations()
{
    foreach(QGraphicsItem * item, mapScene.items()){
        if (item->data(1).toString () == "station"){
            mapScene.removeItem(item);
        }
    }
}

void MapWidget::clearStation(const std::string &stationID)
{
    foreach(QGraphicsItem * item, mapScene.items()){
        if (item->data(0).toString () == stationID.c_str()){
            mapScene.removeItem(item);
            break;
        }
    }
}

void MapWidget::drawStation(const Station& station)
{
    long double coordX, coordY;
    coordinatesToPixels(coordX,coordY,station.getLatitude(),station.getLongitude());

    QPolygonF triangle;
    triangle.append(QPoint(coordX,coordY));
    triangle.append(QPoint(coordX+STATION_SIZE_X,coordY-STATION_SIZE_Y));
    triangle.append(QPoint(coordX-STATION_SIZE_X,coordY-STATION_SIZE_Y));

    //in case there is a station already there, we delete it
    clearStation(station.getStationID());

    //and we add the new one
    const char * color = station.getCurrentOnSiteAlert();
    QGraphicsItem *stationItem = mapScene.addPolygon(triangle,QPen(),QBrush(color));
    stationItem->setData(0,station.getStationID().c_str());
    stationItem->setData(1,"station");
    stationItem->setFlag (QGraphicsItem::ItemIsSelectable, true);

}

void MapWidget::changeStationsColors(const std::set<Station> &changedStations)
{
    for(std::set<Station>::iterator it=changedStations.begin();
                                    it!=changedStations.end(); ++it)
    {
        stations.erase(stations.find(*it));
        stations.insert(*it);
        drawStation(*it);
    }
}



/**ORIGIN FUNCTIONS**/

void MapWidget::clearOrigin()
{
    foreach(QGraphicsItem * item, mapScene.items()){
        QString itemName = item->data(0).toString ();
        if ( (itemName == "epicenter") || (itemName == "circle")){
            mapScene.removeItem(item);
        }
    }
}

void MapWidget::clearCirclesOrigin()
{
    foreach(QGraphicsItem * item, mapScene.items()){
        QString itemName = item->data(0).toString ();
        if (itemName == "circle"){
            mapScene.removeItem(item);
        }
    }
}

void MapWidget::updateEventStations()
{
    std::set<Station> relatedStations = this->currentOrigin.getStations();

    //update stations related
    changeStationsColors(relatedStations);

    //turn off to black the non-related
    foreach (Station currentStation, this->stations) {
        if (relatedStations.find(currentStation) == relatedStations.end()){
            currentStation.setColor(-1);
            drawStation(currentStation);
            stations.erase(stations.find(currentStation));
            stations.insert(currentStation);
        }
    }
}

void MapWidget::paintOrigin(const Origin &origin){

    //if timer of circles painting is on, stop it
    if (circlesTimer->isActive ())
        circlesTimer->stop();

    //if there is a current origin, we erase it
    clearOrigin();


    //anyways, the currentOrigin will be the new one
    this->currentOrigin = origin;

    //check if is the first origin in the event process
    //
    if (this->firstOrigin.getOriginID() ==""){
        this->firstOrigin = origin;
        QDateTime dateTimeOrigin(origin.getOriginDate(),origin.getOriginTime());
        QDateTime currentDateTimeSystem = QDateTime::currentDateTime();
        this->firstOrigin
              .setSystemDateTime(currentDateTimeSystem.
                                 addMSecs(-dateTimeOrigin
                                          .msecsTo(this->firstOrigin
                                                   .getSystemDateTime())));
    }

    this->currentOrigin.setSystemDateTime(firstOrigin.getSystemDateTime());

    long double coordX, coordY;
    long double radius = calculateRadius();

    coordinatesToPixels(coordX,coordY,currentOrigin.getLatitude(),
                                      currentOrigin.getLongitude());
    QPoint center(coordX, coordY);

    //First circle
    QRect rect(0,0,2*radius,2*radius);
    rect.moveCenter(center);
    QRadialGradient radialColor(center,RADIUS_120KM_PIXEL,center);

    // Define the colour interpolation in 120 km (0 = 0km, and 1 = 120km).
    radialColor.setColorAt(0,QColor(EPICENTER_GRADIENT_0_30KM));
    radialColor.setColorAt(0.1428,QColor(EPICENTER_GRADIENT_0_30KM));
    radialColor.setColorAt(0.2856,QColor(EPICENTER_GRADIENT_30_60KM));
    radialColor.setColorAt(0.5714,QColor(EPICENTER_GRADIENT_60_120KM));
    radialColor.setColorAt(1,QColor(EPICENTER_GRADIENT_120KM));


    QGraphicsItem *circleItem = mapScene.addEllipse(rect,
                                                    QPen(),
                                                    QBrush(radialColor));
     circleItem->setData(0,"circle");

    //Epicenter mark (to be on top of the first circle)
    QRect rect2(0,0,3*RADIUS_EPICENTER,3*RADIUS_EPICENTER);
    rect2.moveCenter(center);
    QGraphicsItem *epicenterItem = mapScene.addEllipse(rect2,
                                                       QPen(),
                                                       QBrush(QColor(R_EPICENTER,
                                                                     G_EPICENTER,
                                                                     B_EPICENTER,
                                                                     T_EPICENTER)));
    epicenterItem->setData(0,"epicenter");
    epicenterItem->setFlag (QGraphicsItem::ItemIsSelectable, true);

    //Set the timer (each second) for the concentric circles
    circlesTimer->start(1000);

    //only the last origin (event) will have related stations
    //update only its stations and restart the logical process of the event
    //by setting "firstOrigin" to default object
    //thus the following origin in the system will be the first of the next event
    if (!currentOrigin.getStations().empty()){
        updateEventStations();

        this->firstOrigin = Origin();
    }

}



/**CONCENTRIC CIRCLES**/

/**
 * Function for calculating the expansion circle radius every 5 seconds
 */
long double MapWidget::calculateRadius()
{
    long double radius;
    long int difMseconds;
    QDateTime timeinfo = QDateTime::currentDateTime();

    difMseconds = currentOrigin.getSystemDateTime().msecsTo(timeinfo);

    // getting the radius in meters.
    radius = (difMseconds/1000) * PROPAGATION_SPEED;

    // Calculate the number of pixels to "Radius meters".
    //meanMetres is the medium between the referenced distances in metres of
    //the X-axis at the top and and the botton of the image, which are different
    //due to the shape of heart and its projection
    long double meanMetres = (MAP_METRES_LONGITUDE_BOTTOM+MAP_METRES_LONGITUDE_TOP)/2;
    return radius/meanMetres*mapScene.width();

}


/**
 * Painter method for the concentric circles of the origins
 * representing the seismic wave expansion.
 * Authomatically called by timer each 5s when a origin is set
 */
void MapWidget::paintCircles(){

    //center of the circles: origin epicenter
    long double x,y;
    coordinatesToPixels(x,y,currentOrigin.getLatitude(),currentOrigin.getLongitude());
    QPoint center(x,y);

    //radius of the circle at current time
    long double radius = calculateRadius();

    //if the radius is out of map bounds (hypotenuse), no need to paint anymore
    long double hypotenuse = sqrt(mapScene.width()*mapScene.width()
                             + mapScene.height()*mapScene.height());

    if (radius <= hypotenuse){
        //circle will be fit into a rectangle whose center is moved to epicenter
        QRect rect(0,0,2*radius,2*radius);
        rect.moveCenter(center);
        clearOrigin();
        QRadialGradient radialColor(center,RADIUS_120KM_PIXEL,center);

        // Define the colour interpolation in 120 km (0 = 0km, and 1 = 120km).
        radialColor.setColorAt(0,QColor(EPICENTER_GRADIENT_0_30KM));
        radialColor.setColorAt(0.1428,QColor(EPICENTER_GRADIENT_0_30KM));
        radialColor.setColorAt(0.2856,QColor(EPICENTER_GRADIENT_30_60KM));
        radialColor.setColorAt(0.5714,QColor(EPICENTER_GRADIENT_60_120KM));
        radialColor.setColorAt(1,QColor(EPICENTER_GRADIENT_120KM));


        QGraphicsItem *circleItem = mapScene.addEllipse(rect,
                                                        QPen(),
                                                        QBrush(radialColor));
        circleItem->setData(0,"circle");

        //Epicenter mark (to be on top of the first circle)
        QRect rect2(0,0,3*RADIUS_EPICENTER,3*RADIUS_EPICENTER);
        rect2.moveCenter(center);
        QGraphicsItem *epicenterItem = mapScene.addEllipse(rect2,
                                                           QPen(),
                                                           QBrush(QColor(R_EPICENTER,
                                                                         G_EPICENTER,
                                                                         B_EPICENTER,
                                                                         T_EPICENTER)));
        epicenterItem->setData(0,"epicenter");
        epicenterItem->setFlag (QGraphicsItem::ItemIsSelectable, true);

    }
    else {
        circlesTimer->stop();
        //if the current origin/event stays on screem more than 5 minutes, delete it
        QTimer::singleShot(300000,this,SLOT(clearScreenTimeout()));
    }

}

void MapWidget::clearScreenTimeout(){
    if (firstOrigin.getOriginID() == ""){
        clearOrigin ();
        foreach(Station st,stations){
            st.setColor(-1);
            drawStation(st);
        }
    }
}


/***************************************brief tests**********************************/


/*
 * Private function to test the pixel-coordinates conversion accuracy
 */
void MapWidget::testPixelPrecision(){

    std::cout << convertToDecimalDegrees(38,0,0)<< std::endl;
    std::cout << convertToDecimalDegrees(-14,0,0)<< std::endl;
    std::cout << convertToDecimalDegrees(34,0,0)<< std::endl;
    std::cout << convertToDecimalDegrees(-3,0,0)<< std::endl;

    //cabo san vicente 36°59'39.61" (36.994336) y -8°-56'-9.60" (-8.936)
    std::cout << convertToDecimalDegrees(36,59,39.61)<< std::endl;
    std::cout << convertToDecimalDegrees(-8,-56,-9.6)<< std::endl;
    long double x,y;
    coordinatesToPixels(x,y,36.994336,-8.936);
    std::cout << x << ' ' << y << std::endl;
    mapScene.addLine (x,y,0,0);

    //punta gibraltar 36° 6'34.01" (36.109447) -5°-20'-43.59" (-5.345442)
    std::cout << convertToDecimalDegrees(36,6,34.01)<< std::endl;
    std::cout << convertToDecimalDegrees(-5,-20,-43.59)<< std::endl;
    long double x2,y2;
    coordinatesToPixels(x2,y2,36.109447,-5.345442);
    std::cout << x2 << ' ' << y2 << std::endl;
    mapScene.addLine (x2,y2,mapScene.width (),mapScene.height ());

    //Punta san felipe Cádiz 36°32'16.12"  -6°-18'-1.20"
    std::cout << convertToDecimalDegrees(36,32,16.12)<< std::endl;
    std::cout << convertToDecimalDegrees(-6,-18,-1.20)<< std::endl;
    long double x3,y3;
    coordinatesToPixels(x3,y3,36,32,16.12,-6,-18,-1.20);
    std::cout << x3 << ' ' << y3 << std::endl;
    mapScene.addLine (x3,y3,0,mapScene.height ());

    //Costa de Lisboa (referencia Plaza del Comercio) 38°42'22.00" -9°-8'-10"
    std::cout << convertToDecimalDegrees(38,42,22)<< std::endl;
    std::cout << convertToDecimalDegrees(-9,-8,-10)<< std::endl;
    long double x4,y4;
    coordinatesToPixels(x4,y4,38,42,22,-9,-8,-10);
    std::cout << x4 << ' ' << y4 << std::endl;
    mapScene.addLine (x4,y4,mapScene.width(),0);

    //Cabo de Peniche (por encima de Lisboa) 39°21'30.87" -9°-24'-24.36"
    std::cout << convertToDecimalDegrees(39,21,30.87)<< std::endl;
    std::cout << convertToDecimalDegrees(-9,-24,-24.36)<< std::endl;
    long double x5,y5;
    coordinatesToPixels(x5,y5,39,21,30.87,-9,-24,-24.36);
    std::cout << x5 << ' ' << y5 << std::endl;
    mapScene.addLine (x5,y5,mapScene.width()/2,0);
}

/*
 * Private function just to test placing a origin in the map
 */
void MapWidget::testOrigen(){
    std::set<Station> mystations;
    mystations.insert(
                Station("0x0000", "0x0001", 37.00204023875479, -10.2456402219765,0));
    mystations.insert(
                Station("0x0001", "0x0001", 34.00204023875479, -6.2456402219765,1));
    mystations.insert(
                Station("0x0002", "0x0002", 36.00204023875479, -4.2456402219765,2));
    mystations.insert(
                Station("0x0003", "0x0003", 35.00204023875479, -8.2456402219765,3));

    // getting time from system
    //it simulates 9 seconds of delay:
    QTime timeinfo = QTime(QTime::currentTime().addMSecs(-999));
    QDate dateinfo = QDate::currentDate();


    //Punta san felipe Cádiz 36°32'16.12"  -6°-18'-1.20"
    Origin myOrigin("0x0001b",dateinfo,timeinfo, 
    				convertToDecimalDegrees(36,32,16.12),
                    convertToDecimalDegrees(-6,-18,-1.20), 
                    3.54, mystations);
    paintOrigin(myOrigin);
}

/*
 * Private function just to test the situation of stations in the map
 */
void MapWidget::testStation(){
    std::set<Station> mystations;
    mystations.insert(Station("0x0000", "0x0001",
                              convertToDecimalDegrees(36,59,39.61),
                              convertToDecimalDegrees(-8,-56,-9.6), -1));
    mystations.insert(Station("0x0001", "0x0002",
                              convertToDecimalDegrees(36,32,16.12),
                              convertToDecimalDegrees(-6,-18,-1.20), 0));
    mystations.insert(Station("0x0002", "0x0003",
                              convertToDecimalDegrees(36,32,16.12),
                              convertToDecimalDegrees(-7,-18,-1.20), 1));
    mystations.insert(Station("0x0003", "0x0004",
                              convertToDecimalDegrees(36,40,17.19),
                              convertToDecimalDegrees(-4,-13,-1.10), 2));
    mystations.insert(Station("0x0004", "0x0005",
                              convertToDecimalDegrees(35,36,12.12),
                              convertToDecimalDegrees(-7,-4,-1.22), 3));
    paintStations(mystations);

    //changing one colour:
    std::set<Station> testChangeStationsColor;
    testChangeStationsColor.insert(Station("0x0000", "0x0001",
                                           convertToDecimalDegrees(36,59,39.61),
                                           convertToDecimalDegrees(-8,-56,-9.6), 0));
    changeStationsColors(testChangeStationsColor);
}

/***************************************tests**********************************/
