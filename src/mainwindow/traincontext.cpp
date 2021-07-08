#include "traincontext.h"

#include "viewers/traineventdialog.h"
#include "mainwindow.h"

#include <QtWidgets>

TrainContext::TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_,
	MainWindow* mw_):
	QObject(mw_),
	diagram(diagram_),cont(context_),mw(mw_)
{
	initUI();
}

void TrainContext::resetTrain()
{
	train.reset();
	edName->setText("");
	edStart->setText("");
	edEnd->setText("");
}

void TrainContext::initUI()
{
	//审阅
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("审阅"));

		auto* panel = page->addPannel(tr("基本信息"));

		//基本信息：车次，始发终到 代码有点多
		if constexpr (true) {
			QWidget* w = new QWidget;
			auto* vlay = new QVBoxLayout;
			edName = new QLineEdit;
			edName->setAlignment(Qt::AlignCenter);
			edName->setFocusPolicy(Qt::NoFocus);
			vlay->addWidget(edName);

			auto* hlay = new QHBoxLayout;
			edStart = new QLineEdit;
			edStart->setFocusPolicy(Qt::NoFocus);
			edStart->setAlignment(Qt::AlignCenter);
			hlay->addWidget(edStart);
			hlay->addWidget(new QLabel("->"));
			edEnd = new QLineEdit;
			edEnd->setFocusPolicy(Qt::NoFocus);
			edEnd->setAlignment(Qt::AlignCenter);
			hlay->addWidget(edEnd);
			vlay->addLayout(hlay);
			w->setLayout(vlay);
			w->setMaximumWidth(240);
			panel->addWidget(w, SARibbonPannelItem::Large);
		}

		auto* act = new QAction(QIcon(":/icons/trainline.png"), tr("运行线"), this);
		act->setToolTip(tr("高亮列车运行线\n在所有运行图窗口中，高亮本次列车运行线"));
		auto* btn = panel->addLargeAction(act);
		btn->setMinimumWidth(70);
		connect(act, SIGNAL(triggered()), this, SLOT(actShowTrainLine()));

		panel = page->addPannel(tr("分析"));

		act = new QAction(QIcon(":/icons/clock.png"), tr("事件表"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(showTrainEvents()));
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(70);
	}

	//编辑
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("编辑"));
		auto* panel = page->addPannel(tr("基本"));

		auto* act = new QAction(QIcon(":/icons/timetable.png"), tr("基本编辑"), this);
		auto* btn = panel->addLargeAction(act);
		btn->setMinimumWidth(80);
		connect(act, SIGNAL(triggered()), this, SLOT(actShowBasicWidget()));
	}
	
}

int TrainContext::getBasicWidgetIndex()
{
	return getBasicWidgetIndex(train);
}

int TrainContext::getBasicWidgetIndex(std::shared_ptr<Train> t)
{
	for (int i = 0; i < basicWidgets.size(); i++) {
		auto p = basicWidgets.at(i);
		if (p->train() == t)
			return i;
	}
	return -1;
}

int TrainContext::getBasicWidgetIndex(ads::CDockWidget* dock)
{
	for (int i = 0; i < basicDocks.size(); i++) {
		if (basicDocks.at(i) == dock)
			return i;
	}
	return -1;
}

void TrainContext::removeTrainWidget(std::shared_ptr<Train> train)
{
	removeBasicDockAt(getBasicWidgetIndex(train));
}

void TrainContext::actShowTrainLine()
{
	emit highlightTrainLine(train);
}

void TrainContext::actShowBasicWidget()
{
	int idx = getBasicWidgetIndex();
	if (idx == -1) {
		//创建
		auto* w = new BasicTrainWidget(diagram.trainCollection(), false);
		w->setTrain(train);
		auto* dock = new ads::CDockWidget(tr("列车基本编辑 - %1").arg(train->trainName().full()));
		dock->setWidget(w);
		//dock->setAttribute(Qt::WA_DeleteOnClose);
		connect(dock, SIGNAL(closed()), this, SLOT(onTrainDockClosed()));
		mw->getManager()->addDockWidgetFloating(dock);
		basicWidgets.append(w);
		basicDocks.append(dock);
	}
	else {
		//保证可见
		auto* dock = basicDocks.at(idx);
		if (dock->isClosed()) {
			dock->toggleView(true);
		}
		else
			dock->setAsCurrentTab();
	}
}

void TrainContext::onTrainDockClosed()
{
	auto* dock = static_cast<ads::CDockWidget*>(sender());
	if (dock) {
		int idx = getBasicWidgetIndex(dock);
		if (idx == -1)
			return;
		removeBasicDockAt(idx);
	}
}

void TrainContext::removeBasicDockAt(int idx)
{
	if (idx == -1)
		return;
	auto* dock = basicDocks.takeAt(idx);
	basicWidgets.removeAt(idx);
	dock->deleteDockWidget();
}

void TrainContext::setTrain(std::shared_ptr<Train> train_)
{
	train = train_;
	edName->setText(train->trainName().full());
	edStart->setText(train->starting().toSingleLiteral());
	edEnd->setText(train->terminal().toSingleLiteral());
}

void TrainContext::showTrainEvents()
{
	auto* dialog = new TrainEventDialog(diagram, train, mw);
	dialog->show();
}
