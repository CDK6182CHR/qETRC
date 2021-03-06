QT       += core gui
qtHaveModule(printsupport): QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 2021.09.28   rewrite lib pri files
#include(../../site-packages/SARibbon/SARibbonBar.pri)
#include(../site-packages/ads/ads.pri)

android {
DEFINES += QETRC_MOBILE
DEFINES += QETRC_MOBILE_2
}

! android {
RC_ICONS = rsc/icons/desktop-icon.ico
}

! android {
include(dependencies/qETRC_SARibbonBar.pri)
include(dependencies/qETRC_ads.pri)
}

INCLUDEPATH += src

SOURCES += \
    src/data/algo/timetablecorrector.cpp \
    src/data/analysis/inttrains/intervalcounter.cpp \
    src/data/analysis/inttrains/intervaltraininfo.cpp \
    src/data/analysis/traingap/traingapana.cpp \
    src/util/combos/railstationcombo.cpp \
    src/viewers/stats/intervaltraintable.cpp \
    src/data/calculation/calculationlog.cpp \
    src/data/calculation/gapconstraints.cpp \
    src/data/calculation/greedypainter.cpp \
    src/data/calculation/intervalconflictreport.cpp \
    src/data/calculation/railwaystationeventaxis.cpp \
    src/data/calculation/stationeventaxis.cpp \
    src/data/common/qesystem.cpp \
    src/data/common/stationname.cpp \
    src/data/diagram/config.cpp \
    src/data/diagram/diadiff.cpp \
    src/data/diagram/diagram.cpp \
    src/data/diagram/diagrampage.cpp \
    src/data/diagram/stationbinding.cpp \
    src/data/diagram/trainadapter.cpp \
    src/data/diagram/trainevents.cpp \
    src/data/diagram/traingap.cpp \
    src/data/diagram/trainline.cpp \
    src/data/gapset/crgroups.cpp \
    src/data/gapset/crset.cpp \
    src/data/gapset/gapgroupabstract.cpp \
    src/data/gapset/gapsetabstract.cpp \
    src/data/gapset/transparentset.cpp \
    src/data/rail/forbid.cpp \
    src/data/rail/railcategory.cpp \
    src/data/rail/railinfonote.cpp \
    src/data/rail/railinterval.cpp \
    src/data/rail/railstation.cpp \
    src/data/rail/railtrack.cpp \
    src/data/rail/railway.cpp \
    src/data/rail/ruler.cpp \
    src/data/rail/rulernode.cpp \
    src/data/rail/trackdiagramdata.cpp \
    src/data/train/routing.cpp \
    src/data/train/train.cpp \
    src/data/train/traincollection.cpp \
    src/data/train/trainfiltercore.cpp \
    src/data/train/trainname.cpp \
    src/data/train/trainstation.cpp \
    src/data/train/traintype.cpp \
    src/data/train/typemanager.cpp \
    src/dialogs/batchcopytraindialog.cpp \
    src/dialogs/changestationnamedialog.cpp \
    src/dialogs/correcttimetabledialog.cpp \
    src/dialogs/exchangeintervaldialog.cpp \
    src/dialogs/importtraindialog.cpp \
    src/dialogs/locatedialog.cpp \
    src/dialogs/modifytimetabledialog.cpp \
    src/dialogs/outputsubdiagramdialog.cpp \
    src/dialogs/printdiagramdialog.cpp \
    src/dialogs/rulerfromspeeddialog.cpp \
    src/dialogs/rulerfromtraindialog.cpp \
    src/dialogs/selectrailstationdialog.cpp \
    src/dialogs/trainfilter.cpp \
    src/editors/basictrainwidget.cpp \
    src/editors/configdialog.cpp \
    src/editors/edittrainwidget.cpp \
    src/editors/forbidwidget.cpp \
    src/editors/rail/gapconstraintwidget.cpp \
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
    src/editors/timetablewidget.cpp \
    src/editors/typeconfigdialog.cpp \
    src/editors/typeregexdialog.cpp \
    src/kernel/routingdiagram.cpp \
    src/kernel/trackdiagram.cpp \
    src/mainwindow/routingcontext.cpp \
    src/mobile/adiagrampage.cpp \
    src/mobile/amainwindow.cpp \
    src/mobile/arailanalysis.cpp \
    src/mobile/arailpage.cpp \
    src/mobile/astartpage.cpp \
    src/mobile/atrainoptions.cpp \
    src/mobile/atrainpage.cpp \
    src/model/delegate/generaldoublespindelegate.cpp \
    src/model/delegate/generalspindelegate.cpp \
    src/model/delegate/linestyledelegate.cpp \
    src/model/delegate/qetimedelegate.cpp \
    src/model/delegate/timeintervaldelegate.cpp \
    src/model/rail/forbidlistmodel.cpp \
    src/model/rail/gapconstraintmodel.cpp \
    src/model/rail/railtrackadjustmodel.cpp \
    src/model/train/timetablequickmodel.cpp \
    src/model/train/typemodel.cpp \
    src/model/rail/forbidmodel.cpp \
    src/model/rail/intervaldatamodel.cpp \
    src/model/train/routingcollectionmodel.cpp \
    src/model/train/routingeditmodel.cpp \
    src/model/train/routinglistmodel.cpp \
    src/model/train/trainlistreadmodel.cpp \
    src/railnet/graph/adjacentlistmodel.cpp \
    src/railnet/graph/adjacentlistwidget.cpp \
    src/railnet/graph/edgedatamodels.cpp \
    src/railnet/graph/graphinterval.cpp \
    src/railnet/graph/graphstation.cpp \
    src/railnet/graph/railnet.cpp \
    src/railnet/graph/vertexlistwidget.cpp \
    src/railnet/graph/viewadjacentwidget.cpp \
    src/railnet/path/graphpathmodel.cpp \
    src/railnet/path/pathoperation.cpp \
    src/railnet/path/pathoperationmodel.cpp \
    src/railnet/path/pathselectwidget.cpp \
    src/railnet/path/quickpathselector.cpp \
    src/railnet/path/railpreviewdialog.cpp \
    src/railnet/raildb/raildb.cpp \
    src/railnet/raildb/raildbcontext.cpp \
    src/railnet/raildb/raildbitems.cpp \
    src/railnet/raildb/raildbmodel.cpp \
    src/railnet/raildb/raildbnavi.cpp \
    src/railnet/raildb/raildbwindow.cpp \
    src/util/pagecomboforrail.cpp \
    src/util/qeballoomtip.cpp \
    src/util/railrangecombo.cpp \
    src/util/railrulercombo.cpp \
    src/util/selectrailwaycombo.cpp \
    src/util/selectrailwaystable.cpp \
    src/util/selecttraincombo.cpp \
    src/viewers/compare/diagramcomparedialog.cpp \
    src/viewers/compare/traincomparedialog.cpp \
    src/viewers/diagnosisdialog.cpp \
    src/viewers/events/railsectionevents.cpp \
    src/viewers/events/railsnapevents.cpp \
    src/viewers/events/railstationeventlist.cpp \
    src/viewers/events/railtrackwidget.cpp \
    src/viewers/events/stationtraingapdialog.cpp \
    src/viewers/events/traingapstatdialog.cpp \
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
    src/viewers/stats/intervalcountdialog.cpp \
    src/viewers/stats/intervaltraindialog.cpp \
    src/viewers/timetablequickwidget.cpp \
    src/viewers/traindiffdialog.cpp \
    src/viewers/events/traineventdialog.cpp \
    src/viewers/traininfowidget.cpp \
    src/viewers/trainlinedialog.cpp \
    src/viewers/traintimetableplane.cpp \
    src/wizards/greedypaint/greedypaintfasttest.cpp \
    src/wizards/greedypaint/greedypaintpageconstraint.cpp \
    src/wizards/greedypaint/greedypaintpagepaint.cpp \
    src/wizards/greedypaint/greedypaintwizard.cpp \
    src/wizards/readruler/readrulerpageconfig.cpp \
    src/wizards/readruler/readrulerpageinterval.cpp \
    src/wizards/readruler/readrulerpagepreview.cpp \
    src/wizards/readruler/readrulerpagetrain.cpp \
    src/wizards/readruler/readrulerwizard.cpp \
    src/wizards/rulerpaint/conflictdialog.cpp \
    src/wizards/rulerpaint/rulerpaintpagestart.cpp \
    src/wizards/rulerpaint/rulerpaintpagestation.cpp \
    src/wizards/rulerpaint/rulerpaintpagetable.cpp \
    src/wizards/rulerpaint/rulerpaintwizard.cpp \
    src/wizards/selectpath/selectpathpagepreview.cpp \
    src/wizards/selectpath/selectpathpagestart.cpp \
    src/wizards/selectpath/selectpathwizard.cpp \
    src/wizards/timeinterp/timeinterppagepreview.cpp \
    src/wizards/timeinterp/timeinterppagetrain.cpp \
    src/wizards/timeinterp/timeinterpwizard.cpp

