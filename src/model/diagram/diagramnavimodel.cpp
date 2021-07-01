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
    auto* parentItem = getParentItem(parent);
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

navi::AbstractComponentItem* DiagramNaviModel::getParentItem(const QModelIndex& parent) const
{
    pACI parentItem;
    if (!parent.isValid())
        parentItem = _root.get();
    else
        parentItem = static_cast<pACI>(parent.internalPointer());
    return parentItem;
}


