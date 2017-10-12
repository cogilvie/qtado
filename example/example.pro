QT += gui core network

CONFIG += c++11 console
CONFIG -= app_bundle

INCLUDEPATH += ../lib

LIBS += -L../lib
LIBS += -lqtado

SOURCES += main.cpp

