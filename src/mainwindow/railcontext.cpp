#ifndef QETRC_MOBILE_2
#include "data/rail/forbid.h"
#include "data/rail/ruler.h"
#include "railcontext.h"
#include "mainwindow.h"
#include "rulercontext.h"
#include "traincontext.h"

#include "viewers/sectioncountdialog.h"
#include "dialogs/selectrailstationdialog.h"
#include "dialogs/jointrailwaydialog.h"
#include "viewers/events/stationtimetablesettled.h"
#include "viewers/events/railstationeventlist.h"
#include "viewers/events/railsectionevents.h"
#include "viewers/events/railsnapevents.h"
#include "viewers/railtopotable.h"
#include "editors/forbidwidget.h"
#include "editors/railstationwidget.h"
#include "data/diagram/diagrampage.h"

#include <QStyle>
#include <QLabel>
#include <QApplication>
#include <DockManager.h>
#include <QMessageBox>
#include <QInputDialog>
#include "model/diagram/diagramnavimodel.h"
#include <SARibbonLineEdit.h>
#include <SARibbonComboBox.h>
#include <SARibbonMenu.h>
#include "editors/ruler/rulerwidget.h"
#include "model/rail/rulermodel.h"
#include "viewers/events/stationtraingapdialog.h"
#include "viewers/events/traingapstatdialog.h"
#include "viewers/events/railtrackwidget.h"
#include "navi/navitree.h"
#include "mainwindow/pagecontext.h"
#include "navi/addpagedialog.h"


RailContext::RailContext(Diagram& diagram_, SARibbonContextCategory* context, 
	MainWindow* mw_, QObject* parent):
	QObject(parent),diagram(diagram_),cont(context),mw(mw_)
{
	initUI();
}

void RailContext::resetRailway()
{
	railway.reset();
	refreshData();
}

void RailContext::refreshData()
{
	if (!railway) {
		return;
	}
	updating = true;
	edName->setText(railway->name());

	cbRulers->clear();
	cbRulers->addItem(tr("(按里程)"));
	for (auto r : railway->rulers()) {
		cbRulers->addItem(r->name());
	}
	cbRulers->setCurrentIndex(railway->ordinateIndex() + 1);

	updating = false;
}

void RailContext::refreshAllData()
{
	refreshData();
	for (auto p : rulerWidgets) {
		p->refreshData();
	}
}

void RailContext::updateForbidDiagrams(std::shared_ptr<Forbid> forbid, Direction dir)
{
	foreach(auto p, mw->diagramWidgets) {
		if (p->page()->containsRailway(forbid->railway())) {
			p->updateForbid(forbid, dir);
		}
	}
}

