#include "intervaltraintable.h"

#include <QHeaderView>
#include "data/common/qesystem.h"
#include "util/utilfunc.h"
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "model/delegate/timeintervaldelegate.h"
#include "model/delegate/traintimedelegate.h"

IntervalTrainModel::IntervalTrainModel(QWidget *parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
       tr("车次"),tr("类型"),tr("发站"),tr("发时"),tr("到站"),tr("到时"),
                                  tr("历时"),tr("始发"),tr("终到")
                              });
}

void IntervalTrainModel::refreshData()
{
    setupModel();
}

void IntervalTrainModel::setupModel()
{
    using SI=QStandardItem;
    setRowCount(_data.size());
    for(size_t i=0;i<_data.size();i++){
        const auto& info=_data[i];

        auto* it=new SI(info.train->trainName().full());
        setItem(i,ColTrainName,it);

        setItem(i,ColType,new SI(info.train->type()->name()));
        setItem(i,ColFromStation,new SI(info.from->name.toSingleLiteral()));
        setItem(i, ColToStation, new SI(info.to->name.toSingleLiteral()));

        it = new SI;
        it->setData(QVariant::fromValue(info.from->depart), Qt::EditRole);
        setItem(i,ColFromTime,it);
        
        it = new SI;
        it->setData(QVariant::fromValue(info.to->arrive), Qt::EditRole);
        setItem(i,ColToTime,it);

        int secs=qeutil::secsToStrict(info.from->depart,info.to->arrive,info.addDays);
        it = new SI;
        it->setData(secs, Qt::EditRole);
        setItem(i, ColTime, it);
        setItem(i,ColStarting,new SI(info.train->starting().toSingleLiteral()));
        setItem(i,ColTerminal,new SI(info.train->terminal().toSingleLiteral()));
    }
}

void IntervalTrainModel::resetData(IntervalTrainList &&data_)
{
    _data=std::move(data_);
    setupModel();
}

void IntervalTrainModel::resetData(const IntervalTrainList& data_)
{
    _data = data_;
    setupModel();
}


IntervalTrainTable::IntervalTrainTable(const DiagramOptions& ops, QWidget *parent):
    _ops(ops), model(new IntervalTrainModel(parent))
{
    initUI();
}

void IntervalTrainTable::initUI()
{
    verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    setEditTriggers(QTableView::NoEditTriggers);
    setModel(model);
    horizontalHeader()->setSectionsClickable(true);
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged,
        this, qOverload<int, Qt::SortOrder>(&QTableView::sortByColumn));

    {
        int c=0;
        for(int w:{80, 60, 110, 90, 110, 90, 120, 80, 80}){
            setColumnWidth(c++,w);
        }
    }
    
    auto* time_del = new TrainTimeDelegate(_ops, this);
    setItemDelegateForColumn(IntervalTrainModel::ColFromTime, time_del);
    setItemDelegateForColumn(IntervalTrainModel::ColToTime, time_del);
    setItemDelegateForColumn(IntervalTrainModel::ColTime, new TimeIntervalDelegateHour(this));
}
