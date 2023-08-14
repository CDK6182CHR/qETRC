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

void PathModel::setupRow(int row, const TrainPathSeg& seg, const StationName& lastStation)
{
	using SI = QStandardItem;
	SI* it;

	bool valid = true;

	// MIND: the status check may be unsafe
	if (seg.railway.expired()) {
		// The railway is INVALID
		valid = false;
		it = new SI(tr("<INVALID RAIL>"));
	}
	else {
		auto rail = seg.railway.lock();
		it = new SI(rail->name());
		QVariant v;
		v.setValue(rail);
		it->setData(v, qeutil::RailwayRole);
	}
	setItem(row, ColRail, it);

	setItem(row, ColStart, new SI(lastStation.toSingleLiteral()));
	setItem(row, ColEnd, new SI(seg.end_station.toSingleLiteral()));
	setItem(row, ColMile, new SI(QString::number(seg.mile, 'f', 3)));
	setItem(row, ColDir, new SI(DirFunc::dirToString(seg.dir)));
	
	if (!valid) {
		// set RED row for invalid data
		for (int c = 0; c < ColMAX; c++) {
			item(row, c)->setForeground(Qt::red);
		}
	}
	else {
		for (int c = 0; c < ColMAX; c++) {
			item(row, c)->setForeground(Qt::black);
		}
	}
	
}