void RailContext::initUI()
{
	auto* page = cont->addCategoryPage(tr("线路工具(&8)"));

	auto* panel = page->addPannel(tr(""));

	if constexpr (true) {
		auto* ed = new SARibbonLineEdit;
		edName = ed;

		QWidget* w = new QWidget;
		auto* vlay = new QVBoxLayout;

		auto* lab = new QLabel(tr("当前线路"));
		lab->setAlignment(Qt::AlignCenter);
		vlay->addWidget(lab);

		ed->setFocusPolicy(Qt::NoFocus);
		ed->setAlignment(Qt::AlignCenter);
		ed->setFixedWidth(100);

		vlay->addWidget(ed);
		w->setLayout(vlay);
		panel->addWidget(w, SARibbonPannelItem::Large);
	}

	panel = page->addPannel(tr("纵坐标标尺"));

	if constexpr (true) {
		auto* w = new QWidget;
		auto* vlay = new QVBoxLayout;

		auto* cb = new SARibbonComboBox;
		cbRulers = cb;
		cb->setEditable(false);
		cb->setMinimumWidth(120);
		cb->setMinimumHeight(25);
		connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(actChangeOrdinate(int)));

		//panel->addWidget(cb, SARibbonPannelItem::Large);

		auto* lab = new QLabel(tr("纵坐标标尺"));
		lab->setAlignment(Qt::AlignCenter);

		vlay->addWidget(lab);
		vlay->addWidget(cb);
		w->setLayout(vlay);

		panel->addWidget(w, SARibbonPannelItem::Large);
	}

	panel = page->addPannel(tr("编辑"));

	QAction* act;
	act = new QAction(QIcon(":/icons/rail.png"), tr("基线编辑"), this);
	panel->addLargeAction(act);
	connect(act, SIGNAL(triggered()), this, SLOT(actOpenStationWidget()));

	act = new QAction(QIcon(":/icons/forbid.png"), tr("天窗编辑"), this);
	act->setToolTip(tr("天窗编辑\n打开或切换到到当前线路的天窗编辑面板"));
	connect(act, SIGNAL(triggered()), this, SLOT(actOpenForbidWidget()));
	act->setMenu(mw->forbidMenu);
	panel->addLargeAction(act);

	panel = page->addPannel(tr("标尺"));
	act = new QAction(QIcon(":/icons/ruler.png"), tr("标尺"), this);
    auto* me = new SARibbonMenu(mw);
	act->setToolTip(tr("标尺编辑\n创建或者导航到本线标尺编辑的面板"));
	meRulerWidgets = me;
	act->setMenu(me);
	connect(act, SIGNAL(triggered()), this, SLOT(actSelectRuler()));
	panel->addLargeAction(act);

	act = new QAction(QIcon(":/icons/add.png"), tr("新建标尺"), this);
	act->setToolTip(tr("新建标尺\n新建本线的空白标尺"));
	connect(act, SIGNAL(triggered()), this, SLOT(actAddNewRuler()));
	panel->addLargeAction(act);

	panel = page->addPannel(tr("调整"));
	act = new QAction(QIcon(":/icons/exchange1.png"), tr("线路反排"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actInverseRail()));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/joint.png"), tr("线路拼接"), this);
	connect(act, &QAction::triggered, this, &RailContext::actJointRail);
	panel->addMediumAction(act);

	panel = page->addPannel(tr("分析"));

	act = new QAction(QIcon(":/icons/counter.png"), tr("断面对数"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(showSectionCount()));
	panel->addLargeAction(act);
	panel->addSeparator();

	act = new QAction(QIcon(":/icons/timetable.png"), tr("车站车次"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actStationTrains()));
	act->setToolTip(tr("车站车次\n显示指定车站的（pyETRC风格的）时刻表。"
		"时刻表以车次为单位，只考虑图定时刻。"));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/electronic-clock.png"), tr("车站事件"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actStationEvents()));
	act->setToolTip(tr("车站事件\n显示指定车站的（基于事件的）时刻表。"
		"时刻表以事件为单位，并考虑推定；如果存在停车，则到、开视为不同的事件。"));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/diagram.png"), tr("断面事件"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actSectionEvents()));
	act->setToolTip(tr("断面事件\n显示本线（站间区间）给定里程标位置的列车（通过）时刻表。"));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/clock.png"), tr("运行快照"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actSnapEvents()));
	act->setToolTip(tr("运行快照\n显示本线指定时刻所有列车的运行状态"
		"（如果运行线与指定时刻存在交点）。"));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/rail.png"), tr("股道分析"), this);
	connect(act, &QAction::triggered, this, &RailContext::actShowTrack);
	act->setToolTip(tr("股道分析\n根据手动给出的股道表，或本系统的内置算法，"
		"绘出可能的股道分布情况。"));
	panel->addLargeAction(act);

	act = new QAction(QIcon(":/icons/diagram.png"), tr("线路拓扑"), this);
	connect(act, &QAction::triggered, this, &RailContext::actRailTopo);
	act->setToolTip(tr("线路拓扑\n检查和显示基线的单向站、单双线等特征。"));
	panel->addLargeAction(act);

	panel->addSeparator();
	act = new QAction(QIcon(":/icons/h_expand.png"), tr("间隔分析"), this);
	connect(act, &QAction::triggered, this, &RailContext::actShowTrainGap);
	act->setToolTip(tr("列车间隔分析\n根据车站的列车事件表，分析相邻车次之间的间隔情况。"));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/adjust-2.png"), tr("间隔汇总"), this);
	connect(act, &QAction::triggered, this, &RailContext::actTrainGapSummary);
	act->setToolTip(tr("列车间隔汇总\n一次性列出所有车站（分别的）以及全局的各类最小间隔。"
		"可能有较大的计算代价。"));
	panel->addMediumAction(act);

	act = new QAction(QIcon(":/icons/diagram.png"), tr("快速创建"), this);
	connect(act, &QAction::triggered, this, & RailContext::actCreatePage);
	act->setToolTip(tr("快速创建单线路运行图\n一键创建新的运行图页面，新运行图页面\n"
		"仅包含当前线路，以当前线路命名。"));
	panel->addLargeAction(act);

	panel = page->addPannel("");
	act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
		tr("删除基线"), this);
	act->setToolTip(tr("删除基线\n删除当前基线，并且从所有运行图中移除（空白运行图将被删除）。"
		"\n自V1.0.1版本起，此操作支持撤销/重做。"));
	panel->addLargeAction(act);
	connect(act, &QAction::triggered, this, &RailContext::actRemoveRailwayU);

	act = new QAction(QIcon(":/icons/copy.png"), tr("副本"), this);
	act->setToolTip(tr("创建基线副本\n以输入的名称，创建于当前基线数据一致的副本。"));
	panel->addLargeAction(act);
	connect(act, &QAction::triggered, this, &RailContext::actDulplicateRailway);

	act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
		tr("关闭面板"), this);
	connect(act, &QAction::triggered, mw, &MainWindow::focusOutRailway);
	act->setToolTip(tr("关闭面板\n关闭当前的基线上下文工具栏页面。"));
	panel->addLargeAction(act);
}

void RailContext::updateRailWidget(std::shared_ptr<Railway> rail)
{
	for (auto p : mw->railStationWidgets) {
		if (p->getRailway() == rail)
			p->refreshData();
    }
}

