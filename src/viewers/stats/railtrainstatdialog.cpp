#include "railtrainstatdialog.h"

#include <algorithm>

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFormLayout>

#include "data/train/traincollection.h"
#include "data/diagram/diagramoptions.h"
#include "data/rail/railway.h"
#include "data/diagram/trainadapter.h"
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "data/common/qesystem.h"

#include "model/delegate/generaldoublespindelegate.h"
#include "model/delegate/timeintervaldelegate.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"
#include "editors/train/trainfilterselector.h"
#include "data/train/trainfilterselectorcore.h"


RailTrainStatModel::RailTrainStatModel(std::shared_ptr<Railway> railway, const TrainFilterSelectorCore& filter_core, 
	TrainCollection& coll,
	DiagramOptions& options, QObject* parent):
	QStandardItemModel(parent),
	m_railway(railway), m_filter_core(filter_core), m_coll(coll), m_options(options)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({
		tr("车次"), tr("始发"), tr("终到"), tr("类型"),
		tr("运行线数"), tr("铺画站数"), tr("本线里程"),
		tr("本线总时长"), tr("运行时长"),
		tr("停站时长"), tr("本线旅速"),
		tr("本线技速")
		});
}

void RailTrainStatModel::refreshData()
{
	setupModelData();
}

void RailTrainStatModel::setupModelData()
{
	using SI = QStandardItem;
	setRowCount(m_coll.trains().size());

	int row = 0;
	foreach(auto t, m_coll.trains()) {
		if (!m_filter_core.filter()->check(t)) {
			continue;   // Trail not selected by the filter, pass
		}

		auto adp = t->adapterFor(*m_railway);
		if (!adp) {
			// Not painted on this railway; skip
			continue; 
		}

		double mile = adp->totalMile();
		if (mile < m_min_mile) {
			continue;   // Not enough mile, skip
		}

		setItem(row, ColTrainName, new SI(t->trainName().full()));
		setItem(row, ColStarting, new SI(t->starting().toSingleLiteral()));
		setItem(row, ColTerminal, new SI(t->terminal().toSingleLiteral()));
		setItem(row, ColType, new SI(t->type()->name()));

		auto* it = new SI;
		it->setData(adp->lines().size(), Qt::EditRole);
		setItem(row, ColLineCount, it);

		it = new SI;
		int station_count = std::accumulate(adp->lines().begin(), adp->lines().end(), 0,
			[](int v, const std::shared_ptr<TrainLine>& l) {
				return v + static_cast<int>(l->stations().size());
			});
		it->setData(station_count, Qt::EditRole);
		setItem(row, ColStationCount, it);

		it = new SI;
		it->setData(mile, Qt::EditRole);
		setItem(row, ColMile, it);

		it = new SI;
		auto [run_secs, stay_secs] = adp->runStaySecs(m_options.period_hours);
		it->setData(run_secs + stay_secs, Qt::EditRole);
		setItem(row, ColTotalTime, it);

		it = new SI;
		it->setData(run_secs, Qt::EditRole);
		setItem(row, ColRunningTime, it);

		it = new SI;
		it->setData(stay_secs, Qt::EditRole);
		setItem(row, ColStopTime, it);

		it = new SI;
		double trav_speed = mile / (run_secs + stay_secs) * 3600;
		it->setData(trav_speed, Qt::EditRole);
		setItem(row, ColTravSpeed, it);

		it = new SI;
		double tech_speed = mile / run_secs * 3600;
		it->setData(tech_speed, Qt::EditRole);
		setItem(row, ColTechSpeed, it);

		row++;
	}

	setRowCount(row);
}

void RailTrainStatModel::setMinMile(double mile)
{
	m_min_mile = mile;
	refreshData();
}

RailTrainStatDialog::RailTrainStatDialog(std::shared_ptr<Railway> railway, TrainCollection& coll,
	DiagramOptions& options, QWidget* parent):
	QDialog(parent), m_railway(railway), m_coll(coll), m_options(options)
{
	setAttribute(Qt::WA_DeleteOnClose);
	initUI();
	refreshData();
}

void RailTrainStatDialog::initUI()
{
	setWindowTitle(tr("列车运行统计 @ %1").arg(m_railway->name()));
	resize(800, 600);

	auto* vlay = new QVBoxLayout(this);
	auto* form = new QFormLayout;

	auto* sp = new QDoubleSpinBox;
	sp->setRange(0, 1000000);
	sp->setDecimals(3);
	sp->setSuffix(tr(" km"));
	m_spMinMile = sp;

	form->addRow(tr("最低运行里程"), sp);

	m_filter = new TrainFilterSelector(m_coll);
	form->addRow(tr("列车筛选器"), m_filter);

	m_model = new RailTrainStatModel(m_railway, m_filter->core(), m_coll, m_options, this);
	connect(m_filter, &TrainFilterSelector::filterChanged,
		this, &RailTrainStatDialog::refreshData);
	connect(m_spMinMile, &QDoubleSpinBox::valueChanged,
		m_model, &RailTrainStatModel::setMinMile);

	vlay->addLayout(form);

	auto* lab = new QLabel(tr("以下显示在当前所选线路存在运行线的列车的运行统计信息。"
		"所有统计信息皆仅限本线运行的数据（不包括其他线路上的运行线的数据）。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* table = new QTableView;
	table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	m_table = table;

	table->setModel(m_model);
	table->setEditTriggers(QTableView::NoEditTriggers);
	table->horizontalHeader()->setSortIndicatorShown(true);
	connect(table->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
		table, &QTableView::sortByColumn);

	// Delegates
	auto* dele_double = new GeneralDoubleSpinDelegate(this);
	table->setItemDelegateForColumn(RailTrainStatModel::ColMile, dele_double);
	table->setItemDelegateForColumn(RailTrainStatModel::ColTravSpeed, dele_double);
	table->setItemDelegateForColumn(RailTrainStatModel::ColTechSpeed, dele_double);

	auto* dele_time = new TimeIntervalDelegateHour(this);
	table->setItemDelegateForColumn(RailTrainStatModel::ColTotalTime, dele_time);
	table->setItemDelegateForColumn(RailTrainStatModel::ColRunningTime, dele_time);
	table->setItemDelegateForColumn(RailTrainStatModel::ColStopTime, dele_time);

	vlay->addWidget(table);

	auto* g = new ButtonGroup<2>({ "导出CSV", "关闭" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this,
		SLOT(actExportCsv()), SLOT(close()));
}

void RailTrainStatDialog::refreshData()
{
	m_model->refreshData();
	m_table->resizeColumnsToContents();
}

void RailTrainStatDialog::actExportCsv()
{
	qeutil::exportTableToCsv(m_model, this, tr("运行统计_%1").arg(m_railway->name()));
}
