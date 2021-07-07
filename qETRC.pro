QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../site-packages/SARibbon/SARibbonBar.pri)
include(../site-packages/ads/ads.pri)

INCLUDEPATH += src

SOURCES += \
    src/data/common/stationname.cpp \
    src/data/diagram/config.cpp \
    src/data/diagram/diagram.cpp \
    src/data/diagram/diagrampage.cpp \
    src/data/diagram/trainadapter.cpp \
    src/data/diagram/trainevents.cpp \
    src/data/diagram/trainline.cpp \
    src/data/rail/forbid.cpp \
    src/data/rail/railinterval.cpp \
    src/data/rail/railstation.cpp \
    src/data/rail/railway.cpp \
    src/data/rail/ruler.cpp \
    src/data/rail/rulernode.cpp \
    src/data/common/stationname.cpp \
    src/data/train/routing.cpp \
    src/data/train/train.cpp \
    src/data/train/traincollection.cpp \
    src/data/train/trainname.cpp \
    src/data/train/trainstation.cpp \
    src/data/train/traintype.cpp \
    src/dialogs/importtraindialog.cpp \
    src/editors/railstationwidget.cpp \
    src/editors/trainlistwidget.cpp \
    src/kernel/diagramwidget.cpp \
    src/kernel/trainitem.cpp \
    src/main.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/railcontext.cpp \
    src/mainwindow/traincontext.cpp \
    src/mainwindow/viewcategory.cpp \
    src/model/delegate/combodelegate.cpp \
    src/model/diagram/componentitems.cpp \
    src/model/diagram/diagramnavimodel.cpp \
    src/model/diagram/railtablemodel.cpp \
    src/model/rail/railstationmodel.cpp \
    src/model/train/trainlistmodel.cpp \
    src/navi/addpagedialog.cpp \
    src/navi/navitree.cpp \
    src/util/dialogadapter.cpp \
    src/util/qecontrolledtable.cpp \
    src/util/utilfunc.cpp \
    src/viewers/traineventdialog.cpp

HEADERS += \
    src/data/common/direction.h \
    src/data/common/stationname.h \
    src/data/diagram/config.h \
    src/data/diagram/diagram.h \
    src/data/diagram/diagrampage.h \
    src/data/diagram/trainadapter.h \
    src/data/diagram/trainevents.h \
    src/data/diagram/trainline.h \
    src/data/rail/forbid.h \
    src/data/rail/rail.h \
    src/data/rail/railinterval.h \
    src/data/rail/railintervaldata.hpp \
    src/data/rail/railintervalnode.hpp \
    src/data/rail/railstation.h \
    src/data/rail/railway.h \
    src/data/rail/ruler.h \
    src/data/rail/rulernode.h \
    src/data/train/routing.h \
    src/data/train/train.h \
    src/data/train/traincollection.h \
    src/data/train/trainname.h \
    src/data/train/trainstation.h \
    src/data/train/traintype.h \
    src/dialogs/importtraindialog.h \
    src/editors/railstationwidget.h \
    src/editors/trainlistwidget.h \
    src/kernel/diagramwidget.h \
    src/kernel/trainitem.h \
    src/mainwindow/mainwindow.h \
    src/mainwindow/railcontext.h \
    src/mainwindow/traincontext.h \
    src/mainwindow/version.h \
    src/mainwindow/viewcategory.h \
    src/model/delegate/combodelegate.h \
    src/model/delegate/qedelegate.h \
    src/model/diagram/componentitems.h \
    src/model/diagram/diagramnavimodel.h \
    src/model/diagram/railtablemodel.h \
    src/model/rail/railstationmodel.h \
    src/model/train/trainlistmodel.h \
    src/navi/addpagedialog.h \
    src/navi/navitree.h \
    src/util/buttongroup.hpp \
    src/util/dialogadapter.h \
    src/util/qecontrolledtable.h \
    src/util/qeexceptions.h \
    src/util/utilfunc.h \
    src/viewers/traineventdialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_CXXFLAGS += /utf-8

RESOURCES += \
    rsc/resource.qrc

DISTFILES += \
    rsc/icons/trainline.png
