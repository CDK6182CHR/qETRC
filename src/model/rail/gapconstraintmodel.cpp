#include "gapconstraintmodel.h"

#include <data/gapset/gapsetabstract.h>

GapConstraintModel::GapConstraintModel(QObject *parent) :
    QAbstractTableModel(parent)
{

}

int GapConstraintModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _gapSet ? _gapSet->size():0;
}

int GapConstraintModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColMAX;
}

QVariant GapConstraintModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return {};
    const auto& t=_gapSet->at(index.row());

    if(role==Qt::DisplayRole || role==Qt::EditRole){
        switch (index.column()){
        case ColName: return t->name();
        case ColLimit: return t->limit();
        }
    }else if(role==Qt::ToolTipRole){
        switch(index.column()){
        case ColLimit: return t->description();
        }
    }
    return {};
}

bool GapConstraintModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid()) return false;
    const auto& t=_gapSet->at(index.row());
    if(role==Qt::EditRole){
        switch(index.column()){
        case ColLimit: t->setLimit(value.toInt()); return true;
        }
    }
    return false;
}

QVariant GapConstraintModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation==Qt::Horizontal && role==Qt::DisplayRole){
        switch (section){
        case ColName: return QObject::tr("间隔组");
        case ColLimit: return QObject::tr("最小间隔");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags GapConstraintModel::flags(const QModelIndex &index) const
{
    auto flag=QAbstractTableModel::flags(index);
    if(index.isValid()){
        switch (index.column()){
        case ColLimit: return flag | Qt::ItemIsEditable;
        }
    }
    return flag;
}

#if 0
void GapConstraintModel::setSingleLine(bool singleLine)
{
    beginResetModel();
    _gapSet->setSingleLineAndBuild(singleLine);
    endResetModel();
}
#endif

void GapConstraintModel::setGapSet(gapset::GapSetAbstract *gapSet)
{
    beginResetModel();
    _gapSet=gapSet;
    _gapSet->buildSet();
    //_gapSet->setSingleLineAndBuild(singleLine);
    endResetModel();
}

void GapConstraintModel::setConstrainFromCurrent(const std::map<TrainGapTypePair, int>& mingap,
    int minSecs, int maxSecs)
{
    beginResetModel();
    _gapSet->setConstraintFromMinimal(mingap, minSecs, maxSecs);
    endResetModel();
}

