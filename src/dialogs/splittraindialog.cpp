#include "splittraindialog.h"

#include "data/train/train.h"
#include "util/utilfunc.h"

SplitTrainModel::SplitTrainModel(std::shared_ptr<Train> train, QObject *parent):
    QStandardItemModel(parent), _train(train)
{
    setHorizontalHeaderLabels({
        tr("站名"), tr("到点"), tr("开点"), tr("营业"), tr("停时"),
        tr("新车次名")
    });
    setupModel();
}

void SplitTrainModel::setupModel()
{
    refreshData();
}

void SplitTrainModel::refreshData()
{
    using SI=QStandardItem;
    if (!_train){
        setRowCount(0);
        return;
    }

    setRowCount(_train->stationCount());

    int n=0;
    for (auto itr=_train->timetable().begin(); itr!=_train->timetable().end();++itr,++n){
        auto* it=new SI(itr->name.toSingleLiteral());
        it->setEditable(false);
        setItem(n, ColName, it);

        it=new SI(itr->arrive.toString("hh:mm:ss"));
        it->setEditable(false);
        setItem(n, ColArrive, it);

        it=new SI(itr->arrive.toString("hh:mm:ss"));
        it->setEditable(false);
        setItem(n, ColDepart, it);

        it=new SI;
        it->setCheckState(qeutil::boolToCheckState(itr->business));
        it->setCheckable(false);
        setItem(n, ColBusiness, it);

        it=new SI(itr->stopString());
        it->setEditable(false);
        setItem(n, ColStopTime, it);

        it=new SI;
        it->setCheckState(Qt::Unchecked);
        it->setCheckable(true);
        setItem(n, ColNewTrainName, it);
    }
}
