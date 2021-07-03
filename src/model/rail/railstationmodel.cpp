#include "railstationmodel.h"

#include <QtWidgets>


RailStationModel::RailStationModel(QUndoStack *undo, QObject *parent):
    QStandardItemModel(parent),_undo(undo)
{
    setupModel();
}


void RailStationModel::setRailway(std::shared_ptr<Railway> rail)
{
    railway = rail;
    setupModel();
}

bool RailStationModel::insertRows(int row, int count, const QModelIndex& parent)
{
    using SI = QStandardItem;
    QStandardItemModel::insertRows(row, count, parent);
    //设置新插入的列的数据
    //默认情况下，新插入的行有没有Item? 没有！
    for (int i = row; i < row + count; i++) {
        setItem(i, ColName, new QStandardItem);
        
        auto* it = new SI;
        it->setData(0.0, Qt::EditRole);
        setItem(i, ColMile, it);

        setItem(i, ColCounter, new SI);

        it = new SI;
        it->setData(4, Qt::EditRole);
        setItem(i, ColLevel, it);

        it = new SI;
        it->setCheckState(Qt::Checked);
        setItem(i, ColShow, it);

        it = new SI;
        it->setData(3, Qt::EditRole);
        it->setData(qeutil::passDirStr(PassedDirection::BothVia), Qt::DisplayRole);
        setItem(i, ColDir, it);

        it = new SI;
        it->setCheckState(Qt::Unchecked);
        setItem(i, ColPassenger, it);

        it = new SI;
        it->setCheckState(Qt::Unchecked);
        setItem(i, ColFreight, it);
    }
    return true;
}

void RailStationModel::setupModel()
{
    beginResetModel();
    setColumnCount(ColMAX);
    if (!railway) {
        setRowCount(0);
        return;
    }

    setRowCount(railway->stationCount());

    setHorizontalHeaderLabels({ "站名","里程","对里程","等级","显示","单向站",
        "办客","办货" });

    for (int i = 0; i < rowCount(); i++) {
        //站名
        auto st = railway->stations().at(i);
        QStandardItem* item = new QStandardItem(st->name.toSingleLiteral());
        setItem(i, ColName, item);

        //里程
        item = new QStandardItem(QString::number(st->mile, 'f', 3));
        item->setData(st->mile, Qt::EditRole);
        setItem(i, ColMile, item);

        //对里程
        item = new QStandardItem;
        if (st->counter.has_value())
            item->setData(QString::number(st->counter.value(), 'f', 3), Qt::DisplayRole);
        else
            item->setData("", Qt::DisplayRole);
        setItem(i, ColCounter, item);

        //等级
        item = new QStandardItem(QString::number(st->level));
        item->setData(st->level, Qt::EditRole);
        setItem(i, ColLevel, item);

        //显示
        item = new QStandardItem;
        item->setCheckState(st->_show ? Qt::Checked : Qt::Unchecked);
        item->setCheckable(true);
        setItem(i, ColShow, item);

        //单向站
        item = new QStandardItem;
        item->setData(static_cast<int>(st->direction), Qt::EditRole);
        item->setData(qeutil::passDirStr(st->direction), Qt::DisplayRole);
        setItem(i, ColDir, item);

        //办客
        item = new QStandardItem;
        item->setCheckable(true);
        item->setCheckState(qeutil::boolToCheckState(st->passenger));
        setItem(i, ColPassenger, item);

        //办货
        item = new QStandardItem;
        item->setCheckable(true);
        item->setCheckState(qeutil::boolToCheckState(st->freight));
        setItem(i, ColFreight, item);
    }
    endResetModel();
}

