#include "pathmodel.h"

#include "data/trainpath/trainpath.h"
#include "model/delegate/qedelegate.h"
#include "data/rail/railway.h"

PathModel::PathModel(QObject* parent):
	QStandardItemModel(parent)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({ tr("线路"), tr("起点"), tr("终点"), tr("行别"), tr("里程") });
}

void PathModel::refreshData()
{
	setRowCount(path->segments().size());

	StationName lastStation = path->startStation();
	for (int row = 0; row < path->segments().size(); row++) {
		const auto& seg = path->segments().at(row);
		
		setupRow(row, seg, lastStation);
		lastStation = seg.end_station;
	}
}

void PathModel::insertEmptyRow(int row)
{
	insertRow(row);
	using SI = QStandardItem;

	setItem(row, ColRail, new SI);

	auto* it = new SI;

	if (row > 0) {
		auto* itformer = item(row - 1, ColEnd);
		it->setText(itformer->text());
		it->setEditable(false);
	}
	setItem(row, ColStart, it);

	setItem(row, ColEnd, new SI);

	it = new SI;
	it->setEditable(false);
	setItem(row, ColDir, it);

	it = new SI;
	it->setEditable(false);
	setItem(row, ColMile, it);
}

std::shared_ptr<Railway> PathModel::getRailwayForRow(int row)
{
	return qvariant_cast<std::shared_ptr<Railway>>(item(row, ColRail)->data(qeutil::RailwayRole));
}

void PathModel::onRailwayChanged(int row)
{
	checkRowValidity(row);
}

void PathModel::onStationChanged(const QModelIndex& idx)
{
	checkRowValidity(idx.row());
	if (idx.column() == ColEnd && idx.row() < rowCount() - 1) {
		item(idx.row() + 1, ColStart)->setData(idx.data(Qt::EditRole), Qt::EditRole);
		checkRowValidity(idx.row() + 1);
	}
}

void PathModel::setupRow(int row, const TrainPathSeg& seg, const StationName& lastStation)
{
	using SI = QStandardItem;
	SI* it;

	bool valid = true;

	// MIND: the status check may be unsafe
	if (seg.railway.expired()) {
		// The railway is INVALID
		valid = false;
		it = new SI(seg.rail_name);
	}
	else {
		auto rail = seg.railway.lock();
		it = new SI(rail->name());
		QVariant v;
		v.setValue(rail);
		it->setData(v, qeutil::RailwayRole);
	}
	setItem(row, ColRail, it);

	it = new SI(lastStation.toSingleLiteral());
	it->setEditable(row == 0);
	setItem(row, ColStart, it);
	setItem(row, ColEnd, new SI(seg.end_station.toSingleLiteral()));

	it = new SI(QString::number(seg.mile, 'f', 3));
	it->setEditable(false);
	it->setData(seg.mile, qeutil::DoubleDataRole);
	setItem(row, ColMile, it);

	it = new SI(DirFunc::dirToString(seg.dir));
	it->setEditable(false);
	QVariant v;
	v.setValue(seg.dir);
	it->setData(v, qeutil::DirectionRole);
	setItem(row, ColDir, it);

	checkRowValidity(row);
	
	//if (!valid) {
	//	// set RED row for invalid data
	//	for (int c = 0; c < ColMAX; c++) {
	//		item(row, c)->setForeground(Qt::red);
	//	}
	//}
	//else {
	//	for (int c = 0; c < ColMAX; c++) {
	//		item(row, c)->setForeground(Qt::black);
	//	}
	//}
	
}

void PathModel::checkRowValidity(int row)
{
	bool valid = false;
	do {
		// (1) railway existence
		auto rail = getRailwayForRow(row);
		if (!rail) {
			qInfo() << "Railway " << item(row, ColRail)->text() << " does not exist";
			item(row, ColRail)->setForeground(Qt::red);
			break;
		}

		// (2) station existence
		StationName start_name{ item(row,ColStart)->text() };
		auto start_st = rail->stationByName(start_name);
		if (!start_st) {
			qInfo() << "start station " << start_name.toSingleLiteral() << " does not belong to railway";
			item(row, ColStart)->setForeground(Qt::red);
			break;
		}

		StationName end_name{ item(row,ColEnd)->text() };
		auto end_st = rail->stationByName(end_name);
		if (!end_st) {
			qInfo() << "end station " << end_name.toSingleLiteral() << " does not belong to railway";
			item(row, ColEnd)->setForeground(Qt::red);
			break;
		}

		// (3) existence of interval
		if (start_st->mile == end_st->mile) {
			qInfo() << "start station and end station have same mile";
			item(row, ColMile)->setData(0.0, Qt::EditRole);
			item(row, ColMile)->setForeground(Qt::red);
			break;
		}

		auto dir = rail->gapDirection(start_st, end_st);
		item(row, ColDir)->setText(DirFunc::dirToString(dir));
		QVariant v;
		v.setValue(dir);
		item(row, ColDir)->setData(v, qeutil::DirectionRole);
		if (!start_st->isDirectionVia(dir) || !end_st->isDirectionVia(dir)) {
			qInfo() << "start or end station does not pass the direction " << DirFunc::dirToString(dir);
			item(row, ColDir)->setForeground(Qt::red);
			break;
		}
		double mile = rail->mileBetween(start_st, end_st);
		item(row, ColMile)->setData(mile, Qt::EditRole);
		valid = true;

	} while (false);

	if (valid) {
		for (int c = 0; c < ColMAX; c++) {
			item(row, c)->setForeground(Qt::black);
		}
	}
}


