#include "routinglistmodel.h"
#include "data/train/routing.h"

RoutingListModel::RoutingListModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int RoutingListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return routings.size();
}

int RoutingListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return ColMAX;
}

QVariant RoutingListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())return {};
    if (role == Qt::DisplayRole) {
        auto r = routings.at(index.row());
        switch (index.column())
        {
        case RoutingListModel::ColName:return r->name();
            break;
        case RoutingListModel::ColReal:return QString::number(r->realCount());
            break;
        case RoutingListModel::ColVirtual:return QString::number(r->virtualCount());
            break;
        case RoutingListModel::ColOrder:return r->orderString();
            break;
        }
    }
    return {};
}

QVariant RoutingListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section)
        {
        case RoutingListModel::ColName: return tr("交路名");
            break;
        case RoutingListModel::ColReal:return tr("实");
            break;
        case RoutingListModel::ColVirtual:return tr("虚");
            break;
        case RoutingListModel::ColOrder:return tr("交路序列");
            break;
        }
    }
    return {};
}

void RoutingListModel::setRoutings(const QList<std::shared_ptr<Routing> > &routings)
{
    beginResetModel();
    this->routings = routings;   //copy assign
    endResetModel();
}
