#include "pathlistmodel.h"
#include "data/trainpath/trainpathcollection.h"

PathListModel::PathListModel(TrainPathCollection& pathcoll, QObject* parent):
	QAbstractTableModel(parent), pathcoll(pathcoll)
{
}

int PathListModel::rowCount(const QModelIndex& parent) const
{
	return pathcoll.size();
}

int PathListModel::columnCount(const QModelIndex& parent) const
{
	return ColMAX;
}

QVariant PathListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())return {};
	const auto& path = pathcoll.paths().at(index.row());
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColName: return path->name();
		case ColStart: return path->startStation().toSingleLiteral();
		case ColEnd:return path->empty() ? QString() : path->endStation().toSingleLiteral();
		}
	}
	else if (role == Qt::CheckStateRole) {
		switch (index.column()) {
		case ColValid: return path->valid() ? Qt::Checked : Qt::Unchecked;
		}
	}
	return {};
}

QVariant PathListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case ColName: return QObject::tr("名称");
		case ColValid: return QObject::tr("有效");
		case ColStart: return QObject::tr("起点");
		case ColEnd: return QObject::tr("终点");
		}
	}
	return {};
}

void PathListModel::addPathAt(int index, std::unique_ptr<TrainPath>&& path)
{
	beginInsertRows({}, index, index);
	auto itr = pathcoll.paths().begin();
	std::advance(itr, index);
	pathcoll.paths().emplace(itr, std::move(path));
	endInsertRows();
}

std::unique_ptr<TrainPath> PathListModel::popPathAt(int index)
{
	beginRemoveRows({}, index, index);
	auto p = std::move(pathcoll.paths().at(index));
	auto itr = pathcoll.paths().begin();
	std::advance(itr, index);
	pathcoll.paths().erase(itr);
	endRemoveRows();
	return p;
}

void PathListModel::updatePath(TrainPath* path)
{
	int idx = pathcoll.pathIndex(path);
	emit dataChanged(index(idx, ColStart), index(idx, ColMAX - 1), { Qt::DisplayRole });
}

void PathListModel::refreshData()
{
	beginResetModel();
	endResetModel();
}