HEADERS += \
    src/data/algo/timetablecorrector.h \
    src/data/analysis/inttrains/intervalcounter.h \
    src/data/analysis/inttrains/intervaltraininfo.h \
    src/data/analysis/traingap/traingapana.h \
    src/util/combos/railstationcombo.h \
    src/viewers/stats/intervaltraintable.h \
    src/data/calculation/calculationlog.h \
    src/data/calculation/gapconstraints.h \
    src/data/calculation/greedypainter.h \
    src/data/calculation/intervalconflictreport.h \
    src/data/calculation/railwaystationeventaxis.h \
    src/data/calculation/stationeventaxis.h \
    src/data/common/direction.h \
    src/data/common/qeglobal.h \
    src/data/common/qesystem.h \
    src/data/common/stationname.h \
    src/data/diagram/config.h \
    src/data/diagram/diadiff.h \
    src/data/diagram/diagram.h \
    src/data/diagram/diagrampage.h \
    src/data/diagram/stationbinding.h \
    src/data/diagram/trainadapter.h \
    src/data/diagram/trainevents.h \
    src/data/diagram/traingap.h \
    src/data/diagram/trainline.h \
    src/data/diagram/xtl_matrix.hpp \
    src/data/gapset/crgroups.h \
    src/data/gapset/crset.h \
    src/data/gapset/gapgroupabstract.h \
    src/data/gapset/gapsetabstract.h \
    src/data/gapset/transparentset.h \
    src/data/rail/forbid.h \
    src/data/rail/rail.h \
    src/data/rail/railcategory.h \
    src/data/rail/railinfonote.h \
    src/data/rail/railinterval.h \
    src/data/rail/railintervaldata.hpp \
    src/data/rail/railintervalnode.hpp \
    src/data/rail/railstation.h \
    src/data/rail/railtrack.h \
    src/data/rail/railway.h \
    src/data/rail/ruler.h \
    src/data/rail/rulernode.h \
    src/data/rail/trackdiagramdata.h \
    src/data/train/routing.h \
    src/data/train/train.h \
    src/data/train/traincollection.h \
    src/data/train/trainfiltercore.h \
    src/data/train/trainname.h \
    src/data/train/trainstation.h \
    src/data/train/traintype.h \
    src/data/train/typemanager.h \
    src/dialogs/batchcopytraindialog.h \
    src/dialogs/changestationnamedialog.h \
    src/dialogs/correcttimetabledialog.h \
    src/dialogs/exchangeintervaldialog.h \
    src/dialogs/importtraindialog.h \
    src/dialogs/locatedialog.h \
    src/dialogs/modifytimetabledialog.h \
    src/dialogs/outputsubdiagramdialog.h \
    src/dialogs/printdiagramdialog.h \
    src/dialogs/rulerfromspeeddialog.h \
    src/dialogs/rulerfromtraindialog.h \
    src/dialogs/selectrailstationdialog.h \
    src/dialogs/trainfilter.h \
    src/editors/basictrainwidget.h \
    src/editors/configdialog.h \
    src/editors/edittrainwidget.h \
    src/editors/forbidwidget.h \
    src/editors/rail/gapconstraintwidget.h \
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
    src/editors/timetablewidget.h \
    src/editors/typeconfigdialog.h \
    src/editors/typeregexdialog.h \
    src/kernel/routingdiagram.h \
    src/kernel/trackdiagram.h \
    src/mainwindow/routingcontext.h \
    src/mobile/adiagrampage.h \
    src/mobile/amainwindow.h \
    src/mobile/arailanalysis.h \
    src/mobile/arailpage.h \
    src/mobile/astartpage.h \
    src/mobile/atrainoptions.h \
    src/mobile/atrainpage.h \
    src/model/delegate/generaldoublespindelegate.h \
    src/model/delegate/generalspindelegate.h \
    src/model/delegate/linestyledelegate.h \
    src/model/delegate/qetimedelegate.h \
    src/model/delegate/timeintervaldelegate.h \
    src/model/rail/forbidlistmodel.h \
    src/model/rail/gapconstraintmodel.h \
    src/model/rail/railtrackadjustmodel.h \
    src/model/train/timetablequickmodel.h \
    src/model/train/typemodel.h \
    src/model/rail/forbidmodel.h \
    src/model/rail/intervaldatamodel.h \
    src/model/train/routingcollectionmodel.h \
    src/model/train/routingeditmodel.h \
    src/model/train/routinglistmodel.h \
    src/model/train/trainlistreadmodel.h \
    src/railnet/graph/adjacentlistmodel.h \
    src/railnet/graph/adjacentlistwidget.h \
    src/railnet/graph/edgedatamodels.h \
    src/railnet/graph/graphinterval.h \
    src/railnet/graph/graphstation.h \
    src/railnet/graph/railnet.h \
    src/railnet/graph/vertexlistwidget.h \
    src/railnet/graph/viewadjacentwidget.h \
    src/railnet/graph/xtl_graph.hpp \
    src/railnet/path/graphpathmodel.h \
    src/railnet/path/pathoperation.h \
    src/railnet/path/pathoperationmodel.h \
    src/railnet/path/pathselectwidget.h \
    src/railnet/path/quickpathselector.h \
    src/railnet/path/railpreviewdialog.h \
    src/railnet/raildb/raildb.h \
    src/railnet/raildb/raildbcontext.h \
    src/railnet/raildb/raildbitems.h \
    src/railnet/raildb/raildbmodel.h \
    src/railnet/raildb/raildbnavi.h \
    src/railnet/raildb/raildbwindow.h \
    src/util/pagecomboforrail.h \
    src/util/qeballoomtip.h \
    src/util/railrangecombo.h \
    src/util/railrulercombo.h \
    src/util/selectrailwaycombo.h \
    src/util/selectrailwaystable.h \
    src/util/selecttraincombo.h \
    src/viewers/compare/diagramcomparedialog.h \
    src/viewers/compare/traincomparedialog.h \
    src/viewers/diagnosisdialog.h \
    src/viewers/events/railsectionevents.h \
    src/viewers/events/railsnapevents.h \
    src/viewers/events/railstationeventlist.h \
    src/viewers/events/railtrackwidget.h \
    src/viewers/events/stationtraingapdialog.h \
    src/viewers/events/traingapstatdialog.h \
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
    src/viewers/stats/intervalcountdialog.h \
    src/viewers/stats/intervaltraindialog.h \
    src/viewers/timetablequickwidget.h \
    src/viewers/traindiffdialog.h \
    src/viewers/events/traineventdialog.h \
    src/viewers/traininfowidget.h \
    src/viewers/trainlinedialog.h \
    src/viewers/traintimetableplane.h \
    src/wizards/greedypaint/greedypaintfasttest.h \
    src/wizards/greedypaint/greedypaintpageconstraint.h \
    src/wizards/greedypaint/greedypaintpagepaint.h \
    src/wizards/greedypaint/greedypaintwizard.h \
    src/wizards/readruler/readrulerpageconfig.h \
    src/wizards/readruler/readrulerpageinterval.h \
    src/wizards/readruler/readrulerpagepreview.h \
    src/wizards/readruler/readrulerpagetrain.h \
    src/wizards/readruler/readrulerwizard.h \
    src/wizards/rulerpaint/conflictdialog.h \
    src/wizards/rulerpaint/rulerpaintpagestart.h \
    src/wizards/rulerpaint/rulerpaintpagestation.h \
    src/wizards/rulerpaint/rulerpaintpagetable.h \
    src/wizards/rulerpaint/rulerpaintwizard.h \
    src/wizards/selectpath/selectpathpagepreview.h \
    src/wizards/selectpath/selectpathpagestart.h \
    src/wizards/selectpath/selectpathwizard.h \
    src/wizards/timeinterp/timeinterppagepreview.h \
    src/wizards/timeinterp/timeinterppagetrain.h \
    src/wizards/timeinterp/timeinterpwizard.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

msvc {
    QMAKE_CXXFLAGS += /utf-8 /wd"4267"
    QMAKE_CFLAGS   += /utf-8 /wd"4267"
}

RESOURCES += \
    rsc/resource.qrc

DISTFILES += \
    Android_source/AndroidManifest.xml \
    rsc/icons/trainline.png

TRANSLATIONS += rsc/tr/en.ts rsc/tr/ch.ts

! defined(QETRC_MOBILE_2){
ANDROID_EXTRA_LIBS = D:/QTProject/qETRC/qETRC/lib/ads/Qt5.15.2-android-clang/libqtadvanceddocking.so $$PWD/lib/SARibbon/Qt5.15.2-android-clang/libSARibbonBar.so
}
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/Android_source

ANDROID_EXTRA_LIBS =


