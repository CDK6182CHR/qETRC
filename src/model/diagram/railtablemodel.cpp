#include "railtablemodel.h"
#include "data/diagram/diagram.h"

RailTableModel::RailTableModel(Diagram& diagram_, QObject* parent):
	QAbstractTableModel(parent),diagram(diagram_)
{
}

int RailTableModel::rowCount(const QModelIndex &) const
{
    return diagram.railwayCount();
}

QVariant RailTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};
	auto t = diagram.railways().at(index.row());
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColName:return t->name();
		case ColFirst:return t->firstStationName().toSingleLiteral();
		case ColLast:return t->lastStationName().toSingleLiteral();
		case ColStations:return QString::number(t->stationCount());
		case ColMile:return QString::number(t->totalMile(), 'f', 3);
		}
	}
	return {};
}

QVariant RailTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch (section) {
		case ColName:return tr("线名");
		case ColFirst:return tr("起点");
		case ColLast:return tr("终点");
		case ColMile:return tr("里程");
		case ColStations:return tr("站数");
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

RailListModel::RailListModel(const QList<std::shared_ptr<Railway>>& railways, QObject* parent):
	QAbstractTableModel(parent), _railways(railways)
{
}

int RailListModel::rowCount(const QModelIndex&) const
{
	return _railways.size();
}

QVariant RailListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};
	auto t = _railways.at(index.row());
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColName:return t->name();
		case ColFirst:return t->firstStationName().toSingleLiteral();
		case ColLast:return t->lastStationName().toSingleLiteral();
		case ColStations:return QString::number(t->stationCount());
		case ColMile:return QString::number(t->totalMile(), 'f', 3);
		}
	}
	return {};
}

QVariant RailListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch (section) {
		case ColName:return tr("线名");
		case ColFirst:return tr("起点");
		case ColLast:return tr("终点");
		case ColMile:return tr("里程");
		case ColStations:return tr("站数");
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

std::shared_ptr<Railway> RailListModel::takeAt(int row)
{
	beginRemoveRows({}, row, row);
	auto t = _railways.takeAt(row);
	endRemoveRows();
	return t;
}

void RailListModel::append(std::shared_ptr<Railway> railway)
{
	int row = _railways.size();
	beginInsertRows({}, row, row);
	_railways.append(railway);
	endInsertRows();
}

void RailListModel::append(const QList<std::shared_ptr<Railway>>& railways)
{
	int row = _railways.size();
	beginInsertRows({}, row, row + railways.size() - 1);
	_railways.append(railways);
	endInsertRows();
}

void RailListModel::clear()
{
	beginResetModel();
	_railways.clear();
	endResetModel();
}

void RailListModel::setRailways(const QList<std::shared_ptr<Railway>>& lst)
{
	beginResetModel();
	_railways = lst;
	endResetModel();
}

void RailListModel::moveUp(int row)
{
	if (row > 0) {
		std::swap(_railways[row - 1], _railways[row]);
	}
	emit dataChanged(index(row - 1, 0), index(row, ColMAX - 1));
}

void RailListModel::moveDown(int row)
{
	if (row >= 0 && row < _railways.size() - 1) {
		moveUp(row + 1);
	}
}
