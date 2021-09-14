#include "raildbmodel.h"
#include "raildb.h"
#include "data/rail/railway.h"

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

QModelIndex RailDBModel::indexByPath(const std::deque<int> &path)
{
    QModelIndex idx{};
    for(int i:path){
        idx=index(i,0,idx);
    }
    return idx;
}

QModelIndex RailDBModel::railIndexBrute(std::shared_ptr<Railway> railway)
{
    return railIndexBruteFrom(railway,{});
}

RailDBModel::pACI RailDBModel::getParentItem(const QModelIndex &parent)const
{
    if(parent.isValid()){
        return static_cast<pACI>(parent.internalPointer());
    }else{
        return _root.get();
    }
}

QModelIndex RailDBModel::railIndexBruteFrom(std::shared_ptr<Railway> railway,
                                            const QModelIndex &idx)
{
    auto* it=getParentItem(idx);
    for(int i=0;i<it->childCount();i++){
        auto* sub=it->child(i);
        if(!sub) continue;
        if(sub->type()==navi::RailCategoryItem::Type){
            // 递归
            auto subres=railIndexBruteFrom(railway,index(i,0,idx));
            if(subres.isValid()){
                return subres;
            }
        }else if(sub->type()==navi::RailwayItemDB::Type){
            if (static_cast<navi::RailwayItemDB*>(sub)->railway()==railway){
                return index(sub->row(),0,idx);
            }
        }
    }
    return {};
}

std::shared_ptr<Railway> RailDBModel::railwayByIndex(const QModelIndex& idx)
{
    if (!idx.isValid())return nullptr;
    if (auto* it = static_cast<pACI>(idx.internalPointer());
        it->type() == navi::RailwayItemDB::Type) {
        return static_cast<navi::RailwayItemDB*>(it)->railway();
    }
    return nullptr;
}

void RailDBModel::onRailInfoChanged(std::shared_ptr<Railway> railway, const std::deque<int>& path)
{
    auto idx = indexByPath(path);
    if (!idx.isValid() || railwayByIndex(idx)!=railway) {
        qDebug() << "RailDBModel::onRailInfoChanged: WARNING: invalid path to railway: " <<
            railway->name() << ", will use brute-force alg. " << Qt::endl;
        idx = railIndexBrute(railway);
    }
    emit dataChanged(idx, index(idx.row(), ACI::DBColMAX - 1, idx.parent()), 
        { Qt::EditRole });
}

void RailDBModel::resetModel()
{
    beginResetModel();
    _root=std::make_unique<navi::RailCategoryItem>(_raildb,0,nullptr);
    endResetModel();
}
