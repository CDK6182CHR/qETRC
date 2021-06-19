QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src

SOURCES += \
    src/data/common/stationname.cpp \
    src/data/rail/forbid.cpp \
    src/data/rail/railinterval.cpp \
    src/data/rail/railstation.cpp \
    src/data/rail/railway.cpp \
    src/data/rail/ruler.cpp \
    src/data/rail/rulernode.cpp \
    src/data/rail/stationname.cpp \
    src/data/train/train.cpp \
    src/data/train/trainname.cpp \
    src/data/train/trainstation.cpp \
    src/data/train/traintype.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/data/common/direction.h \
    src/data/common/stationname.h \
    src/data/rail/forbid.h \
    src/data/rail/rail.h \
    src/data/rail/railinterval.h \
    src/data/rail/railintervaldata.hpp \
    src/data/rail/railintervalnode.hpp \
    src/data/rail/railstation.h \
    src/data/rail/railway.h \
    src/data/rail/ruler.h \
    src/data/rail/rulernode.h \
    src/data/rail/stationname.h \
    src/data/train/train.h \
    src/data/train/trainname.h \
    src/data/train/trainstation.h \
    src/data/train/traintype.h \
    src/mainwindow.h \
    src/util/qeexceptions.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
