#include "stationnameregextable.h"
#include "model/general/qemoveablemodel.h"

#include <QTableView>
#include <QHeaderView>

#include "data/common/qesystem.h"

StationNameRegexTable::StationNameRegexTable(QWidget* parent):
	QEControlledTable(parent), model(new QEMoveableModel(this))
{
	initUI();
}

QVector<QRegularExpression> StationNameRegexTable::names() const
{
	QVector<QRegularExpression> res;
	for (int i = 0; i < model->rowCount(); i++) {
		if (auto* it = model->item(i, 0)) {
			if (!it->text().isEmpty()) {
				QRegularExpression re(it->text());
				res.push_back(re);
			}
		}
	}
	return res;
}

void StationNameRegexTable::initUI()
{
	model->setColumnCount(1);
	model->setHorizontalHeaderLabels({ tr("站名正则") });
	
	table()->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
	table()->setEditTriggers(QTableView::AllEditTriggers);
	table()->setModel(model);
	table()->setColumnWidth(0, 300);
}

void StationNameRegexTable::refreshData(const QVector<QRegularExpression>& data)
{
	model->setRowCount(data.size());
	for (int i = 0; i < model->rowCount(); i++) {
		model->setItem(i, new QStandardItem(data.at(i).pattern()));
	}
}

void StationNameRegexTable::clearNames()
{
	refreshData({});
}
