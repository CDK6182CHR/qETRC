#include "raildbmodel.h"
#include "raildb.h"


RailDBModel::RailDBModel(std::shared_ptr<RailDB> raildb, QObject *parent) :
    QAbstractItemModel(parent), _raildb(raildb),
    _root(std::make_unique<navi::RailCategoryItem>(raildb,0,nullptr))
{

}

QModelIndex RailDBModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row,column,parent)) return {};
    auto* parentItem=getParentItem(parent);
    auto* childItem=parentItem->child(row);
    if(childItem){
        return createIndex(row,column,childItem);
    }else return {};
}

QModelIndex RailDBModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    pACI childItem = static_cast<pACI>(child.internalPointer());
    pACI parentItem = childItem->parent();

    if (!parentItem || parentItem == _root.get())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int RailDBModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    auto* parentItem = getParentItem(parent);
    if (parentItem)
        return parentItem->childCount();
    return 0;
}

int RailDBModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return navi::AbstractComponentItem::DBColMAX;
}

QVariant RailDBModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const ACI* item = static_cast<ACI*>(index.internalPointer());

    if(item)
        return item->data(index.column());
    return {};
}

QVariant RailDBModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section)
        {
        case navi::AbstractComponentItem::DBColName: return tr("线名");
        case navi::AbstractComponentItem::DBColMile:return tr("里程");
        case navi::AbstractComponentItem::DBColStart:return tr("起点");
        case navi::AbstractComponentItem::DBColEnd:return tr("终点");
        case navi::AbstractComponentItem::DBColAuthor:return tr("作者");
        case navi::AbstractComponentItem::DBColVersion:return tr("版本");
        default:return {};
        }
    }
    return {};
}

RailDBModel::pACI RailDBModel::getItem(const QModelIndex &idx) const
{
    if(idx.isValid())
        return static_cast<pACI>(idx.internalPointer());
    else return nullptr;
}

RailDBModel::pACI RailDBModel::getParentItem(const QModelIndex &parent)const
{
    if(parent.isValid()){
        return static_cast<pACI>(parent.internalPointer());
    }else{
        return _root.get();
    }
}

void RailDBModel::resetModel()
{
    beginResetModel();
    _root=std::make_unique<navi::RailCategoryItem>(_raildb,0,nullptr);
    endResetModel();
}
