QT       += core gui
qtHaveModule(printsupport): QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../site-packages/SARibbon/SARibbonBar.pri)
include(../site-packages/ads/ads.pri)

INCLUDEPATH += src

SOURCES += \
    src/data/common/qesystem.cpp \
    src/data/common/stationname.cpp \
    src/data/diagram/config.cpp \
    src/data/diagram/diagram.cpp \
    src/data/diagram/diagrampage.cpp \
    src/data/diagram/trainadapter.cpp \
    src/data/diagram/trainevents.cpp \
    src/data/diagram/traingap.cpp \
    src/data/diagram/trainline.cpp \
    src/data/rail/forbid.cpp \
    src/data/rail/railcategory.cpp \
    src/data/rail/railinterval.cpp \
    src/data/rail/railstation.cpp \
    src/data/rail/railway.cpp \
    src/data/rail/ruler.cpp \
    src/data/rail/rulernode.cpp \
    src/data/common/stationname.cpp \
    src/data/train/routing.cpp \
    src/data/train/train.cpp \
    src/data/train/traincollection.cpp \
    src/data/train/trainfiltercore.cpp \
    src/data/train/trainname.cpp \
    src/data/train/trainstation.cpp \
    src/data/train/traintype.cpp \
    src/dialogs/batchcopytraindialog.cpp \
    src/dialogs/changestationnamedialog.cpp \
    src/dialogs/exchangeintervaldialog.cpp \
    src/dialogs/importtraindialog.cpp \
    src/dialogs/modifytimetabledialog.cpp \
    src/dialogs/outputsubdiagramdialog.cpp \
    src/dialogs/printdiagramdialog.cpp \
    src/dialogs/rulerfromtraindialog.cpp \
    src/dialogs/selectrailstationdialog.cpp \
    src/dialogs/trainfilter.cpp \
    src/editors/basictrainwidget.cpp \
    src/editors/configdialog.cpp \
    src/editors/forbidwidget.cpp \
    src/editors/railstationwidget.cpp \
    src/editors/routing/addroutingnodedialog.cpp \
    src/editors/routing/batchparseroutingdialog.cpp \
    src/editors/routing/detectroutingdialog.cpp \
    src/editors/routing/parseroutingdialog.cpp \
    src/editors/routing/routingdiagramwidget.cpp \
    src/editors/routing/routingedit.cpp \
    src/editors/routing/routingwidget.cpp \
    src/editors/ruler/rulertabpy.cpp \
    src/editors/ruler/rulerwidget.cpp \
    src/editors/systemjsondialog.cpp \
    src/editors/typeconfigdialog.cpp \
    src/editors/typeregexdialog.cpp \
    src/kernel/routingdiagram.cpp \
    src/mainwindow/routingcontext.cpp \
    src/model/delegate/generaldoublespindelegate.cpp \
    src/model/delegate/generalspindelegate.cpp \
    src/model/delegate/linestyledelegate.cpp \
    src/model/delegate/qetimedelegate.cpp \
    src/model/delegate/timeintervaldelegate.cpp \
    src/model/train/timetablequickmodel.cpp \
    src/model/train/typemodel.cpp \
    src/model/rail/forbidmodel.cpp \
    src/model/rail/intervaldatamodel.cpp \
    src/model/train/routingcollectionmodel.cpp \
    src/model/train/routingeditmodel.cpp \
    src/model/train/routinglistmodel.cpp \
    src/model/train/trainlistreadmodel.cpp \
    src/util/railrulercombo.cpp \
    src/util/selectrailwaycombo.cpp \
    src/util/selecttraincombo.cpp \
    src/viewers/diagnosisdialog.cpp \
    src/viewers/events/railsectionevents.cpp \
    src/viewers/events/railsnapevents.cpp \
    src/viewers/events/railstationeventlist.cpp \
    src/viewers/events/stationtraingapdialog.cpp \
    src/viewers/rulerrefdialog.cpp \
    src/viewers/events/stationtimetablesettled.cpp \
    src/editors/trainlistwidget.cpp \
    src/kernel/diagramwidget.cpp \
    src/kernel/trainitem.cpp \
    src/main.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/pagecontext.cpp \
    src/mainwindow/railcontext.cpp \
    src/mainwindow/rulercontext.cpp \
    src/mainwindow/traincontext.cpp \
    src/mainwindow/viewcategory.cpp \
    src/model/delegate/combodelegate.cpp \
    src/model/delegate/postivespindelegate.cpp \
    src/model/diagram/componentitems.cpp \
    src/model/diagram/diagramnavimodel.cpp \
    src/model/diagram/railtablemodel.cpp \
    src/model/general/qemoveablemodel.cpp \
    src/model/rail/railstationmodel.cpp \
    src/model/rail/rulermodel.cpp \
    src/model/train/timetablestdmodel.cpp \
    src/model/train/trainlistmodel.cpp \
    src/navi/addpagedialog.cpp \
    src/navi/navitree.cpp \
    src/util/dialogadapter.cpp \
    src/util/linestylecombo.cpp \
    src/util/qecontrolledtable.cpp \
    src/util/utilfunc.cpp \
    src/viewers/sectioncountdialog.cpp \
    src/viewers/timetablequickwidget.cpp \
    src/viewers/traindiffdialog.cpp \
    src/viewers/events/traineventdialog.cpp \
    src/viewers/traininfowidget.cpp \
    src/viewers/trainlinedialog.cpp \
    src/viewers/traintimetableplane.cpp \
    src/wizards/readruler/readrulerpageconfig.cpp \
    src/wizards/readruler/readrulerpageinterval.cpp \
    src/wizards/readruler/readrulerpagepreview.cpp \
    src/wizards/readruler/readrulerpagetrain.cpp \
    src/wizards/readruler/readrulerwizard.cpp \
    src/wizards/rulerpaint/conflictdialog.cpp \
    src/wizards/rulerpaint/rulerpaintpagestart.cpp \
    src/wizards/rulerpaint/rulerpaintpagestation.cpp \
    src/wizards/rulerpaint/rulerpaintpagetable.cpp \
    src/wizards/rulerpaint/rulerpaintwizard.cpp

