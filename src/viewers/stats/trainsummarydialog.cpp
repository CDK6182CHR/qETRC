#include "trainsummarydialog.h"

#include <chrono>

#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFileDialog>

#include "data/train/train.h"
#include "data/train/traintype.h"
#include "data/train/traincollection.h"
#include "data/train/routing.h"
#include "data/diagram/diagramoptions.h"
#include "data/analysis/runstat/trainintervalstat.h"
#include "data/common/qesystem.h"
#include "editors/train/trainfilterselector.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"
#include "model/delegate/generaldoublespindelegate.h"
#include "model/delegate/timeintervaldelegate.h"


TrainInfoSummaryModel::TrainInfoSummaryModel(const DiagramOptions& ops, TrainCollection& coll, 
	const TrainFilterSelectorCore& filterCore, QObject* parent):
	QStandardItemModel(parent), m_ops(ops), m_coll(coll),
	m_filterCore(filterCore)
{
	setHorizontalHeaderLabels({
		tr("车次"), tr("始发"),tr("终到"), tr("类型"),
		tr("铺画里程"), tr("铺画时间"), tr("铺画旅速"), tr("铺画技速"),
		tr("总时间"), tr("总运行时间"), tr("总停站时间"), tr("总里程"), tr("总旅速"), tr("总技速"),
		tr("交路"), tr("车底"), tr("担当"),
		});
}

void TrainInfoSummaryModel::setupModel()
{
	using namespace std::chrono_literals;
	auto tm_start = std::chrono::steady_clock::now();
	using SI = QStandardItem;
	setRowCount(m_coll.trainCount());

	int r = 0;
	for (int i = 0; i < m_coll.trainCount(); i++) {
		auto train = m_coll.trainAt(i);

		if (!m_filterCore.filter()->check(train))
			continue;

		setItem(r, ColTrainName, new SI(train->trainName().full()));
		setItem(r, ColStarting, new SI(train->starting().toSingleLiteral()));
		setItem(r, ColTerminal, new SI(train->terminal().toSingleLiteral()));
		setItem(r, ColType, new SI(train->type()->name()));

		// 铺画里程/时间
		double loc_mile = train->localMile();
		auto* it = new SI;
		it->setData(loc_mile, Qt::EditRole);
		setItem(r, ColPaintedMile, it);

		auto [loc_run_secs, loc_stay_secs] = train->localRunStaySecs(m_ops.period_hours);
		int loc_tot_secs = loc_run_secs + loc_stay_secs;
		it = new SI;
		it->setData(loc_tot_secs, Qt::EditRole);
		setItem(r, ColPaintedTime, it);

		double loc_trav_speed = loc_mile / (loc_tot_secs / 3600.0);
		double loc_tech_speed = loc_mile / (loc_run_secs / 3600.0);
		it = new SI;
		it->setData(loc_trav_speed, Qt::EditRole);
		setItem(r, ColPaintedTravSpeed, it);

		it = new SI;
		it->setData(loc_tech_speed, Qt::EditRole);
		setItem(r, ColPaintedTechSpeed, it);

		// 总里程/时间  需要图论算法  其时间复杂度能否接受有待检验
		TrainIntervalStat stat(m_ops, train);
		auto res = stat.compute();

		it = new SI;
		it->setData(res.totalSecs, Qt::EditRole);
		setItem(r, ColTotalTime, it);

		it = new SI;
		it->setData(res.runSecs, Qt::EditRole);
		setItem(r, ColTotalRunTime, it);

		it = new SI;
		it->setData(res.stopSecs, Qt::EditRole);
		setItem(r, ColTotalStayTime, it);

		if (res.railResults.isValid) {
			it = new SI;
			it->setData(res.railResults.totalMiles, Qt::EditRole);
			setItem(r, ColTotalMile, it);
			double total_trav_speed = res.railResults.totalMiles / (res.totalSecs / 3600.0);
			double total_tech_speed = res.railResults.totalMiles / (res.runSecs / 3600.0);
			it = new SI;
			it->setData(total_trav_speed, Qt::EditRole);
			setItem(r, ColTotalTravSpeed, it);
			it = new SI;
			it->setData(total_tech_speed, Qt::EditRole);
			setItem(r, ColTotalTechSpeed, it);
		}
		else {
			setItem(r, ColTotalMile, new SI);
			setItem(r, ColTotalTravSpeed, new SI);
			setItem(r, ColTotalTechSpeed, new SI);
		}

		// Routing data, if available
		if (train->hasRouting()) {
			auto routing = train->routing().lock();
			setItem(r, ColRouting, new SI(routing->name()));
			setItem(r, ColCarriage, new SI(routing->model()));
			setItem(r, ColOwner, new SI(routing->owner()));
		}
		else {
			setItem(r, ColRouting, new SI);
			setItem(r, ColCarriage, new SI);
			setItem(r, ColOwner, new SI);
		}
		r++;
	}
	setRowCount(r);
	auto tm_end = std::chrono::steady_clock::now();
	qInfo() << "TrainInfoSummaryModel::setupModel() completed in "
		<< (tm_end - tm_start) / 1.0ms << " ms";
}

void TrainInfoSummaryModel::refreshData()
{
	setupModel();
}

TrainSummaryDialog::TrainSummaryDialog(const DiagramOptions& m_ops, TrainCollection& coll, QWidget* parent):
	QDialog(parent), m_ops(m_ops), m_coll(coll),
	m_filter(new TrainFilterSelector(m_coll)),
	m_model(new TrainInfoSummaryModel(m_ops, m_coll, m_filter->core(), this))
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("列车信息汇总"));
	initUI();
	refreshData();
}

void TrainSummaryDialog::initUI()
{
	resize(1000, 800);
	auto* vlay = new QVBoxLayout(this);

	vlay->addWidget(m_filter);
	m_table = new QTableView(this);
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
	m_table->setModel(m_model);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	//m_table->horizontalHeader()->setStretchLastSection(true);

	connect(m_table->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
		m_table, &QTableView::sortByColumn);
	m_table->horizontalHeader()->setSortIndicatorShown(true);

	{
		int c = 0;
		for (int w : {120, 100, 100, 40, 60, 60, 60, 60,
			60, 60, 60, 60, 60, 60, 60, 60, 60}) {
			m_table->setColumnWidth(c++, w);
		}
	}

	auto* dele_double = new GeneralDoubleSpinDelegate(this, 3);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColPaintedMile, dele_double);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColPaintedTechSpeed, dele_double);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColPaintedTravSpeed, dele_double);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColTotalMile, dele_double);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColTotalTechSpeed, dele_double);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColTotalTravSpeed, dele_double);

	auto* dele_time = new TimeIntervalDelegateHour(this);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColPaintedTime, dele_time);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColTotalTime, dele_time);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColTotalRunTime, dele_time);
	m_table->setItemDelegateForColumn(TrainInfoSummaryModel::ColTotalStayTime, dele_time);

	vlay->addWidget(m_table);

	connect(m_filter, &TrainFilterSelector::filterChanged, this, &TrainSummaryDialog::refreshData);

	auto* g = new ButtonGroup<3>({ "刷新","导出CSV","关闭" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this, { SLOT(refreshData()), SLOT(exportCsv()), SLOT(close()) });
}

void TrainSummaryDialog::exportCsv()
{
	qeutil::exportTableToCsv(m_model, m_table, this, tr("列车统计信息"));
}

void TrainSummaryDialog::refreshData()
{
	m_model->refreshData();
	//m_table->resizeColumnsToContents();
}
