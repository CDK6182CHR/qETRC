#include "rulerrefdialog.h"


RulerRefModel::RulerRefModel(Diagram &diagram_,
                             std::shared_ptr<Train> train_, QObject *parent):
    QStandardItemModel(parent),diagram(diagram_),train(train_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
       tr("区间"),tr("标准"),tr("起"),tr("停"),tr("实际"),tr("里程"),
                                  tr("技速"),tr("附加"),tr("差时")
                              });
    setIndex(0);
}

void RulerRefModel::setupModel()
{
    beginResetModel();
    // todo ...
    endResetModel();
}

void RulerRefModel::setIndex(int i)
{
    adp=train->adapters().at(i);
    setupModel();
}
