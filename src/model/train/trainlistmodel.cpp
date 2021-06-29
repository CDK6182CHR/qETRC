#include "trainlistmodel.h"



TrainListModel::TrainListModel(TrainCollection& collection, QObject* parent):
	QAbstractTableModel(parent), coll(collection)
{
}

int TrainListModel::rowCount(const QModelIndex& parent) const
{
	return coll.size();
}

int TrainListModel::columnCount(const QModelIndex& parent) const
{
	return MAX_COLUMNS;
}

QVariant TrainListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};
	std::shared_ptr<const Train> t = coll.trainAt(index.row());
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColTrainName:return t->trainName().full();
		case ColStarting:return t->starting().toSingleLiteral();
		case ColTerminal:return t->terminal().toSingleLiteral();
		case ColType:return t->type()->name();
		//todo: 具体实现
		case ColMile:return 0;
		case ColSpeed:return 0;
		}
	}
	else if (role == Qt::CheckStateRole) {
		if (index.column() == ColShow)
			return t->isShow() ? Qt::Checked : Qt::Unchecked;
	}
	return {};
}

bool TrainListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;
	auto t = coll.trainAt(index.row());
	if (role == Qt::CheckStateRole) {
		if (index.column() == ColShow) {
			Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
			bool show = (state == Qt::Checked);
			if (show != t->isShow()) {
				t->setIsShow(show);
				emit trainShowChanged(t, show);
				return true;
			}
		}
	}
	return false;
}

Qt::ItemFlags TrainListModel::flags(const QModelIndex& index) const
{
	switch (index.column()) {
	case ColShow:return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
	}
	return QAbstractTableModel::flags(index);
}

QVariant TrainListModel::headerData(int section, Qt::Orientation orientation, int role) const
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