int RailContext::rulerWidgetIndex(std::shared_ptr<Ruler> ruler)
{
    for(int i=0;i<rulerWidgets.size();i++){
        auto* w=rulerWidgets.at(i);
        if(w->getRuler()==ruler)
            return i;
    }
    return -1;
}

int RailContext::forbidWidgetIndex(std::shared_ptr<Railway> railway)
{
	for (int i = 0; i < forbidWidgets.size(); i++) {
		if (forbidWidgets.at(i)->getRailway() == railway)
			return i;
	}
	return -1;
}

int RailContext::forbidWidgetIndex(const Railway& railway)
{
	for (int i = 0; i < forbidWidgets.size(); i++) {
		if (forbidWidgets.at(i)->getRailway().get() == &railway)
			return i;
	}
	return -1;
}

void RailContext::actChangeOrdinate(int i)
{
	if (updating)
		return;

	int oldindex = railway->ordinateIndex();
	int idx = i - 1;

	if (idx == oldindex)
		return;
	updating = true;
	changeRailOrdinate(railway, idx);
	
	updating = false;
}


void RailContext::changeRailOrdinate(std::shared_ptr<Railway> railway, int idx)
{
	int oldindex = railway->ordinateIndex();
	if (oldindex == idx) return;
	//第一次操作要现场执行，试一下能不能行
	railway->setOrdinateIndex(idx);
	bool flag = railway->calStationYCoeff();
	if (flag) {
		//设置成功，cmd压栈，然后更新运行图
		mw->getUndoStack()->push(new qecmd::ChangeOrdinate(this, railway, oldindex));
		commitOrdinateChange(railway);
	}
	else {
		//提示设置失败，还原
		QMessageBox::warning(mw, tr("错误"),
			tr("设置排图标尺：所选标尺[%1]不能用作排图标尺，"
				"因为通过该标尺已有数据不能确定所有通过车站的坐标。已还原为上一次的设置。")
			.arg(railway->getRuler(idx)->name()));
		cbRulers->setCurrentIndex(oldindex + 1);
	}
}

void RailContext::actSelectRuler()
{
	if (!railway)return;
	QStringList lst;
	for (auto ruler : railway->rulers()) {
		lst << ruler->name();
	}
	if (lst.isEmpty()) {
		QMessageBox::warning(mw, tr("编辑标尺"), tr("当前线路尚无标尺，请先添加标尺。"));
	}
	bool ok;
	QString res = QInputDialog::getItem(mw, tr("编辑标尺"), tr("请选择要导航到的标尺"), lst,
		0, false, &ok);
	if (!ok)return;
	auto ruler = railway->rulerByName(res);
	emit selectRuler(ruler);   //这个用来告诉main，focusInRuler
	openRulerWidget(ruler);
}


void RailContext::openRulerWidget(std::shared_ptr<Ruler> ruler)
{
	int i = rulerWidgetIndex(ruler);
	if (i == -1) {
		//创建
		auto* rw = new RulerWidget(ruler, false);
		auto* dock = new ads::CDockWidget(tr("标尺编辑 - %1 - %2").arg(ruler->name())
			.arg(ruler->railway()->name()));
		dock->setWidget(rw);
		rulerWidgets.append(rw);
		rulerDocks.append(dock);
		connect(rw, &RulerWidget::actChangeRulerData, mw->getRulerContext(),
			&RulerContext::actChangeRulerData);
		connect(rw, &RulerWidget::focusInRuler, mw, &MainWindow::focusInRuler);
		connect(rw, &RulerWidget::actChangeRulerName, mw->getRulerContext(),
			&RulerContext::actChangeRulerName);
		connect(rw, &RulerWidget::actRemoveRuler, mw->getRulerContext(),
			&RulerContext::actRemoveRulerNavi);
		meRulerWidgets->addAction(dock->toggleViewAction());
		mw->getManager()->addDockWidgetFloating(dock);
	}
	else {
		//显示
		auto* dock = rulerDocks.at(i);
		if (dock->isClosed()) {
			dock->toggleView(true);
		}
		else {
			dock->setAsCurrentTab();
		}
	}
}

ForbidTabWidget* RailContext::getOpenForbidWidget(std::shared_ptr<Railway> railway)
{
	int i = forbidWidgetIndex(railway);
	if (i == -1) {
		//创建
		auto* rw = new ForbidTabWidget(railway, false);
		auto* dock = new ads::CDockWidget(tr("天窗编辑 - %1").arg(railway->name()));
		dock->setWidget(rw);
		forbidWidgets.append(rw);
		forbidDocks.append(dock);
		connect(rw, &ForbidTabWidget::forbidChanged,
			this, &RailContext::actUpdadteForbidData);
		connect(rw, &ForbidTabWidget::forbidShowToggled,
			this, &RailContext::actToggleForbidShow);
		mw->forbidMenu->addAction(dock->toggleViewAction());
		mw->getManager()->addDockWidgetFloating(dock);
		return rw;
	}
	else {
		//显示
		auto* dock = forbidDocks.at(i);
		if (dock->isClosed()) {
			dock->toggleView(true);
		}
		else {
			dock->setAsCurrentTab();
		}
		return forbidWidgets.at(i);
	}
}

