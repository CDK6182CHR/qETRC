#include "rulermodel.h"
#include <QString>
#include <QVariant>
#include <functional>
#include <utility>

#include "data/rail/rail.h"
#include "model/delegate/qedelegate.h"

RulerModel::RulerModel(std::shared_ptr<Ruler> ruler_, QObject* parent) :
	QStandardItemModel(parent), ruler(ruler_)
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

void RulerModel::setupModel()
{
	if (!ruler) {
		setRowCount(0);
		return;
	}
	updating = true;
	beginResetModel();
	using SI = QStandardItem;
	const auto& rail = ruler->railway();
	setRowCount(rail.stationCount() * 2);

	int row = 0;
	QString spliter = ruler->different() ? QStringLiteral("->") : QStringLiteral("<->");

	for (auto n = ruler->firstDownNode(); n; n = n->nextNodeDiffCirc(), row++) {
		auto& railint = n->railInterval();
		QString s = tr("%1%2%3").arg(railint.fromStationNameLit())
			.arg(spliter).arg(railint.toStationNameLit());

		auto* it = new SI(s);
		setItem(row, ColInterval, it);

		it = new SI;
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

		it = new SI(QString::number(railint.mile(), 'f', 3));
		it->setEditable(false);
		it->setData(railint.mile(), qeutil::DoubleDataRole);
		setItem(row, ColMile, it);

		it = new SI;
		it->setEditable(false);
		if (n->interval) {
			double spd = railint.mile() / n->interval * 3600.0;
			it->setText(QString::number(spd, 'f', 3));
		}
		else {
			it->setText("NA");
		}
		setItem(row, ColSpeed, it);
	}
	setRowCount(row);
	endResetModel();
	updating = false;
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

void RulerModel::actApply()
{

}

