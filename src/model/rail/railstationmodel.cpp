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
        it->setCheckable(true);
        it->setEditable(false);
        setItem(i, ColShow, it);

        it = new SI;
        it->setData(3, Qt::EditRole);
        setItem(i, ColDir, it);

        it = new SI;
        it->setCheckState(Qt::Unchecked);
        it->setCheckable(true);
        it->setEditable(false);
        setItem(i, ColPassenger, it);

        it = new SI;
        it->setCheckState(Qt::Unchecked);
        it->setEditable(false);
        it->setCheckable(true);
        setItem(i, ColFreight, it);
    }
    return true;
}

QVariant RailStationModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && index.column() == ColDir && role == Qt::DisplayRole) {
        return qeutil::passDirStr(index.data(Qt::EditRole).toInt());
    }
    return QStandardItemModel::data(index, role);
}

Qt::ItemFlags RailStationModel::flags(const QModelIndex& index) const
{
    return QStandardItemModel::flags(index);
}

bool RailStationModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, 
    const QModelIndex& destinationParent, int destinationChild)
{
    if (sourceParent == destinationParent) {
        for (int i = 0; i < count; i++) {
            int src = sourceRow + i, dst = destinationChild + i;
            if (src != dst) {
                //在dst前面插入一行，内容为src；然后删除src那一行
                insertRow(dst);
                int nsrc = src > dst ? src + 1 : src;
                for (int j = 0; j < ColMAX; j++) {
                    setItem(dst, j, takeItem(nsrc, j));  //注行号可能变了!
                }
                removeRow(nsrc);
            }
        }
    }

    return QStandardItemModel::moveRows(sourceParent, sourceRow, count, 
        destinationParent, destinationChild);
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
        item->setEditable(false);
        setItem(i, ColShow, item);

        //单向站
        item = new QStandardItem;
        item->setData(static_cast<int>(st->direction), Qt::EditRole);
        setItem(i, ColDir, item);

        //办客
        item = new QStandardItem;
        item->setCheckable(true);
        item->setEditable(false);
        item->setCheckState(qeutil::boolToCheckState(st->passenger));
        setItem(i, ColPassenger, item);

        //办货
        item = new QStandardItem;
        item->setCheckable(true);
        item->setEditable(false);
        item->setCheckState(qeutil::boolToCheckState(st->freight));
        setItem(i, ColFreight, item);
    }
    endResetModel();
}

