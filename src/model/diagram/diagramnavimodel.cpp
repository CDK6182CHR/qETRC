#include "diagramnavimodel.h"


DiagramNaviModel::DiagramNaviModel(Diagram& diagram, QObject* parent):
    QAbstractItemModel(parent),_diagram(diagram),
    _root(std::make_unique<navi::DiagramItem>(diagram))
{
}

QModelIndex DiagramNaviModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    pACI parentItem;
    if (!parent.isValid())
        parentItem = _root.get();
    else
        parentItem = static_cast<pACI>(parent.internalPointer());
    auto* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return {};
}

QModelIndex DiagramNaviModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    pACI childItem = static_cast<pACI>(child.internalPointer());
    pACI parentItem = childItem->parent();

    if (!parentItem || parentItem == _root.get())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DiagramNaviModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;
    auto* parentItem = getItem(parent);
    if (parentItem)
        return parentItem->childCount();
    return 0;
}

int DiagramNaviModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return ACI::ColMaxNumber;
}

QVariant DiagramNaviModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const ACI* item = static_cast<ACI*>(index.internalPointer());

    //临时实现..
    if(item)
        return item->data(index.column());
    return {};
}

QVariant DiagramNaviModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole&&orientation==Qt::Horizontal) {
        switch (section) {
        case ACI::ColItemName:return QObject::tr("项目");
        case ACI::ColItemNumber:return QObject::tr("数量");
        case ACI::ColDescription:return QObject::tr("概要");
        }
    }
    return {};
}

void DiagramNaviModel::resetModel()
{
    beginResetModel();
    _root = std::make_unique<navi::DiagramItem>(_diagram);
    endResetModel();
}

void DiagramNaviModel::importRailways(const QList<std::shared_ptr<Railway>> rails)
{
    auto idx = index(navi::DiagramItem::RowRailways, 0);
    int cnt = _diagram.railways().size();
    beginInsertRows(idx, cnt, cnt + rails.size() - 1);
    navi::RailwayListItem* it = static_cast<navi::RailwayListItem*>(idx.internalPointer());
    it->appendRailways(rails);
    endInsertRows();
}

void DiagramNaviModel::removeTailRailways(int cnt)
{
    int tot = _diagram.railways().size();
    auto idx = index(navi::DiagramItem::RowRailways, 0);
    navi::RailwayListItem* it = static_cast<navi::RailwayListItem*>(idx.internalPointer());
    beginRemoveRows(idx, tot - cnt, tot - 1);
    it->removeTailRailways(cnt);
    endRemoveRows();
}


navi::AbstractComponentItem* DiagramNaviModel::getItem(const QModelIndex& idx) const
{
    pACI parentItem;
    if (!idx.isValid())
        parentItem = _root.get();
    else
        parentItem = static_cast<pACI>(idx.internalPointer());
    return parentItem;
}

void DiagramNaviModel::resetTrainList()
{
    beginResetModel();
    auto idx = index(navi::DiagramItem::RowTrains, 0);
    auto item = static_cast<navi::TrainListItem*>(getItem(idx));
    item->resetChildren();
    endResetModel();
}

//void DiagramNaviModel::resetRailwayList()
//{
//    beginResetModel();
//    auto idx = index(navi::DiagramItem::RowRailways, 0);
//}


void DiagramNaviModel::resetPageList()
{
    beginResetModel();
    auto idx = index(navi::DiagramItem::RowPages, 0);
    auto item = static_cast<navi::PageListItem*>(getItem(idx));
    item->resetChildren();
    endResetModel();
}

void DiagramNaviModel::insertPage(std::shared_ptr<DiagramPage> page, int idx)
{
    QModelIndex idx0 = index(navi::DiagramItem::RowPages, 0);
    auto* item = static_cast<navi::PageListItem*>(getItem(idx0));
    beginInsertRows(idx0, idx, idx);
    item->insertPage(page, idx);
    endInsertRows();
}

