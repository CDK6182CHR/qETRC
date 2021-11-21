#include "stationtimetablesettled.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QScroller>

#include "util/buttongroup.hpp"
#include "util/utilfunc.h"
#include "data/diagram/diagram.h"
#include "data/train/routing.h"
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "data/common/qesystem.h"

StationTimetableSettledModel::StationTimetableSettledModel(
        Diagram& dia,
        std::shared_ptr<Railway> rail_,
        std::shared_ptr<RailStation> station_, QObject *parent):
    QStandardItemModel(parent),diagram(dia), rail(rail_),station(station_),
    lst(dia.stationTrainsSettled(rail,station))
{
    setColumnCount(ColMAX);
    setupModel();
}

void StationTimetableSettledModel::setupModel()
{
    beginResetModel();
    setRowCount(lst.size());
    using SI=QStandardItem;

    setHorizontalHeaderLabels({
        tr("车次"),tr("站名"),tr("到点"),tr("开点"),tr("类型"),tr("停站"),
        tr("行别"),tr("始发"),tr("终到"),tr("股道"),tr("车底"),tr("担当"),tr("备注")
        });

    for(int i=0;i<rowCount();i++){
        const auto& p=lst.at(i);
        std::shared_ptr<Train> train=p.first->train();
        setItem(i,ColTrainName,new SI(train->trainName().full()));
        setItem(i,ColStationName,new SI(p.second->trainStation->name.toSingleLiteral()));
        setItem(i,ColArrive,new SI(p.second->trainStation->arrive
                                   .toString("hh:mm:ss")));
        setItem(i,ColDepart,new SI(p.second->trainStation->depart
                                   .toString("hh:mm:ss")));
        setItem(i,ColType,new SI(train->type()->name()));
        QString stop;
        if(train->isStartingStation(p.second))
            stop=tr("始发");
        else if(train->isTerminalStation(p.second))
            stop=tr("终到");
        else if(p.second->trainStation->isStopped())
            stop=tr("停车");
        else stop=tr("通过");
        setItem(i,ColStop,new SI(stop));
        setItem(i,ColDir,new SI(DirFunc::dirToString(p.first->dir())));
        setItem(i,ColStarting,new SI(train->starting().toSingleLiteral()));
        setItem(i,ColTerminal,new SI(train->terminal().toSingleLiteral()));
        setItem(i,ColTrack,new SI(p.second->trainStation->track));
        setItem(i,ColNote,new SI(p.second->trainStation->note));
        if(train->hasRouting()){
            auto r=train->routing().lock();
            setItem(i,ColModel,new SI(r->model()));
            setItem(i,ColOwner,new SI(r->owner()));
        }else{
            setItem(i,ColModel,new SI("-"));
            setItem(i,ColOwner,new SI("-"));
        }
    }
    endResetModel();
}

StationTimetableSettledDialog::StationTimetableSettledDialog(
        Diagram &diagram_, std::shared_ptr<Railway> rail_,
        std::shared_ptr<RailStation> station_, QWidget *parent):
    QDialog(parent),diagram(diagram_),rail(rail_),station(station_),
    model(new StationTimetableSettledModel(diagram,rail,station,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void StationTimetableSettledDialog::initUI()
{
    setWindowTitle(tr("车站图定时刻表 - %1 @ %2").arg(station->name.toSingleLiteral())
        .arg(rail->name()));
    resize(600,600);

    auto* vlay=new QVBoxLayout;
    auto* lab=new QLabel(tr("此功能与pyETRC的车站时刻表功能逻辑一致，"
        "只显示列车图定时刻表中，包含此站的列车，而不进行推定，并且以列车为单元。"
        "如果需要基于事件的车站事件表，请使用车站事件表功能。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    ckHidePass=new QCheckBox(tr("隐藏通过列车"));
    connect(ckHidePass, &QCheckBox::toggled, this, 
        &StationTimetableSettledDialog::onHidePassChanged);
    vlay->addWidget(ckHidePass);
    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnsToContents();
    vlay->addWidget(table);
    QScroller::grabGesture(table,QScroller::TouchGesture);

    auto* g=new ButtonGroup<2>({"导出CSV","关闭"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(outputCsv()),SLOT(close())});
    vlay->addLayout(g);
    setLayout(vlay);
}

void StationTimetableSettledDialog::onHidePassChanged(bool on)
{
    if (on) {
        for (int i = 0; i < model->rowCount(); i++) {
            bool a = (model->item(i, StationTimetableSettledModel::ColStop)->text() ==
                QObject::tr("通过"));
            table->setRowHidden(i, a);
        }
    }
    else {
        for (int i = 0; i < model->rowCount(); i++) {
            table->setRowHidden(i, false);
        }
    }
}

void StationTimetableSettledDialog::outputCsv()
{
    QString iname = tr("%1图定时刻表.csv").arg(station->name.toSingleLiteral());
    qeutil::exportTableToCsv(model, this, iname);
}
