#include "trainlistreadmodel.h"

#include "data/train/train.h"
#include "data/train/traintype.h"

TrainListReadModel::TrainListReadModel(QObject *parent):
    QAbstractTableModel(parent)
{
}

TrainListReadModel::TrainListReadModel(const QList<std::shared_ptr<Train>> &trains, QObject *parent):
    QAbstractTableModel(parent), _trains(trains)
{

}

int TrainListReadModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _trains.size();
}

int TrainListReadModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColMAX;
}

QVariant TrainListReadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    std::shared_ptr<Train> t = _trains.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColTrainName:return t->trainName().full();
        case ColStarting:return t->starting().toSingleLiteral();
        case ColTerminal:return t->terminal().toSingleLiteral();
        case ColType:return t->type()->name();
        case ColMile:return QString::number(t->localMile(), 'f', 3);
        case ColSpeed:return QString::number(t->localTraverseSpeed(), 'f', 3);
        }
    }
    else if (role == Qt::CheckStateRole) {
        if (index.column() == ColShow)
            return t->isShow() ? Qt::Checked : Qt::Unchecked;
    }
    return {};
}

//这里不需要考虑操作撤销之类的问题
void TrainListReadModel::sort(int column, Qt::SortOrder order)
{
    beginResetModel();
    auto& lst = _trains;
    if (order == Qt::AscendingOrder) {
        switch (column) {
        case ColTrainName:std::stable_sort(lst.begin(), lst.end(), &Train::ltName); break;
        case ColStarting:std::stable_sort(lst.begin(), lst.end(), &Train::ltStarting); break;
        case ColTerminal:std::stable_sort(lst.begin(), lst.end(), &Train::ltTerminal); break;
        case ColType:std::stable_sort(lst.begin(), lst.end(), &Train::ltType); break;
        case ColShow:std::stable_sort(lst.begin(), lst.end(), &Train::ltShow); break;
        case ColMile:std::stable_sort(lst.begin(), lst.end(), &Train::ltMile); break;
        case ColSpeed:std::stable_sort(lst.begin(), lst.end(), &Train::ltTravSpeed); break;
        default:break;
        }
    }
    else {
        switch (column) {
        case ColTrainName:std::stable_sort(lst.begin(), lst.end(), &Train::gtName); break;
        case ColStarting:std::stable_sort(lst.begin(), lst.end(), &Train::gtStarting); break;
        case ColTerminal:std::stable_sort(lst.begin(), lst.end(), &Train::gtTerminal); break;
        case ColType:std::stable_sort(lst.begin(), lst.end(), &Train::gtType); break;
        case ColShow:std::stable_sort(lst.begin(), lst.end(), &Train::gtShow); break;
        case ColMile:std::stable_sort(lst.begin(), lst.end(), &Train::gtMile); break;
        case ColSpeed:std::stable_sort(lst.begin(), lst.end(), &Train::gtTravSpeed); break;
        default:break;
        }
    }
    endResetModel();
}

QVariant TrainListReadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole&&orientation==Qt::Horizontal) {
        switch (section) {
        case ColTrainName:return QObject::tr("车次");
        case ColStarting:return QObject::tr("始发站");
        case ColTerminal:return QObject::tr("终到站");
        case ColType:return QObject::tr("类型");
        case ColShow:return QObject::tr("显示");
        case ColMile:return QObject::tr("铺画里程");
        case ColSpeed:return QObject::tr("铺画旅速");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void TrainListReadModel::appendTrain(std::shared_ptr<Train> train)
{
    int row=_trains.size();
    beginInsertRows({},row,row);
    _trains.append(train);
    endInsertRows();
}

void TrainListReadModel::removeTrainAt(int i)
{
    beginRemoveRows({},i,i);
    _trains.removeAt(i);
    endRemoveRows();
}

std::shared_ptr<Train> TrainListReadModel::takeTrainAt(int i)
{
    beginRemoveRows({},i,i);
    auto t=_trains.takeAt(i);
    endRemoveRows();
    return t;
}

void TrainListReadModel::clearTrains()
{
    beginRemoveRows({},0,_trains.size()-1);
    _trains.clear();
    endRemoveRows();
}

void TrainListReadModel::appendTrains(const QList<std::shared_ptr<Train> > trains)
{
    int r=_trains.size();
    beginInsertRows({},r,r+trains.size()-1);
    _trains.append(trains);
    endInsertRows();
}

