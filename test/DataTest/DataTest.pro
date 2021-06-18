QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

INCLUDEPATH += ../../src/data

SOURCES +=  tst_railtest.cpp \
    ../../src/data/railway.cpp \
    ../../src/data/railstation.cpp \
    ../../src/data/railinterval.cpp \
    ../../src/data/stationname.cpp \
    ../../src/data/ruler.cpp \
    ../../src/data/rulernode.cpp \

HEADERS += railway.h

QMAKE_CXXFLAGS += /utf-8
