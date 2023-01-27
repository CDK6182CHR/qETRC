#include "railstationeventlist.h"

#include "data/rail/railway.h"
#include "data/train/traintype.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"

#include "data/common/qesystem.h"
#include "data/diagram/diagram.h"
#include "data/train/routing.h"
#include "editors/train/trainfilterselector.h"
#include "stationtraingapdialog.h"
#include "model/delegate/qedelegate.h"
#include "data/common/qeglobal.h"
#include "util/pagecomboforrail.h"
#include "data/train/train.h"

#include <QFormLayout>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QCheckBox>
#include <QAction>
#include <QScroller>

RailStationEventListModel::RailStationEventListModel(Diagram& diagram, const std::shared_ptr<Railway>& rail, const std::shared_ptr<RailStation>& station, QObject* parent) : QStandardItemModel(parent),
diagram(diagram),
rail(rail),
station(station),
lst(diagram.stationEvents(rail, station))
{
	setColumnCount(ColMAX);
	setupModel();
}


void RailStationEventListModel::setupModel()
{
	setHorizontalHeaderLabels({
		tr("车次"),tr("时间"),tr("事件"),tr("位置"),
		tr("类型"),tr("方向"),tr("始发"),tr("终到"),tr("车底"),
		tr("担当"),tr("备注")
		});
	setRowCount(lst.size());
	using SI = QStandardItem;
	for (int i = 0; i < lst.size(); i++) {
		const auto& p = *(lst.at(i));
		auto line = p.line;
		auto train = line->train();
		const auto& ev = p;
		auto* it = new SI(line->train()->trainName().full());
		QVariant v;
		v.setValue(train);
		it->setData(v, qeutil::TrainRole);
		setItem(i, ColTrainName, it);
		it = new SI(ev.time.toString("hh:mm:ss"));
		it->setData(ev.time, qeutil::TimeDataRole);
		setItem(i, ColTime, it);
		setItem(i, ColEventType, new SI(qeutil::eventTypeString(ev.type)));
		setItem(i, ColPos, new SI(ev.posString()));
		setItem(i, ColTrainType, new SI(line->train()->type()->name()));
		setItem(i, ColDirection, new SI(DirFunc::dirToString(line->dir())));
		setItem(i, ColStarting, new SI(train->starting().toSingleLiteral()));
		setItem(i, ColTerminal, new SI(train->terminal().toSingleLiteral()));
		if (train->hasRouting()) {
			auto rt = train->routing().lock();
			setItem(i, ColModel, new SI(rt->model()));
			setItem(i, ColOwner, new SI(rt->owner()));
		}
		else {
			setItem(i, ColModel, new SI("-"));
			setItem(i, ColOwner, new SI("-"));
		}
		setItem(i, ColNote, new SI(ev.note));
	}
}

std::shared_ptr<const Train> RailStationEventListModel::trainForRow(int row) const
{
	return qvariant_cast<std::shared_ptr<const Train>>(
		item(row, ColTrainName)->data(qeutil::TrainRole));
}

QTime RailStationEventListModel::timeForRow(int row) const
{
	return item(row, ColTime)->data(qeutil::TimeDataRole).toTime();
}

RailStationEventListDialog::RailStationEventListDialog(Diagram& diagram, const std::shared_ptr<Railway>& rail, const std::shared_ptr<RailStation>& station, QWidget* parent) : QDialog(parent),
diagram(diagram),
rail(rail),
station(station),
model(new RailStationEventListModel(diagram, rail, station)),
filter(new TrainFilterSelector(diagram.trainCollection(),this))
{
    setWindowTitle(tr("车站事件表 - %1 @ %2").arg(station->name.toSingleLiteral(),rail->name()));
	resize(800, 800);
	setAttribute(Qt::WA_DeleteOnClose);
	initUI();
}

