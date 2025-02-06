#include "pathlistreadmodel.h"
#include "data/trainpath/trainpath.h"

PathListReadModel::PathListReadModel(QObject *parent)
    : QAbstractTableModel{parent}
{

}

void PathListReadModel::resetData(std::vector<TrainPath *> &&paths)
{
    beginResetModel();
    _paths=std::move(paths);
    endResetModel();
}

void PathListReadModel::resetData(const std::vector<TrainPath *> &paths)
{
    beginResetModel();
    _paths=paths;
    endResetModel();
}

int PathListReadModel::rowCount([[maybe_unused]] const QModelIndex &parent) const
{
    return _paths.size();
}

int PathListReadModel::columnCount([[maybe_unused]] const QModelIndex &parent) const
{
    return ColMAX;
}

QVariant PathListReadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};
    auto* path=_paths.at(index.row());
    if (role == Qt::DisplayRole){
        switch (index.column()){
        case ColName: return path->name();
        case ColStart: return path->startStation().toSingleLiteral();
        case ColEnd: return path->empty()?"":path->endStation().toSingleLiteral();
        }
    }else if(role == Qt::CheckStateRole){
        switch (index.column()){
        case ColValid: return path->valid()?Qt::Checked:Qt::Unchecked;
        }
    }
    return {};
}

QVariant PathListReadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Horizontal && role == Qt::DisplayRole){
        switch (section){
        case ColName: return tr("名称");
        case ColValid: return tr("可用");
        case ColStart: return tr("起点");
        case ColEnd: return tr("终点");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}