HEADERS += \
    src/data/common/direction.h \
    src/data/common/qeglobal.h \
    src/data/common/qesystem.h \
    src/data/common/stationname.h \
    src/data/diagram/config.h \
    src/data/diagram/diagram.h \
    src/data/diagram/diagrampage.h \
    src/data/diagram/trainadapter.h \
    src/data/diagram/trainevents.h \
    src/data/diagram/traingap.h \
    src/data/diagram/trainline.h \
    src/data/rail/forbid.h \
    src/data/rail/rail.h \
    src/data/rail/railcategory.h \
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
    src/data/train/trainfiltercore.h \
    src/data/train/trainname.h \
    src/data/train/trainstation.h \
    src/data/train/traintype.h \
    src/dialogs/batchcopytraindialog.h \
    src/dialogs/changestationnamedialog.h \
    src/dialogs/exchangeintervaldialog.h \
    src/dialogs/importtraindialog.h \
    src/dialogs/modifytimetabledialog.h \
    src/dialogs/outputsubdiagramdialog.h \
    src/dialogs/printdiagramdialog.h \
    src/dialogs/rulerfromtraindialog.h \
    src/dialogs/selectrailstationdialog.h \
    src/dialogs/trainfilter.h \
    src/editors/basictrainwidget.h \
    src/editors/configdialog.h \
    src/editors/forbidwidget.h \
    src/editors/railstationwidget.h \
    src/editors/routing/addroutingnodedialog.h \
    src/editors/routing/batchparseroutingdialog.h \
    src/editors/routing/detectroutingdialog.h \
    src/editors/routing/parseroutingdialog.h \
    src/editors/routing/routingdiagramwidget.h \
    src/editors/routing/routingedit.h \
    src/editors/routing/routingwidget.h \
    src/editors/ruler/rulertabpy.h \
    src/editors/ruler/rulerwidget.h \
    src/editors/systemjsondialog.h \
    src/editors/typeconfigdialog.h \
    src/editors/typeregexdialog.h \
    src/kernel/routingdiagram.h \
    src/mainwindow/routingcontext.h \
    src/model/delegate/generaldoublespindelegate.h \
    src/model/delegate/generalspindelegate.h \
    src/model/delegate/linestyledelegate.h \
    src/model/delegate/qetimedelegate.h \
    src/model/delegate/timeintervaldelegate.h \
    src/model/train/timetablequickmodel.h \
    src/model/train/typemodel.h \
    src/model/rail/forbidmodel.h \
    src/model/rail/intervaldatamodel.h \
    src/model/train/routingcollectionmodel.h \
    src/model/train/routingeditmodel.h \
    src/model/train/routinglistmodel.h \
    src/model/train/trainlistreadmodel.h \
    src/util/railrulercombo.h \
    src/util/selectrailwaycombo.h \
    src/util/selecttraincombo.h \
    src/viewers/diagnosisdialog.h \
    src/viewers/events/railsectionevents.h \
    src/viewers/events/railsnapevents.h \
    src/viewers/events/railstationeventlist.h \
    src/viewers/events/stationtraingapdialog.h \
    src/viewers/rulerrefdialog.h \
    src/viewers/events/stationtimetablesettled.h \
    src/editors/trainlistwidget.h \
    src/kernel/diagramwidget.h \
    src/kernel/trainitem.h \
    src/mainwindow/mainwindow.h \
    src/mainwindow/pagecontext.h \
    src/mainwindow/railcontext.h \
    src/mainwindow/rulercontext.h \
    src/mainwindow/traincontext.h \
    src/mainwindow/version.h \
    src/mainwindow/viewcategory.h \
    src/model/delegate/combodelegate.h \
    src/model/delegate/postivespindelegate.h \
    src/model/delegate/qedelegate.h \
    src/model/diagram/componentitems.h \
    src/model/diagram/diagramnavimodel.h \
    src/model/diagram/railtablemodel.h \
    src/model/general/qemoveablemodel.h \
    src/model/rail/railstationmodel.h \
    src/model/rail/rulermodel.h \
    src/model/train/timetablestdmodel.h \
    src/model/train/trainlistmodel.h \
    src/navi/addpagedialog.h \
    src/navi/navitree.h \
    src/util/buttongroup.hpp \
    src/util/dialogadapter.h \
    src/util/linestylecombo.h \
    src/util/qecontrolledtable.h \
    src/util/qeexceptions.h \
    src/util/utilfunc.h \
    src/viewers/sectioncountdialog.h \
    src/viewers/timetablequickwidget.h \
    src/viewers/traindiffdialog.h \
    src/viewers/events/traineventdialog.h \
    src/viewers/traininfowidget.h \
    src/viewers/trainlinedialog.h \
    src/viewers/traintimetableplane.h \
    src/wizards/readruler/readrulerpageconfig.h \
    src/wizards/readruler/readrulerpageinterval.h \
    src/wizards/readruler/readrulerpagepreview.h \
    src/wizards/readruler/readrulerpagetrain.h \
    src/wizards/readruler/readrulerwizard.h \
    src/wizards/rulerpaint/conflictdialog.h \
    src/wizards/rulerpaint/rulerpaintpagestart.h \
    src/wizards/rulerpaint/rulerpaintpagestation.h \
    src/wizards/rulerpaint/rulerpaintpagetable.h \
    src/wizards/rulerpaint/rulerpaintwizard.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

msvc {
    QMAKE_CXXFLAGS += /utf-8
    QMAKE_CFLAGS += /utf-8
}

RESOURCES += \
    rsc/resource.qrc

DISTFILES += \
    rsc/icons/trainline.png