void RailContext::openForbidWidget(std::shared_ptr<Railway> railway)
{
	getOpenForbidWidget(railway);
}

void RailContext::removeRulerWidgetAt(int i)
{
	auto dock = rulerDocks.takeAt(i);
	rulerWidgets.removeAt(i);
	dock->deleteDockWidget();
}

void RailContext::actStationTrains()
{
	if (!railway)return;
	auto* dialog = new SelectRailStationDialog(railway, mw);
	connect(dialog, &SelectRailStationDialog::stationSelected,
		this, &RailContext::stationTrains);
	dialog->open();
}

void RailContext::stationTrains(std::shared_ptr<RailStation> station)
{
	auto* dialog = new StationTimetableSettledDialog(diagram, railway, station, mw);
	dialog->show();
}

void RailContext::actStationEvents()
{
	if (!railway)return;
	auto* dialog = new SelectRailStationDialog(railway, mw);
	connect(dialog, &SelectRailStationDialog::stationSelected,
		this, &RailContext::stationEvents);
	dialog->open();
}

void RailContext::stationEvents(std::shared_ptr<RailStation> station)
{
	auto* dialog = new RailStationEventListDialog(diagram, railway, station, mw);
	connect(dialog, &RailStationEventListDialog::locateOnEvent,
		mw, &MainWindow::locateDiagramOnStation);
	dialog->show();
}

void RailContext::actSectionEvents()
{
	auto* dialog = new RailSectionEventsDialog(diagram, railway, mw);
	connect(dialog, &RailSectionEventsDialog::locateToEvent,
		mw, &MainWindow::locateDiagramOnMile);
	dialog->show();
}

void RailContext::actSnapEvents()
{
	auto* dialog = new RailSnapEventsDialog(diagram, railway, mw);
	connect(dialog, &RailSnapEventsDialog::locateToEvent,
		mw, &MainWindow::locateDiagramOnMile);
	dialog->show();
}

void RailContext::actAddNewRuler()
{
	mw->getUndoStack()->push(new qecmd::AddNewRuler(railway->validRulerName(tr("新标尺")),
		railway, this));
}

void RailContext::refreshForbidWidgetBasic(std::shared_ptr<Forbid> forbid)
{
	int i = forbidWidgetIndex(forbid->railway());
	if (i != -1) {
		auto* w = forbidWidgets.at(i);
		w->refreshBasicData(forbid);
	}
}

void RailContext::refreshForbidBasicTable(std::shared_ptr<Forbid> forbid)
{
	int i = forbidWidgetIndex(forbid->railway());
	if (i != -1) {
		auto* w = forbidWidgets.at(i);
		w->refreshData(forbid);
	}
}

void RailContext::actInverseRail()
{
	auto rail = railway->cloneBase();
	rail->mergeIntervalData(*railway);
	rail->reverse();
	rail->calStationYCoeff();   // 2022.11.13 fix
	mw->getUndoStack()->push(new qecmd::UpdateRailStations(this, railway, rail, false, diagram.pathCollection(),
		mw->contextTrain));
}

void RailContext::actShowTrainGap()
{
	if (!railway)return;
	auto st = SelectRailStationDialog::getStation(railway, mw);
	if (!st)return;
	auto* dlg = new StationTrainGapDialog(diagram, railway, st, mw);
	connect(dlg, &StationTrainGapDialog::locateToEvent,
		mw, &MainWindow::locateDiagramOnStation);
	dlg->show();
}

void RailContext::actTrainGapSummary()
{
	using namespace std::chrono_literals;
	if (!railway)return;
	auto start = std::chrono::system_clock::now();
	auto* dlg = new TrainGapSummaryDialog(diagram, railway, mw);
	connect(dlg, &TrainGapSummaryDialog::locateOnEvent,
		mw, &MainWindow::locateDiagramOnStation);
	auto end = std::chrono::system_clock::now();
	mw->showStatus(tr("列车间隔分析  用时%1毫秒").arg((end - start) / 1ms));
	dlg->show();
}

void RailContext::actShowTrack()
{
	if (!railway)return;
	auto st = SelectRailStationDialog::getStation(railway, mw);
	if (!st)return;

	auto* dlg = new RailTrackWidget(diagram, railway, st, mw);
	connect(dlg, &RailTrackWidget::actSaveTrackOrder,
		this, &RailContext::actSaveTrackOrder);
	connect(dlg, &RailTrackWidget::saveTrackToTimetable,
		this, &RailContext::actSaveTrackToTimetable);
	dlg->show();
}

void RailContext::actSaveTrackOrder(std::shared_ptr<Railway> railway, std::shared_ptr<RailStation> station, const QList<QString>& order)
{
	mw->getUndoStack()->push(new qecmd::SaveTrackOrder(railway, station, order));
}

void RailContext::actSaveTrackToTimetable(const QVector<TrainStation*>& stations, const QVector<QString>& trackNames)
{
	mw->getUndoStack()->push(new qecmd::SaveTrackToTimetable(stations, trackNames, this));
}

