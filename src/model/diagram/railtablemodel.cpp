#include "railtablemodel.h"

RailTableModel::RailTableModel(Diagram& diagram_, QObject* parent):
	QAbstractTableModel(parent),diagram(diagram_)
{
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
