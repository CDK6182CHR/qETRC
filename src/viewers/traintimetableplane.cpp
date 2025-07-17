#include "traintimetableplane.h"

#include <QHeaderView>

#include "data/common/qesystem.h"
#include "model/delegate/traintimedelegate.h"
#include "data/train/train.h"

TrainTimetablePlane::TrainTimetablePlane(const DiagramOptions& ops, QWidget *parent) :
    QTableView(parent), _ops(ops), model(new TimetableConstModel(this))
{
    setModel(model);
    verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    setEditTriggers(QTableView::NoEditTriggers);
    setItemDelegateForColumn(TimetableStdModel::ColArrive,
        new TrainTimeDelegate(_ops, this));
    setItemDelegateForColumn(TimetableStdModel::ColDepart,
        new TrainTimeDelegate(_ops, this));
}

void TrainTimetablePlane::setTrain(std::shared_ptr<const Train> train_)
{
    train=train_;
    model->setTrain(train);
    resizeColumnsToContents();
    setWindowTitle(tr("列车时刻表 - %1").arg(train->trainName().full()));
}