void RailContext::actCreatePage()
{
	if (!railway)return;
	auto page = std::make_shared<DiagramPage>(diagram.config(), QList<std::shared_ptr<Railway>>{railway},
		diagram.validPageName(railway->name()));
	mw->naviView->addNewPageApply(page);
}

void RailContext::actRailTopo()
{
	auto* w = new RailTopoTable(railway, mw);
	w->show();
}

void RailContext::actJointRail()
{
	auto* d = new JointRailwayDialog(diagram.railCategory(), railway, mw);
	connect(d, &JointRailwayDialog::applied,
		this, &RailContext::actJointRailApplied);
	d->show();
}

void RailContext::actJointRailApplied(std::shared_ptr<Railway> rail, std::shared_ptr<Railway> data)
{
	// 必须直接提交，不能是打开站表后提交。因为还有其他的信息（标尺天窗）
	
	mw->getUndoStack()->beginMacro(tr("拼接线路: %1").arg(rail->name()));
	// 先要取消标尺排图，如果打开了的话
	if (rail->ordinate()) {
		rail->resetOrdinate();
		mw->getUndoStack()->push(new qecmd::ChangeOrdinate(this, rail, -1));
	}
	mw->getUndoStack()->push(new qecmd::UpdateRailStations(this, rail, data, false, diagram.pathCollection(),
		mw->contextTrain));
	mw->getUndoStack()->endMacro();
}

void RailContext::commitChangeRailName(std::shared_ptr<Railway> rail)
{
	//2021.08.15取消：更改站名时不必刷新页面
	//否则同时更改的线路信息就没了！
	//updateRailWidget(rail);
	if (rail == railway) {
		refreshData();
	}
	//更新打开的窗口标题
	for (int i = 0; i < mw->railStationWidgets.size(); i++) {
		auto w = mw->railStationWidgets.at(i);
		w->refreshBasicData();
		if (w->getRailway() == rail) {
			mw->railStationDocks.at(i)->setWindowTitle(tr("基线编辑 - ") + rail->name());
		}
	}

	emit railNameChanged(rail);
}

void RailContext::actUpdateTimetable(std::shared_ptr<Railway> railway, std::shared_ptr<Railway> newtable, bool equiv)
{
	mw->getUndoStack()->push(new qecmd::UpdateRailStations(this, railway, newtable, equiv, diagram.pathCollection(),
		mw->contextTrain));
}

void RailContext::actUpdadteForbidData(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data)
{
	mw->getUndoStack()->push(new qecmd::UpdateForbidData(forbid, data, this));
}

void RailContext::actToggleForbidShow(std::shared_ptr<Forbid> forbid, Direction dir)
{
	mw->getUndoStack()->push(new qecmd::ToggleForbidShow(forbid, dir, this));
}

void RailContext::afterRailStationChanged(std::shared_ptr<Railway> railway, bool equiv)
{
	updateRailWidget(railway);
	mw->onStationTableChanged(railway, equiv);
	//emit stationTableChanged(railway, equiv);  // 2023.08.25: change to direct call
	foreach(auto p, rulerWidgets) {
		p->getModel()->updateRailIntervals(railway, equiv);
	}
	foreach(auto p, forbidWidgets) {
		p->updateAllRailIntervals(railway, equiv);
	}
}

void RailContext::commitOrdinateChange(std::shared_ptr<Railway> railway)
{
	railway->calStationYCoeff();
	mw->updateRailwayDiagrams(railway);
}

void RailContext::showSectionCount()
{
	if (!railway)return;
	auto* dialog = new SectionCountDialog(diagram, railway, mw);
	dialog->show();
}

void RailContext::refreshRulerTable(std::shared_ptr<Ruler> ruler)
{
	for (auto p : rulerWidgets) {
		if (p->getRuler() == ruler)
			p->refreshData();
	}
}

void RailContext::removeRulerWidgetsForRailway(std::shared_ptr<Railway> rail)
{
	for (auto ruler : rail->rulers())
		removeRulerWidget(ruler);
}

void RailContext::removeRulerWidget(std::shared_ptr<Ruler> ruler)
{
	auto cont = mw->getRulerContext();
	if (ruler == cont->getRuler()) {
		cont->resetRuler();
		emit cont->focusOutRuler();
	}
	int i = 0;
	while (i < rulerWidgets.size()) {
		if (rulerWidgets.at(i)->getRuler() == ruler) {
			removeRulerWidgetAt(i);
		}
		else 
			i++;
	}
}

void RailContext::onRulerNameChanged(std::shared_ptr<Ruler> ruler)
{
	for (int i = 0; i < rulerWidgets.size(); i++) {
		auto w = rulerWidgets.at(i);
		if (w->getRuler() == ruler) {
			//2021.08.15  这里不能更新全局，否则数据更新就没了
			w->refreshBasicData();
			rulerDocks.at(i)->setWindowTitle(tr("标尺编辑 - %1 - %2").arg(ruler->name())
				.arg(ruler->railway()->name()));
		}
	}
	if ((ruler->railway()) == railway) {
		cbRulers->setItemText(ruler->index()+1, ruler->name());
	}
}

