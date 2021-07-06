#include "traincontext.h"

#include "viewers/traineventdialog.h"
#include "mainwindow.h"

TrainContext::TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_,
	MainWindow* mw_):
	QObject(mw_),
	diagram(diagram_),cont(context_),mw(mw_)
{
	initUI();
}

void TrainContext::initUI()
{
	//审阅
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("审阅"));
		auto* panel = page->addPannel(tr(""));

		auto* act = new QAction(QIcon(":/icons/clock.png"), tr("事件表"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(showTrainEvents()));
		auto* btn = panel->addLargeAction(act);
		btn->setMinimumWidth(70);
	}

	//编辑
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("编辑"));
		auto* panel = page->addPannel(tr(""));
	}
	
}

void TrainContext::setTrain(std::shared_ptr<Train> train_)
{
	train = train_;
    //todo 相关更新...
}

void TrainContext::showTrainEvents()
{
	auto* dialog = new TrainEventDialog(diagram, train, mw);
	dialog->show();
}