void RailStationEventListDialog::initUI()
{
	auto* vlay = new QVBoxLayout;
	auto* form = new QFormLayout;

	auto* g = new ButtonGroup<2, QHBoxLayout, QCheckBox>({ "站前事件","站后事件" });
	g->get(0)->setChecked(true); g->get(1)->setChecked(true);
	ckPosPre = g->get(0); ckPosPost = g->get(1);
	g->connectAllTo(SIGNAL(toggled(bool)), this,
		SLOT(onPosShowChanged()));
	g->addWidget(filter);
	connect(filter, &TrainFilterSelector::filterChanged,
		this, &RailStationEventListDialog::onFilterChanged);
	form->addRow(tr("筛选"), g);
	vlay->addLayout(form);

	auto* lab = new QLabel(tr("注：站前事件表示发生在所选车站上行端（里程小端）的事件，"
		"可以理解为从指定车站向小里程端偏移一充分小距离处所能观察到的事件，"
		"例如下行列车到达、上行列车出发；站后事件相应指代下行端的事件。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);
	lab = new QLabel(tr("共有[%1]个相关事件, 筛选后有[%1]个事件").arg(model->rowCount()));
	lbCount = lab;
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	table = new QTableView;
	table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	table->setModel(model);
	table->resizeColumnsToContents();
	table->setEditTriggers(QTableView::NoEditTriggers);
	table->horizontalHeader()->setSortIndicatorShown(true);
    QScroller::grabGesture(table,QScroller::TouchGesture);
	connect(table->horizontalHeader(),
		SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), table,
		SLOT(sortByColumn(int, Qt::SortOrder)));
	vlay->addWidget(table);

	auto* act = new QAction(tr("定位到运行图"), table);
	table->addAction(act);
	table->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(act, &QAction::triggered, this, &RailStationEventListDialog::actLocate);

	auto* h = new ButtonGroup<3>({ "导出CSV","间隔分析" ,"关闭"});
	h->connectAll(SIGNAL(clicked()), this, {
		SLOT(toCsv()),SLOT(gapAnalysis()), SLOT(close())
		});
	vlay->addLayout(h);
	setLayout(vlay);
}

// not used
void RailStationEventListDialog::showTableRows(std::function<bool(int)> pred)
{
	int cnt = 0;
	for (int r = 0; r < model->rowCount(); r++) {
		if (pred(r))
			table->showRow(r);
		if (!table->isRowHidden(r))cnt++;
	}
	updateShownRows(cnt);
}

// not used
void RailStationEventListDialog::hideTableRows(std::function<bool(int)> pred)
{
	int cnt = 0;
	for (int r = 0; r < model->rowCount(); r++) {
		if (pred(r))
			table->hideRow(r);
		if (!table->isRowHidden(r))cnt++;
	}
	updateShownRows(cnt);
}

void RailStationEventListDialog::filtTableRows(std::function<bool(int)> pred)
{
	int cnt = 0;
	for (int r = 0; r < model->rowCount(); r++) {
		table->setRowHidden(r, !pred(r));
		if (!table->isRowHidden(r))cnt++;
	}
	updateShownRows(cnt);
}

void RailStationEventListDialog::updateShownRows(int cnt)
{
	lbCount->setText(tr("共有[%1]个相关事件, 筛选后有[%2]个事件")
		.arg(model->rowCount()).arg(cnt));
}

// not used
void RailStationEventListDialog::onPreShowChanged(bool on)
{
	if (on)
		showTableRows([this](int row) {
		const QString& s = model->item(row,
			RailStationEventListModel::ColPos)->text();
		return s == tr("站前") || s == tr("前后");
			});
	else
		hideTableRows([this](int row) {
		const QString& s = model->item(row,
			RailStationEventListModel::ColPos)->text();
		return s == tr("站前");
			});
}

// not used
void RailStationEventListDialog::onPostShowChanged(bool on)
{
	if (on)
		showTableRows([this](int row) {
		const QString& s = model->item(row,
			RailStationEventListModel::ColPos)->text();
		return s == tr("站后") || s == tr("前后");
			});
	else
		hideTableRows([this](int row) {
		const QString& s = model->item(row,
			RailStationEventListModel::ColPos)->text();
		return s == tr("站后");
			});
}

void RailStationEventListDialog::onPosShowChanged()
{
	filtTableRows([this](int row)->bool {
		const QString& s = model->item(row, RailStationEventListModel::ColPos)
			->text();
		return (ckPosPre->isChecked() && (s == tr("站前") || s == tr("前后"))) ||
			(ckPosPost->isChecked() && (s == tr("站后") || s == tr("前后")));
		});
}

void RailStationEventListDialog::toCsv()
{
	QString s = QObject::tr("%1车站事件表.csv").arg(station->name.toSingleLiteral());
	qeutil::exportTableToCsv(model, this, s);
}



void RailStationEventListDialog::gapAnalysis()
{
	auto* dialog = new StationTrainGapDialog(diagram, rail, station, model->getData(),
		*filter->filter(), this);
	connect(dialog, &StationTrainGapDialog::locateToEvent,
		this, &RailStationEventListDialog::locateOnEvent);
	dialog->show();
}

void RailStationEventListDialog::onFilterChanged(const TrainFilterCore* core)
{
	filtTableRows([core,this](int row)->bool {
		return core->check(model->trainForRow(row));
		});
}

void RailStationEventListDialog::actLocate()
{
	auto&& idx = table->currentIndex();
	if (!idx.isValid())return;
	auto time = model->timeForRow(idx.row());
	int i = PageComboForRail::dlgGetPageIndex(diagram, rail, this, tr("选择运行图"),
		tr("请选择要定位到的运行图窗口名："));
	if (i == -1)return;
	emit locateOnEvent(i, rail, station, time);
}