void RailContext::removeRulerAt(const Railway& rail, int i, bool isord)
{
	if (&rail == railway.get()) {
		updating = true;
		cbRulers->removeItem(i + 1);
		if (isord) {
			cbRulers->setCurrentIndex(0);
		}
		updating = false;
	}
	emit rulerRemovedAt(rail, i);
}

void RailContext::insertRulerAt(const Railway& rail, std::shared_ptr<Ruler> ruler, bool isord)
{
	if (&rail == railway.get()) {
		updating = true;
		cbRulers->insertItem(ruler->index() + 1, ruler->name());
		if (isord) {
			cbRulers->setCurrentIndex(ruler->index() + 1);
		}
		updating = false;
	}
	emit rulerInsertedAt(rail, ruler->index());
}

void RailContext::commitAddNewRuler(std::shared_ptr<Ruler> ruler)
{
	insertRulerAt(*(ruler->railway()), ruler, false);
	openRulerWidget(ruler);
}

void RailContext::undoAddNewRuler(std::shared_ptr<Ruler> ruler)
{
	mw->getRulerContext()->commitRemoveRuler(ruler, false);
}

void RailContext::commitForbidChange(std::shared_ptr<Forbid> forbid)
{
	updateForbidDiagrams(forbid, Direction::Down);
	updateForbidDiagrams(forbid, Direction::Up);
	refreshForbidBasicTable(forbid);
}

void RailContext::commitToggleForbidShow(std::shared_ptr<Forbid> forbid, Direction dir)
{
	updateForbidDiagrams(forbid, dir);
	refreshForbidWidgetBasic(forbid);
}

void RailContext::removeAllDocks()
{
	foreach(auto p, rulerDocks) {
		p->deleteDockWidget();
	}
	rulerDocks.clear();
	rulerWidgets.clear();
	foreach(auto p, forbidDocks) {
		p->deleteDockWidget();
	}
	forbidDocks.clear();
	forbidWidgets.clear();
}

void RailContext::setRailway(std::shared_ptr<Railway> rail)
{
	railway = rail;
	refreshData();
}

void RailContext::actOpenStationWidget()
{
	if(railway)
        mw->actOpenRailStationWidget(railway);
}

void RailContext::actOpenForbidWidget()
{
	if (railway)
		openForbidWidget(railway);
}

void RailContext::actRemoveRailwayU()
{
	if (!railway)return;
	if (int i = diagram.railCategory().getRailwayIndex(railway); i != -1) {
		removeRailwayAtU(i);
	}
}

void RailContext::actDulplicateRailway()
{
	if (railway)
		emit dulplicateRailway(railway);
}

#if 0
[[deprecated]]
void RailContext::actRemoveRailway()
{
	if (railway) {
		auto flag = QMessageBox::question(mw, tr("删除线路"),
			tr("删除当前线路，同时从所有运行图页面中删除当前线路，空白运行图将被删除。"
				"请注意此操作牵涉较广，且不可撤销。\n"
				"你是否确认要删除线路[%1]？").arg(railway->name()));
		if (flag != QMessageBox::Yes)
			return;
		int i = diagram.railCategory().getRailwayIndex(railway);
		if (i != -1) {
			mw->naviModel->removeRailwayAt(i);
		}
	}
}
#endif

void RailContext::openForbidWidgetTab(std::shared_ptr<Forbid> forbid, 
	std::shared_ptr<Railway> railway)
{
	auto* w = getOpenForbidWidget(railway); 
	w->setCurrentIndex(forbid->index());
}


void RailContext::actChangeRailName(std::shared_ptr<Railway> rail, const QString &name)
{
    mw->getUndoStack()->push(new qecmd::ChangeRailName(rail,name,this));
}

void RailContext::actAddNamedNewRuler(std::shared_ptr<Railway> railway, const QString& name)
{
	mw->getUndoStack()->push(new qecmd::AddNewRuler(name, railway, this));
}

void RailContext::actUpdateRailNotes(std::shared_ptr<Railway> railway, const RailInfoNote& note)
{
	mw->getUndoStack()->push(new qecmd::UpdateRailNote(railway, note));
}

void RailContext::commitSaveTrackToTimetable()
{
	mw->contextTrain->refreshCurrentTrainWidgets();
}


