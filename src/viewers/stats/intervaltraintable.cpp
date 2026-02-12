#include "intervaltraintable.h"

#include <QHeaderView>
#include "data/common/qesystem.h"
#include "util/utilfunc.h"
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "model/delegate/timeintervaldelegate.h"
#include "model/delegate/traintimedelegate.h"
#include "model/delegate/generaldoublespindelegate.h"
#include "data/diagram/diagramoptions.h"
#include "data/analysis/runstat/trainintervalstat.h"


IntervalTrainModel::IntervalTrainModel(const DiagramOptions& ops, QWidget *parent):
    QStandardItemModel(parent), _ops(ops)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
       tr("车次"),tr("类型"),tr("发站"),tr("发时"),tr("到站"),tr("到时"),
                                  tr("历时"),tr("始发"),tr("终到"), 
								  tr("里程"), tr("旅速"), tr("技速")
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
		const auto& stat = _statData[i];

        auto* it=new SI(info.train->trainName().full());
        setItem(i,ColTrainName,it);

        setItem(i,ColType,new SI(info.train->type()->name()));
        setItem(i,ColFromStation,new SI(info.from->name.toSingleLiteral()));
        setItem(i, ColToStation, new SI(info.to->name.toSingleLiteral()));

        it = new SI;
        it->setData(info.from->depart.toQVariant(), Qt::EditRole);
        setItem(i,ColFromTime,it);
        
        it = new SI;
        it->setData(info.to->arrive.toQVariant(), Qt::EditRole);
        setItem(i,ColToTime,it);

        int secs=qeutil::secsToStrict(info.from->depart,info.to->arrive,info.addDays, _ops.period_hours);
        it = new SI;
        it->setData(secs, Qt::EditRole);
        setItem(i, ColTime, it);
        setItem(i,ColStarting,new SI(info.train->starting().toSingleLiteral()));
        setItem(i,ColTerminal,new SI(info.train->terminal().toSingleLiteral()));

        // 2026.02.12: Mile and speed if available
        if (stat.railResults.isValid) {
            it = new SI;
			it->setData(stat.railResults.totalMiles, Qt::EditRole);
            setItem(i, ColTotalMile, it);
			it = new SI;
			it->setData(stat.railResults.travelSpeed, Qt::EditRole);
			setItem(i, ColTravSpeed, it);
			it = new SI;
			it->setData(stat.railResults.techSpeed, Qt::EditRole);
            setItem(i, ColTechSpeed, it);
        }
        else {
            it = new SI;
            it->setData(NAN, Qt::EditRole);
            setItem(i, ColTotalMile, it);

            it = new SI;
            it->setData(NAN, Qt::EditRole);
            setItem(i, ColTravSpeed, it);

            it = new SI;
            it->setData(NAN, Qt::EditRole);
            setItem(i, ColTechSpeed, it);
        }
    }
}

void IntervalTrainModel::resetData(IntervalTrainList &&data_)
{
    _data=std::move(data_);
	updateTrainStatData();
    setupModel();
}

void IntervalTrainModel::resetData(const IntervalTrainList& data_)
{
    _data = data_;
	updateTrainStatData();
    setupModel();
}

void IntervalTrainModel::updateTrainStatData()
{
    _statData = TrainIntervalStat::computeFromIntervalInfo(_ops, _data, false);
}

IntervalTrainTable::IntervalTrainTable(const DiagramOptions& ops, QWidget *parent):
    QTableView(parent), _ops(ops), model(new IntervalTrainModel(_ops, parent))
{
    initUI();
}

void IntervalTrainTable::initUI()
{
    verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    setEditTriggers(QTableView::NoEditTriggers);
    setModel(model);
    horizontalHeader()->setSectionsClickable(true);
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged,
        this, qOverload<int, Qt::SortOrder>(&QTableView::sortByColumn));

    {
        int c=0;
        for(int w:{80, 60, 110, 90, 110, 90, 120, 80, 80, 80, 80, 80}){
            setColumnWidth(c++,w);
        }
    }
    
    auto* time_del = new TrainTimeDelegate(_ops, this);
    setItemDelegateForColumn(IntervalTrainModel::ColFromTime, time_del);
    setItemDelegateForColumn(IntervalTrainModel::ColToTime, time_del);
    setItemDelegateForColumn(IntervalTrainModel::ColTime, new TimeIntervalDelegateHour(this));

    auto* num_del = new GeneralDoubleSpinDelegate(this);
	setItemDelegateForColumn(IntervalTrainModel::ColTotalMile, num_del);
	setItemDelegateForColumn(IntervalTrainModel::ColTravSpeed, num_del);
	setItemDelegateForColumn(IntervalTrainModel::ColTechSpeed, num_del);

    horizontalHeader()->setSortIndicatorShown(true);
}
