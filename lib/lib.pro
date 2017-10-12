TEMPLATE = lib
TARGET = qtado

CONFIG += c++11

QT += core network

SOURCES += tado.cpp

HEADERS += tado.h \
           libtado.h

DEFINES += BUILDING_TADO
