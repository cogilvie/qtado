TEMPLATE = lib
TARGET = qtado

CONFIG += c++11 staticlib

DESTDIR = $$OUT_PWD/../../lib

QT += core network

SOURCES += tado.cpp

HEADERS += tado.h \
           libtado.h

DEFINES += BUILDING_TADO
