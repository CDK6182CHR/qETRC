#include "forbidlistmodel.h"
#include <data/rail/railway.h>
#include <data/rail/forbid.h>


SelectForbidModel::SelectForbidModel(QObject *parent):
    QAbstractListModel(parent)
{

}

std::vector<std::shared_ptr<Forbid> > SelectForbidModel::selectedForbids()
{
    std::vector<std::shared_ptr<Forbid>> res{};
    for (int i=0;i<railway->forbids().size();i++){
        if (_selected.at(i)){
            res.push_back(railway->forbids().at(i));
        }
    }
    return res;
}

void SelectForbidModel::setRailway(std::shared_ptr<Railway> _railway)
{
    beginResetModel();
    this->railway=_railway;
    _selected.resize(railway->forbids().size());
    endResetModel();
}

int SelectForbidModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(railway){
        return railway->forbids().size();
    }else{
        return 0;
    }
}

QVariant SelectForbidModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())return {};
    if(!railway) return {};
    auto forbid=railway->forbids().at(index.row());
    switch (role){
    case Qt::DisplayRole: return forbid->name();
    case Qt::CheckStateRole: return _selected.at(index.row())?Qt::Checked:Qt::Unchecked;
    }
    return {};
}

Qt::ItemFlags SelectForbidModel::flags(const QModelIndex &index) const
{
    auto flag=Base::flags(index);
    if(index.isValid()){
        return flag | Qt::ItemIsUserCheckable;
    }
    return flag;
}

bool SelectForbidModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role==Qt::CheckStateRole){
        _selected.setBit(index.row(),
                         qvariant_cast<Qt::CheckState>(value)==Qt::Checked);
        return true;
    }
    return false;
}
