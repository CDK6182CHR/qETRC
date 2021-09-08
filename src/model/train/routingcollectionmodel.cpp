#include "routingcollectionmodel.h"

#include "data/train/traincollection.h"
#include "data/train/routing.h"

RoutingCollectionModel::RoutingCollectionModel(TrainCollection &coll_, QObject *parent):
    QAbstractTableModel(parent),coll(coll_)
{

}

int RoutingCollectionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return coll.routings().count();
}

int RoutingCollectionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColMAX;
}

QVariant RoutingCollectionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    if (role == Qt::DisplayRole) {
        const auto& rt = coll.routingAt(index.row());
        switch (index.column())
        {
        case RoutingCollectionModel::ColName: return rt->name();
        case RoutingCollectionModel::ColOrder:return rt->orderString();
        case RoutingCollectionModel::ColModel:return rt->model();
        case RoutingCollectionModel::ColOwner:return rt->owner();
        case RoutingCollectionModel::ColNote:return rt->note();
        default:
            break;
        }
    }
    else if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case RoutingCollectionModel::ColName: 
            return coll.routingAt(index.row())->isHighlighted() ? Qt::Checked : Qt::Unchecked;
        }
    }
    return {};
}

bool RoutingCollectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid()) return false;
    if(role == Qt::CheckStateRole){
        if (index.column() == ColName){
            auto rt = coll.routingAt(index.row());
            bool on = (qvariant_cast<Qt::CheckState>(value) == Qt::Checked);
            if (rt->isHighlighted() != on)
                emit routingHighlightChanged(coll.routingAt(index.row()), on);
            return true;
        }
    }
    return false;
}

QVariant RoutingCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section) {
        case ColName: return tr("交路名");
        case ColOrder: return tr("车次列");
        case ColModel: return tr("车底");
        case ColOwner: return tr("担当");
        case ColNote: return tr("注释");
        }
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

Qt::ItemFlags RoutingCollectionModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
    case ColName:return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    }
    return QAbstractTableModel::flags(index);
}

void RoutingCollectionModel::refreshData()
{
    beginResetModel();
    endResetModel();
}

void RoutingCollectionModel::onRoutingInfoChanged(std::shared_ptr<Routing> routing)
{
    int i = coll.getRoutingIndex(routing);
    emit dataChanged(index(i, ColName), index(i, ColMAX - 1), { Qt::DisplayRole });
}

void RoutingCollectionModel::addRoutingAt(std::shared_ptr<Routing> routing, int i)
{
    beginInsertRows({}, i, i);
    routing->updateTrainHooks();
    coll.routings().insert(i, routing);
    endInsertRows();
}

void RoutingCollectionModel::removeRoutingAt(int i)
{
    beginRemoveRows({}, i, i);
    auto r = coll.routings().takeAt(i);
    r->resetTrainHooks();
    endRemoveRows();
}

void RoutingCollectionModel::appendRoutings(const QList<std::shared_ptr<Routing>>& routings)
{
    int r = coll.routings().size();
    beginInsertRows({}, r, r + routings.size() - 1);
    coll.routings().append(routings);
    foreach(auto t, routings) {
        t->updateTrainHooks();
    }
    endInsertRows();
}

void RoutingCollectionModel::removeTailRoutings(int count)
{
    beginRemoveRows({}, coll.routings().size() - count, coll.routings().size()-1);
    for (int i = 0; i < count; i++) {
        auto t = coll.routings().takeLast();
        t->resetTrainHooks();
    }
    endRemoveRows();
}

void RoutingCollectionModel::onHighlightChangedByContext(std::shared_ptr<Routing> routing)
{
    int i= coll.getRoutingIndex(routing);
    auto&& idx=index(i,ColName);
    emit dataChanged(idx,idx,{Qt::CheckStateRole});
}