void RailContext::removeRailwayAtU(int i)
{
	auto& dia = diagram;
	auto rail = dia.railwayAt(i);

	// 2023.08.22: re-bind affected trains (that bound by path)
	std::vector<std::shared_ptr<Train>> affect_trains = diagram.trainCollection().affectedTrainsByRailInPath(rail);

	// 2023.08.22: the remove/reset pages should be redone before the removing, 
	// while the rebind should be redone after that. Thus, the rebind operation is implmented using macro, 
	// while the remove/reset are implemented using parents.

	// 2023.09.04: The rebind operation should be redone/undone AFTER the operation of removing railway.
	// Thus, the macro is not suitable for this.
	//mw->getUndoStack()->beginMacro(tr("删除线路: %1").arg(rail->name()));
	auto* cmd = new qecmd::RemoveRailway(rail, i, this);
	
	// re-bind all the affected paths
	auto* cmd_rebind = new qecmd::RebindTrainsByPaths(std::move(affect_trains), mw->contextTrain, cmd);

	// 注意这里只会删除一条线路
	// 可能会涉及多个Page的删除！倒着来
	for (int i = dia.pages().size() - 1; i >= 0; i--) {
		auto page = dia.pages().at(i);
		if (page->containsRailway(rail)) {
			if (page->railwayCount() > 1) {
				// 修订Page
				new qecmd::ResetPage(page, page->takenRailCopy(rail), mw->contextPage, cmd);
			}
			else {
				new qecmd::RemovePage(dia, i, mw->naviView, cmd);
			}
		}
	}
	mw->getUndoStack()->push(cmd);
	//mw->getUndoStack()->push(cmd_rebind);
	//mw->getUndoStack()->endMacro();
}

void RailContext::commitRemoveRailwayU(std::shared_ptr<Railway> railway, int index)
{
	Q_UNUSED(railway)
	mw->naviModel->commitRemoveRailwayAtU(index);
}

void RailContext::undoRemoveRailwayU(std::shared_ptr<Railway> railway, int index)
{
	mw->naviModel->commitInsertRailway(index, railway);
}

void RailContext::actImportRailways(QList<std::shared_ptr<Railway>>& rails)
{
	auto* cmd = new qecmd::ImportRailways(mw->naviModel, rails);
	std::set<std::shared_ptr<Train>> affected_trains{};
	foreach(auto rail, rails) {
		for (const auto& path : diagram.pathCollection().paths()) {
			if (path->containsRailwayByName(rail->name())) {
				const auto& trains = path->trainsShared();
				affected_trains.insert(trains.begin(), trains.end());
			}
		}
	}
	if (!affected_trains.empty()) {
		new qecmd::RebindTrainsByPaths(std::vector<std::shared_ptr<Train>>(affected_trains.begin(), affected_trains.end()),
			mw->contextTrain, cmd);
	}

	mw->getUndoStack()->push(cmd);
}





qecmd::ChangeRailName::ChangeRailName(std::shared_ptr<Railway> rail_,
	const QString& name, RailContext* context, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("修改线名: ") + name, parent),
	rail(rail_), newname(name), cont(context)
{
}

void qecmd::ChangeRailName::undo()
{
	std::swap(rail->nameRef(), newname);
	cont->commitChangeRailName(rail);
}

void qecmd::ChangeRailName::redo()
{
	std::swap(rail->nameRef(), newname);
	cont->commitChangeRailName(rail);
}


qecmd::UpdateRailStations::UpdateRailStations(RailContext* context,
	std::shared_ptr<Railway> old_,
	std::shared_ptr<Railway> new_, bool equiv_, 
	TrainPathCollection& pathcoll, TrainContext* contTrain,
	QUndoCommand* parent) :
	QUndoCommand(parent), cont(context), railold(old_), railnew(new_), equiv(equiv_)
{
	qDebug() << "UpdateRailStations: equiv " << equiv << Qt::endl;
	setText(QObject::tr("更新基线数据: %1").arg(old_->name()));
	ordinateIndex = old_->ordinateIndex();

	// 2023.08.23: rebind trains
	auto trains = pathcoll.affectedTrainsByRailway(railold);
	if (!trains.empty()) {
		new qecmd::RebindTrainsByPaths(std::move(trains), contTrain, this);
	}
}

void qecmd::UpdateRailStations::undo()
{
	railold->swapBaseWith(*railnew);
	//railold->setOrdinateIndex(ordinateIndex);
	cont->afterRailStationChanged(railold, equiv);
	QUndoCommand::undo();   // rebind
}

void qecmd::UpdateRailStations::redo()
{
	railold->swapBaseWith(*railnew);
	cont->afterRailStationChanged(railold, equiv);
	QUndoCommand::redo();   // rebind
}

qecmd::ChangeOrdinate::ChangeOrdinate(RailContext *context_, std::shared_ptr<Railway> rail_,
                                      int index_, QUndoCommand *parent):
    QUndoCommand(QObject::tr("更改排图标尺: ")+rail_->name(),parent),
    cont(context_),rail(rail_),index(index_)
{}


void qecmd::ChangeOrdinate::undo()
{
	int idx = rail->ordinateIndex();
	rail->setOrdinateIndex(index);
	index = idx;
	cont->commitOrdinateChange(rail);
}

void qecmd::ChangeOrdinate::redo()
{
	if (first) {
		first = false;
		return;
	}
	int idx = rail->ordinateIndex();
	rail->setOrdinateIndex(index);
	index = idx;
	cont->commitOrdinateChange(rail);
}

void qecmd::AddNewRuler::undo()
{
	auto r = railway->takeLastRuler();
	cont->undoAddNewRuler(r);
}

