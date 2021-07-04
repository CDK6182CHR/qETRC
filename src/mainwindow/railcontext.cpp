#include "railcontext.h"
#include "mainwindow.h"

RailContext::RailContext(Diagram& diagram_, SARibbonContextCategory* context, 
	MainWindow* mw_, QObject* parent):
	QObject(parent),diagram(diagram_),cont(context),mw(mw_)
{
	initUI();
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

void RailContext::setRailway(std::shared_ptr<Railway> rail)
{
	railway = rail;
}

void RailContext::actOpenStationWidget()
{
	if(railway)
		mw->actOpenRailStationWidget(railway);
}
