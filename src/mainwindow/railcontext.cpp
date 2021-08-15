#include "railcontext.h"
#include "mainwindow.h"

#include "viewers/sectioncountdialog.h"
#include "dialogs/selectrailstationdialog.h"
#include "viewers/stationtimetablesettled.h"
#include "viewers/railstationeventlist.h"
#include "viewers/railsectionevents.h"
#include "viewers/railsnapevents.h"

#include <QtWidgets>

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

void RailContext::initUI()
{
	auto* page = cont->addCategoryPage(tr("数据"));

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
		ed->setFixedWidth(120);

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
	auto* btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);
	connect(act, SIGNAL(triggered()), this, SLOT(actOpenStationWidget()));

	act = new QAction(QIcon(":/icons/close.png"), tr("删除基线"), this);
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);
	connect(act, SIGNAL(triggered()), this, SLOT(actRemoveRailway()));

	panel = page->addPannel(tr("标尺"));
	act = new QAction(QIcon(":/icons/ruler.png"), tr("标尺"), this);
	auto* me = new SARibbonMenu;
	act->setToolTip(tr("标尺编辑\n创建或者导航到本线标尺编辑的面板"));
	meRulerWidgets = me;
	act->setMenu(me);
	connect(act, SIGNAL(triggered()), this, SLOT(actSelectRuler()));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);

	act = new QAction(QIcon(":/icons/add.png"), tr("新建标尺"), this);
	act->setToolTip(tr("新建标尺\n新建本线的空白标尺"));
	connect(act, SIGNAL(triggered()), this, SLOT(actAddNewRuler()));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);

	panel = page->addPannel(tr("分析"));

	act = new QAction(QIcon(":/icons/counter.png"), tr("断面对数"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(showSectionCount()));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);

	act = new QAction(QIcon(":/icons/timetable.png"), tr("车站车次表"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actStationTrains()));
	act->setToolTip(tr("车站车次表\n显示指定车站的（pyETRC风格的）时刻表。"
		"时刻表以车次为单位，只考虑图定时刻。"));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(80);

	act = new QAction(QIcon(":/icons/electronic-clock.png"), tr("车站事件表"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actStationEvents()));
	act->setToolTip(tr("车站事件表\n显示指定车站的（基于事件的）时刻表。"
		"时刻表以事件为单位，并考虑推定；如果存在停车，则到、开视为不同的事件。"));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(80);

	act = new QAction(QIcon(":/icons/diagram.png"), tr("断面事件表"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actSectionEvents()));
	act->setToolTip(tr("断面事件表\n显示本线（站间区间）给定里程标位置的列车（通过）时刻表。"));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(80);

	act = new QAction(QIcon(":/icons/clock.png"), tr("运行快照"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(actSnapEvents()));
	act->setToolTip(tr("运行快照\n显示本线指定时刻所有列车的运行状态"
		"（如果运行线与指定时刻存在交点）。"));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(80);
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

void RailContext::actChangeOrdinate(int i)
{
	if (updating)
		return;

	int oldindex = railway->ordinateIndex();
	int idx = i - 1;

	if (idx == oldindex)
		return;
	updating = true;

	//第一次操作要现场执行，试一下能不能行
	railway->setOrdinateIndex(idx);
	bool flag = railway->calStationYValue(diagram.config());
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
	updating = false;
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
			.arg(ruler->railway().name()));
		dock->setWidget(rw);
		rulerWidgets.append(rw);
		rulerDocks.append(dock);
		connect(rw, &RulerWidget::actChangeRulerData, mw->getRulerContext(),
			&RulerContext::actChangeRulerData);
		connect(rw, &RulerWidget::focusInRuler, mw, &MainWindow::focusInRuler);
		connect(rw, &RulerWidget::actChangeRulerName, mw->getRulerContext(),
			&RulerContext::actChangeRulerName);
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
	dialog->show();
}

void RailContext::actSectionEvents()
{
	auto* dialog = new RailSectionEventsDialog(diagram, railway, mw);
	dialog->show();
}

void RailContext::actSnapEvents()
{
	auto* dialog = new RailSnapEventsDialog(diagram, railway, mw);
	dialog->show();
}

void RailContext::actAddNewRuler()
{
	mw->getUndoStack()->push(new qecmd::AddNewRuler(railway->validRulerName(tr("新标尺")),
		railway, this));
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
	mw->getUndoStack()->push(new qecmd::UpdateRailStations(this, railway, newtable, equiv));
}

void RailContext::commitUpdateTimetable(std::shared_ptr<Railway> railway, bool equiv)
{
	updateRailWidget(railway);
	emit stationTableChanged(railway, equiv);
}

void RailContext::commitOrdinateChange(std::shared_ptr<Railway> railway)
{
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
				.arg(ruler->railway().name()));
		}
	}
	if (&(ruler->railway()) == railway.get()) {
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
}

void RailContext::commitAddNewRuler(std::shared_ptr<Ruler> ruler)
{
	insertRulerAt(ruler->railway(), ruler, false);
	openRulerWidget(ruler);
}

void RailContext::undoAddNewRuler(std::shared_ptr<Ruler> ruler)
{
	mw->getRulerContext()->commitRemoveRuler(ruler, false);
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



void RailContext::actChangeRailName(std::shared_ptr<Railway> rail, const QString &name)
{
    mw->getUndoStack()->push(new qecmd::ChangeRailName(rail,name,this));
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
	std::shared_ptr<Railway> new_, bool equiv_, QUndoCommand* parent) :
	QUndoCommand(parent), cont(context), railold(old_), railnew(new_), equiv(equiv_)
{
	setText(QObject::tr("更新基线数据: %1").arg(old_->name()));
	ordinateIndex = old_->ordinateIndex();
}

void qecmd::UpdateRailStations::undo()
{
	railold->swapBaseWith(*railnew);
	railold->setOrdinateIndex(ordinateIndex);
	cont->commitUpdateTimetable(railold, equiv);
}

void qecmd::UpdateRailStations::redo()
{
	railold->swapBaseWith(*railnew);
	cont->commitUpdateTimetable(railold, equiv);
}

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
		auto r = railway->addEmptyRuler(name, true);
		theRuler = r->clone();
		setText(QObject::tr("新建空白标尺: %1").arg(r->name()));
	}
	else {
		railway->addRulerFrom(theRuler->getRuler(0));
	}
	cont->commitAddNewRuler(railway->rulers().last());
}
