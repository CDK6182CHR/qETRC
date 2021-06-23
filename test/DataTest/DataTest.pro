QT += testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

INCLUDEPATH += ../../src

SOURCES +=  tst_railtest.cpp \
    ../../src/data/rail/railstation.cpp \
    ../../src/data/common/stationname.cpp \
    ../../src/data/rail/railway.cpp \
    ../../src/data/rail/railinterval.cpp \
    ../../src/data/rail/rulernode.cpp \
    ../../src/data/rail/ruler.cpp \
    ../../src/data/rail/forbid.cpp \
    ../../src/data/train/trainname.cpp \
    ../../src/data/train/trainstation.cpp \
    ../../src/data/train/train.cpp \
    ../../src/data/diagram/trainadapter.cpp \
    ../../src/data/diagram/trainline.cpp \


QMAKE_CXXFLAGS += /utf-8
