#include "rulermodel.h"
#include <QString>
#include <QVariant>
#include <functional>
#include <utility>

#include "data/rail/rail.h"
#include "model/delegate/qedelegate.h"

RulerModel::RulerModel(std::shared_ptr<Ruler> ruler_, QObject* parent) :
	IntervalDataModel(ruler_->railway(), parent), ruler(ruler_)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({
			tr("区间"),tr("分"),tr("秒"),
			tr("起"),tr("停"),tr("距离"),
			tr("均速")
		});
	setupModel();
	connect(this, &QStandardItemModel::dataChanged, this,
		&RulerModel::onDataChanged);
}

void RulerModel::refreshData()
{
	setupModel();
}

std::shared_ptr<Railway> RulerModel::appliedRuler()
{
	auto res = ruler->clone();
	auto nr = res->getRuler(0);
	int row = 0;
	for (auto p = nr->firstDownNode(); p; p = p->nextNodeDiffCirc(),row++) {
		if (row >= rowCount()) {
			qDebug() << "RulerModel::appliedRuler: WARNING: " <<
				"number of rows FEWER than number of intervals. row=" <<
				row << ", interval=" << p->railInterval();
			break;
		}
		p->interval = rowIntervalSecs(row);
		p->start = item(row, ColStart)->data(Qt::EditRole).toInt();
		p->stop = item(row, ColStop)->data(Qt::EditRole).toInt();
	}
	if (row < rowCount()) {
		qDebug() << "RulerModel::appliedRuler: WARNING: " <<
			"number of rows MORE than number of intervals. row=" << row;
	}
	return res;
}

void RulerModel::setupModel()
{
	if (!ruler) {
		setRowCount(0);
		return;
	}
	if (!ruler->different()) {
		qDebug() << "RulerModel::setupModel: WARNING: ruler not `different`: " <<
			ruler->name() << ". It will be set to `different`." << Qt::endl;
		ruler->setDifferent(true);
	}

	IntervalDataModel::setupModel();

	//后面的都可以删掉

	updating = true;
	beginResetModel();
	using SI = QStandardItem;
	const auto& rail = ruler->railway();
	setRowCount(rail.stationCount() * 2);

	int row = 0;

	for (auto n = ruler->firstDownNode(); n; n = n->nextNodeDiffCirc(), row++) {
		auto& railint = n->railInterval();
		QString s = intervalString(railint);

		
	}
	setRowCount(row);
	endResetModel();
	updating = false;
}

void RulerModel::setupRow(int row, std::shared_ptr<RailInterval> railint)
{
	using SI = QStandardItem;
	IntervalDataModel::setupRow(row, railint);
	
	auto n = railint->getRulerNode(ruler);

	auto* it = new SI;
	it->setData(n->interval / 60, Qt::EditRole);
	setItem(row, ColMinute, it);

	it = new SI;
	it->setData(n->interval % 60, Qt::EditRole);
	setItem(row, ColSeconds, it);

	it = new SI;
	it->setData(n->start, Qt::EditRole);
	setItem(row, ColStart, it);

	it = new SI;
	it->setData(n->stop, Qt::EditRole);
	setItem(row, ColStop, it);

	it = new SI(QString::number(railint->mile(), 'f', 3));
	it->setEditable(false);
	it->setData(railint->mile(), qeutil::DoubleDataRole);
	setItem(row, ColMile, it);

	it = new SI;
	it->setEditable(false);
	if (n->interval) {
		double spd = railint->mile() / n->interval * 3600.0;
		it->setText(QString::number(spd, 'f', 3));
	}
	else {
		it->setText("NA");
	}
	setItem(row, ColSpeed, it);
}

int RulerModel::rowIntervalSecs(int row) const
{
	return item(row, ColMinute)->data(Qt::EditRole).toInt() * 60 +
		item(row, ColSeconds)->data(Qt::EditRole).toInt();
}

QString RulerModel::intervalString(const RailInterval& railint) const
{
	return tr("%1->%3").arg(railint.fromStationNameLit())
		.arg(railint.toStationNameLit());
}

void RulerModel::copyRowData(int from, int to)
{
	item(to, ColMinute)->setData(item(from, ColMinute)->data(Qt::EditRole), Qt::EditRole);
	item(to, ColSeconds)->setData(item(from, ColSeconds)->data(Qt::EditRole), Qt::EditRole);
	item(to, ColStart)->setData(item(from, ColStart)->data(Qt::EditRole), Qt::EditRole);
	item(to, ColStop)->setData(item(from, ColStop)->data(Qt::EditRole), Qt::EditRole);
}


void RulerModel::actCancel()
{
	setupModel();
}


void RulerModel::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	if (updating)
		return;
	if (std::max(static_cast<int>(ColMinute), topLeft.column()) <=
		std::min(static_cast<int>(ColSeconds), bottomRight.column())) {
		for (int r = topLeft.row(); r <= bottomRight.row(); r++) {
			int interv = item(r, ColMinute)->data(Qt::EditRole).toInt() * 60 +
				item(r, ColSeconds)->data(Qt::EditRole).toInt();
			if (interv) {
				double m = item(r, ColMile)->data(qeutil::DoubleDataRole).toDouble();
				double spd = m / interv * 3600.0;
				item(r, ColSpeed)->setText(QString::number(spd, 'f', 3));
			}
			else {
				item(r, ColSpeed)->setText("NA");
			}
		}
	}
}



