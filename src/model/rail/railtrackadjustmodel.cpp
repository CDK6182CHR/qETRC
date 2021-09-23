#include "railtrackadjustmodel.h"

#include "data/rail/railtrack.h"

RailTrackAdjustModel::RailTrackAdjustModel(QVector<std::shared_ptr<Track> > &order,
                                           QObject *parent):
    QAbstractTableModel(parent),order(order)
{

}

int RailTrackAdjustModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return order.size();
}

int RailTrackAdjustModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant RailTrackAdjustModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.column()==0){
        switch(role){
        case Qt::EditRole:
        case Qt::DisplayRole:return order.at(index.row())->name();
        }
    }
    return {};
}

bool RailTrackAdjustModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()){
        switch(role){
        case Qt::EditRole:
        case Qt::DisplayRole:
            order.at(index.row())->setName(value.toString());
            return true;
        }
    }
    return false;
}

QVariant RailTrackAdjustModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal && section==0 && role==Qt::DisplayRole)
        return tr("股道名称");
    else return QAbstractTableModel::headerData(section,orientation,role);
}

void RailTrackAdjustModel::moveUp(int row)
{
    if(row>0 && row<order.size()){
        auto t=order.takeAt(row);
        order.insert(row-1,std::move(t));
        emit dataChanged(index(row-1,0),index(row,0),{Qt::DisplayRole});
    }
}

void RailTrackAdjustModel::moveDown(int row)
{
    if (row>=0 && row<order.size()-1){
        moveUp(row+1);
    }
}

Qt::ItemFlags RailTrackAdjustModel::flags(const QModelIndex &idx) const
{
    auto flags=QAbstractTableModel::flags(idx);
    if (idx.isValid())
        return flags | Qt::ItemIsEditable;
    else return flags;
}
