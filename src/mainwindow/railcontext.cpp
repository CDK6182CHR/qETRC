#include "railcontext.h"
#include "mainwindow.h"

#include "viewers/sectioncountdialog.h"

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
	act = new QAction(QIcon(":icons/ruler.png"), tr("标尺"), this);
	auto* me = new SARibbonMenu;
	act->setToolTip(tr("标尺编辑\n创建或者导航到本线标尺编辑的面板"));
	meRulerWidgets = me;
	act->setMenu(me);
	connect(act, SIGNAL(triggered()), this, SLOT(actSelectRuler()));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);

	panel = page->addPannel(tr("分析"));

	act = new QAction(QIcon(":/icons/counter.png"), tr("断面对数"), this);
	connect(act, SIGNAL(triggered()), this, SLOT(showSectionCount()));
	btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);
}

void RailContext::updateRailWidget(std::shared_ptr<Railway> rail)
{
	for (auto p : mw->railStationWidgets) {
		if (p->getRailway() == rail)
			p->refreshData();
	}
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
	//创建dock...
}

void RailContext::commitChangeRailName(std::shared_ptr<Railway> rail)
{
	updateRailWidget(rail);
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
