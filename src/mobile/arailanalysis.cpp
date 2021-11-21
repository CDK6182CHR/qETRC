#ifdef QETRC_MOBILE

#include "arailanalysis.h"
#include <util/buttongroup.hpp>
#include <util/selectrailwaycombo.h>
#include <QMessageBox>
#include <viewers/sectioncountdialog.h>
#include <viewers/events/railsectionevents.h>
#include <viewers/events/railsnapevents.h>
#include <viewers/events/railstationeventlist.h>
#include <viewers/events/railtrackwidget.h>
#include <viewers/events/stationtimetablesettled.h>
#include <viewers/events/stationtraingapdialog.h>
#include <viewers/events/traingapstatdialog.h>
#include <dialogs/selectrailstationdialog.h>

ARailAnalysis::ARailAnalysis(Diagram &diagram, SelectRailwayCombo *cbRails,
                             QWidget *parent):
    QWidget(parent),diagram(diagram),cbRails(cbRails)
{
    initUI();

    // 1. 断面对数表
    // 2. 车站车次表
    // 3. 车站事件表
    // 4. 断面事件表
    // 5. 运行快照
    // 6. 股道分析
    // 7. 间隔分析
    // 8. 间隔汇总
}

void ARailAnalysis::initUI()
{

    auto* vlay=new ButtonGroup<8,QVBoxLayout>({"断面对数表",
                                              "车站车次表",
                                              "车站事件表",
                                              "断面事件表",
                                              "运行快照",
                                              "股道分析",
                                              "间隔分析",
                                              "间隔汇总"});
    vlay->connectAll(SIGNAL(clicked()),this,{
                         SLOT(sectionCount()),
                          SLOT(stationTrains()),
                          SLOT(stationEvents()),
                          SLOT(sectionEvents()),
                          SLOT(railSnap()),
                          SLOT(trackAnalysis()),
                          SLOT(intervalAnalysis()),
                          SLOT(intervalSumamry())
                     });
    setLayout(vlay);
}

void ARailAnalysis::sectionCount()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto* w=new SectionCountDialog(diagram,rail,this);
    w->showMaximized();
}

void ARailAnalysis::stationTrains()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto st=SelectRailStationDialog::getStation(rail,this);
    if (!st) return;

    auto* w=new StationTimetableSettledDialog(diagram,rail,st,this);
    w->showMaximized();
}

void ARailAnalysis::stationEvents()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto st=SelectRailStationDialog::getStation(rail,this);
    if (!st) return;

    auto* w=new RailStationEventListDialog(diagram,rail,st,this);
    w->showMaximized();
}

void ARailAnalysis::sectionEvents()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto* w=new RailSectionEventsDialog(diagram,rail,this);
    w->showMaximized();
}

void ARailAnalysis::railSnap()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto* w=new RailSnapEventsDialog(diagram,rail,this);
    w->showMaximized();
}

void ARailAnalysis::trackAnalysis()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto st=SelectRailStationDialog::getStation(rail,this);
    if (!st) return;

    auto* w=new RailTrackWidget(diagram,rail,st,this);
    w->showMaximized();
}

void ARailAnalysis::intervalAnalysis()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto st=SelectRailStationDialog::getStation(rail,this);
    if (!st) return;


    auto* w=new StationTrainGapDialog(diagram,rail,st,this);
    w->showMaximized();
}

void ARailAnalysis::intervalSumamry()
{
    auto rail=cbRails->railway();
    if (!rail){
        QMessageBox::warning(this,tr("错误"),tr("当前没有选中线路！"));
        return;
    }

    auto* w=new TrainGapSummaryDialog(diagram,rail,this);
    w->showMaximized();
}

#endif