void DiagramNaviModel::removePageAt(int idx)
{
    QModelIndex idx0 = index(navi::DiagramItem::RowPages, 0);
    auto* item = static_cast<navi::PageListItem*>(getItem(idx0));
    beginRemoveRows(idx0, idx, idx);
    item->removePageAt(idx);
    endRemoveColumns();
}

void DiagramNaviModel::onPageNameChanged(int i)
{
    QModelIndex idx0 = index(navi::DiagramItem::RowPages, 0);
    QModelIndex idx1 = index(i, 0, idx0);
    emit dataChanged(idx1, idx1);
}

void DiagramNaviModel::onRailNameChanged(std::shared_ptr<Railway> rail)
{
    int i = _diagram.railCategory().getRailwayIndex(rail);
    QModelIndex idx0 = index(navi::DiagramItem::RowRailways, 0);
    QModelIndex idx1 = index(i, 0, idx0);
    emit dataChanged(idx1, idx1);
}

void DiagramNaviModel::commitAddRailway(std::shared_ptr<Railway> rail)
{
    auto idx = index(navi::DiagramItem::RowRailways, 0);
    int cnt = _diagram.railways().size();
    beginInsertRows(idx, cnt, cnt);
    navi::RailwayListItem* it = static_cast<navi::RailwayListItem*>(idx.internalPointer());
    it->appendRailway(rail);
    endInsertRows();
    emit newRailwayAdded(rail);
}

void DiagramNaviModel::undoAddRailway()
{
    auto rail = _diagram.railways().last();
    removeTailRailways(1);
    emit undoneAddRailway(rail);
}

void DiagramNaviModel::commitAddTrain(std::shared_ptr<Train> train)
{
    QModelIndex par = index(navi::DiagramItem::RowTrains, 0);
    auto* item = static_cast<navi::TrainListItem*>(par.internalPointer());
    int r = _diagram.trainCollection().trainCount();
    beginInsertRows(par, r, r);
    item->addNewTrain(train);
    endInsertRows();
    emit newTrainAdded(train);
}

void DiagramNaviModel::undoAddTrain()
{
    QModelIndex par = index(navi::DiagramItem::RowTrains, 0);
    auto* item = static_cast<navi::TrainListItem*>(par.internalPointer());
    int r = _diagram.trainCollection().trainCount() - 1;
    auto t = _diagram.trainCollection().trains().last();
    beginRemoveRows(par, r, r);
    item->undoAddNewTrain();
    endRemoveRows();
    emit undoneAddTrain(t);
}

void DiagramNaviModel::onTrainDataChanged(const QModelIndex& topleft, const QModelIndex& botright)
{
    QModelIndex par = index(navi::DiagramItem::RowTrains, 0);
    emit dataChanged(index(topleft.row(), 0, par),
        index(botright.row(), navi::DiagramItem::ColMaxNumber, par));
}

void DiagramNaviModel::commitRemoveSingleTrain(int i)
{
    QModelIndex par = index(navi::DiagramItem::RowTrains, 0);
    auto train = diagram().trainCollection().trainAt(i);
    auto* item = static_cast<navi::TrainListItem*>(par.internalPointer());
    beginRemoveRows(par, i, i);
    item->removeTrainAt(i);
    endRemoveRows();
    emit trainRemoved(train);
}

void DiagramNaviModel::undoRemoveSingleTrain(int i, std::shared_ptr<Train> train)
{
    QModelIndex par = index(navi::DiagramItem::RowTrains, 0);
    auto* item = static_cast<navi::TrainListItem*>(par.internalPointer());
    beginInsertRows(par, i, i);
    item->undoRemoveTrainAt(train, i);
    endInsertRows();
    emit undoneTrainRemove(train);
}

void DiagramNaviModel::removeRailwayAt(int i)
{
    QModelIndex par = index(navi::DiagramItem::RowRailways, 0);
    auto* item = static_cast<navi::RailwayListItem*>(par.internalPointer());
    auto rail = _diagram.railwayAt(i);
    beginRemoveRows(par, i, i);
    item->removeRailwayAt(i);
    endRemoveRows();
    emit railwayRemoved(rail);
}



