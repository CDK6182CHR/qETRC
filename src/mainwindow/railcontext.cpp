#include "railcontext.h"
#include "mainwindow.h"

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
	//todo...
}

void RailContext::initUI()
{
	auto* page = cont->addCategoryPage(tr("数据"));
	auto* panel = page->addPannel(tr("编辑"));
	
	QAction* act;
	act = new QAction(QIcon(":/icons/rail.png"), tr("基线编辑"), this);
	auto* btn = panel->addLargeAction(act);
	btn->setMinimumWidth(70);
	connect(act, SIGNAL(triggered()), this, SLOT(actOpenStationWidget()));
}

void RailContext::updateRailWidget(std::shared_ptr<Railway> rail)
{
	for (auto p : mw->railStationWidgets) {
		if (p->getRailway() == rail)
			p->refreshData();
	}
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
