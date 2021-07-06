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


		panel = page->addPannel(tr("分析"));

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
	edName->setText(train->trainName().full());
	edStart->setText(train->starting().toSingleLiteral());
	edEnd->setText(train->terminal().toSingleLiteral());
}

void TrainContext::showTrainEvents()
{
	auto* dialog = new TrainEventDialog(diagram, train, mw);
	dialog->show();
}