void qecmd::AddNewRuler::redo()
{
	if (!theRuler) {
		// 第一次执行时，需要新建
		theRuler = railway->addEmptyRuler(name, true);
		theData = theRuler->clone();
		setText(QObject::tr("新建空白标尺: %1").arg(theRuler->name()));
	}
	else {
		railway->restoreRulerFrom(theRuler, theData->getRuler(0));
	}
	cont->commitAddNewRuler(railway->rulers().last());
}

qecmd::UpdateForbidData::UpdateForbidData(std::shared_ptr<Forbid> forbid_,
                                          std::shared_ptr<Railway> data_,
                                          RailContext *context, QUndoCommand *parent):
    QUndoCommand(QObject::tr("更新天窗: %1 - %2").arg(forbid_->name(),
        forbid_->railway()->name()),parent),
    forbid(forbid_),data(data_),cont(context){}


void qecmd::UpdateForbidData::undo()
{
	forbid->swap(*(data->getForbid(0)));
	cont->commitForbidChange(forbid);
}

void qecmd::UpdateForbidData::redo()
{
	forbid->swap(*(data->getForbid(0)));
	cont->commitForbidChange(forbid);
}

qecmd::ToggleForbidShow::ToggleForbidShow(std::shared_ptr<Forbid> forbid_,
	Direction dir_, RailContext* context, QUndoCommand* parent):
	QUndoCommand(parent),forbid(forbid_),dir(dir_),cont(context)
{
	QString s = (forbid->isDirShow(dir) ? QObject::tr("隐藏") : QObject::tr("显示"));
	setText(QObject::tr("%1%2天窗-%3").arg(s, forbid->name(), DirFunc::dirToString(dir)));
}

void qecmd::ToggleForbidShow::undo()
{
	forbid->toggleDirShow(dir);
	cont->commitToggleForbidShow(forbid, dir);
}

void qecmd::ToggleForbidShow::redo()
{
	forbid->toggleDirShow(dir);
	cont->commitToggleForbidShow(forbid, dir);
}

qecmd::UpdateRailNote::UpdateRailNote(std::shared_ptr<Railway> railway, 
	const RailInfoNote& data, QUndoCommand* parent):
	QUndoCommand(QObject::tr("更新线路备注: %1").arg(railway->name()),parent),
	railway(railway),data(data)
{
}

void qecmd::UpdateRailNote::undo()
{
	std::swap(railway->notes(), data);
}

void qecmd::UpdateRailNote::redo()
{
	std::swap(railway->notes(), data);
}

qecmd::SaveTrackOrder::SaveTrackOrder(std::shared_ptr<Railway> railway, 
	std::shared_ptr<RailStation> station, const QList<QString>& order, QUndoCommand* parent):
	QUndoCommand(QObject::tr("保存股道表: %1 @ %2").arg(railway->name(),
		station->name.toSingleLiteral()),parent),
	railway(railway),station(station),order(order)
{
}

void qecmd::SaveTrackOrder::undo()
{
	std::swap(station->tracks, order);
}
void qecmd::SaveTrackOrder::redo()
{
	std::swap(station->tracks, order);
}

void qecmd::SaveTrackToTimetable::undo()
{
	commit();
}
void qecmd::SaveTrackToTimetable::redo()
{
	commit();
}

void qecmd::SaveTrackToTimetable::commit()
{
	assert(stations.size() == trackNames.size());
	auto it1 = stations.begin();
	auto it2 = trackNames.begin();
	while (it1 != stations.end()) {
		std::swap((*it1++)->track, (*it2++));
	}
	cont->commitSaveTrackToTimetable();
}

qecmd::RemoveRailway::RemoveRailway(std::shared_ptr<Railway> railway, int index,
	RailContext* context, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("删除线路: %1").arg(railway->name()), parent),
	railway(railway), index(index), cont(context)
{
}

void qecmd::RemoveRailway::undo()
{
	// 这里先执行Railway的删除undo  （重新添加）
	railway->setValid(true);
	cont->undoRemoveRailwayU(railway, index);

	QUndoCommand::undo();
}

void qecmd::RemoveRailway::redo()
{
	railway->setValid(false);
	// 这里是Railway的删除操作
	cont->commitRemoveRailwayU(railway, index);

	// 2023.08.22: move to last?
	QUndoCommand::redo();    // 先redo （执行所有的Page修订删除）
}


qecmd::ImportRailways::ImportRailways(DiagramNaviModel* navi_,
	const QList<std::shared_ptr<Railway>>& rails_, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("导入") + QString::number(rails_.size()) + QObject::tr("条线路"), parent),
	navi(navi_), rails(rails_)
{
}

void qecmd::ImportRailways::undo()
{
	navi->removeTailRailways(rails.size());
	QUndoCommand::undo();   // rebind trains
}

void qecmd::ImportRailways::redo()
{
	//添加到线路表后面，然后inform change
	//现在不需要通知别人发生变化...
	navi->importRailways(rails);
	QUndoCommand::redo();   // rebind trains
}


#endif

