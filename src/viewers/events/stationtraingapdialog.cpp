#include "stationtraingapdialog.h"
#include "data/diagram/traingap.h"
#include "data/diagram/trainline.h"

#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include "util/buttongroup.hpp"
#include <data/common/qesystem.h>
#include "util/utilfunc.h"
#include "model/delegate/timeintervaldelegate.h"

StationTrainGapModel::StationTrainGapModel(std::shared_ptr<Railway> railway_,
                                           std::shared_ptr<RailStation> station_,
                                           std::vector<TrainGap> &&data_, QObject *parent):
    QStandardItemModel(parent),railway(railway_),station(station_),
    data(std::forward<std::vector<TrainGap>>(data_))
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("间隔类型"),tr("间隔时长"),tr("方向"),tr("位置"),
        tr("前车次"),tr("前时刻"), tr("后车次"),tr("后时刻"),tr("数量")
        });
}

void StationTrainGapModel::refreshData()
{
    setupModel();
}

void StationTrainGapModel::setupModel()
{
    using SI = QStandardItem;
    setRowCount(data.size());
    for (int i = 0; i < data.size(); i++) {
        const auto& ev = data.at(i);
        setItem(i, ColGapType, new SI(ev.typeString()));
        auto* it = new SI;
        it->setData(ev.secs(), Qt::EditRole);
        setItem(i, ColPeriod, it);
        setItem(i, ColDir, new SI(DirFunc::dirToString(ev.leftDir())));
        setItem(i, ColPos, new SI(RailStationEvent::posToString(ev.position())));
        setItem(i, ColLeftTrain, new SI(ev.left->line->train()->trainName().full()));
        setItem(i, ColLeftTime, new SI(ev.left->time.toString("hh:mm:ss")));
        setItem(i, ColRightTrain, new SI(ev.right->line->train()->trainName().full()));
        setItem(i, ColRightTime, new SI(ev.right->time.toString("hh:mm:ss")));
        it = new SI;
        it->setData(ev.num, Qt::EditRole);
        setItem(i, ColNumber, it);
    }
}



StationTrainGapDialog::StationTrainGapDialog(std::shared_ptr<Railway> railway_,
                                             std::shared_ptr<RailStation> station_,
                                             std::vector<TrainGap> &&data_, QWidget *parent):
    QDialog(parent),railway(railway_),station(station_),
    model(new StationTrainGapModel(railway_,station_,std::forward<std::vector<TrainGap>>(data_)))
{
    setWindowTitle(tr("车站间隔分析 - %1 @ %2").arg(station->name.toSingleLiteral(),
        railway->name()));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(700,700);
    initUI();
    refreshData();
}

void StationTrainGapDialog::refreshData()
{
    model->refreshData();
    table->resizeColumnsToContents();
}

void StationTrainGapDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("下表为当前车站【已经显示的事件】对应的时间间隔。\n" 
        "目前仅考虑双线情况（不考虑会车），且仅认为车站同一端（例如，小里程端，即上行端）发生的" 
        "两事件存在关联，考虑其间隔（例如，同一车站同一方向终到和始发的两个车次事件无关）。" 
        ""));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->horizontalHeader()->setSortIndicatorShown(true);
    table->setItemDelegateForColumn(StationTrainGapModel::ColPeriod,
        new TimeIntervalDelegate(this));
    connect(table->horizontalHeader(),
            qOverload<int,Qt::SortOrder>(&QHeaderView::sortIndicatorChanged),
            table,
            qOverload<int,Qt::SortOrder>(&QTableView::sortByColumn)
            );
    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"导出CSV","关闭"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actToCsv()),SLOT(close())});
}

void StationTrainGapDialog::actToCsv()
{
    QString text=QString("%1_%2间隔分析").arg(railway->name(),station->name.toSingleLiteral());
    qeutil::exportTableToCsv(model,this,text);
}
