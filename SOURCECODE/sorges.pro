#-------------------------------------------------
#
# SORGES PROJECT
# 
# Developers:
# Jose Felipe Sanchez Arenas
# Andres Jesus Sanchez Anillo
# Ruben de Jesus Moreno Leyva
# Adan Moises Toscano Lopez
#
#
#-------------------------------------------------
QT       += core gui
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sorges
TEMPLATE = app

SOURCES += main.cpp\
           widgets/mapwidget.cpp \
           widgets/origindatawidget.cpp \
           widgets/stationsdatawidget.cpp \
           classes/origin.cpp \
           classes/station.cpp \
           classes/dataprocessing.cpp \
           classes/simulationplanner.cpp

HEADERS  += widgets/mapwidget.h \
			widgets/origindatawidget.h \
			widgets/stationsdatawidget.h \
            classes/origin.h \
            classes/station.h \
            classes/dataprocessing.h \
            classes/simulationplanner.h \
            config/mapdefinition.h

FORMS    += ui/mapwidget.ui \
			ui/origindatawidget.ui \
			ui/stationsdatawidget.ui

RESOURCES += images/map.qrc \
			 images/logo.qrc
