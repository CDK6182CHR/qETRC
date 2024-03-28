#ifndef QETRC_MOBILE_2
#include "traincontext.h"

#include "viewers/events/traineventdialog.h"
#include "mainwindow.h"
#include "viewcategory.h"
#include "data/train/train.h"
#include "data/train/routing.h"
#include "viewers/trainlinedialog.h"
#include "viewers/rulerrefdialog.h"
#include "dialogs/exchangeintervaldialog.h"
#include "dialogs/modifytimetabledialog.h"
#include "viewers/diagnosisdialog.h"
#include "viewers/timetablequickwidget.h"
#include "viewers/traininfowidget.h"
#include "editors/trainlistwidget.h"
#include "model/train/trainlistmodel.h"
#include "data/common/qesystem.h"
#include "data/rail/railway.h"
#include "railcontext.h"
#include "routingcontext.h"
#include "data/diagram/trainadapter.h"

#include <DockManager.h>

#include <QApplication>
#include <QMessageBox>
#include <QColorDialog>
#include <SARibbonContextCategory.h>
#include <SARibbonLineEdit.h>
#include <QLabel>
#include <SARibbonComboBox.h>
#include <SARibbonCheckBox.h>
#include <SARibbonMenu.h>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <util/linestylecombo.h>
#include <util/utilfunc.h>
#include <chrono>

#include "editors/basictrainwidget.h"
#include "editors/edittrainwidget.h"
#include "wizards/timeinterp/timeinterpwizard.h"
#include "data/train/traintype.h"
#include "dialogs/locatedialog.h"
#include "dialogs/correcttimetabledialog.h"
#include "util/pagecomboforrail.h"
#include "data/algo/timetablecorrector.h"
#include "editors/trainpath/selectpathdialog.h"
#include "model/diagram/diagramnavimodel.h"
#include "dialogs/selectroutingdialog.h"

TrainContext::TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_,
	MainWindow* mw_) :
	QObject(mw_),
	diagram(diagram_), cont(context_), mw(mw_)
{
	initUI();
}

void TrainContext::resetTrain()
{
	train.reset();
	refreshData();
}

void TrainContext::initUI()
{
	//审阅
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("列车审阅(&6)"));

		auto* panel = page->addPannel(tr("基本信息"));

		//基本信息：车次，始发终到 代码有点多
		if constexpr (true) {
			QWidget* w = new QWidget;
			auto* vlay = new QVBoxLayout;
			edName = new SARibbonLineEdit;
			edName->setAlignment(Qt::AlignCenter);
			edName->setFocusPolicy(Qt::NoFocus);
			vlay->addWidget(edName);

			auto* hlay = new QHBoxLayout;
			edStart = new SARibbonLineEdit;
			edStart->setFocusPolicy(Qt::NoFocus);
			edStart->setAlignment(Qt::AlignCenter);
			hlay->addWidget(edStart);
			hlay->addWidget(new QLabel("->"));
			edEnd = new SARibbonLineEdit;
			edEnd->setFocusPolicy(Qt::NoFocus);
			edEnd->setAlignment(Qt::AlignCenter);
			hlay->addWidget(edEnd);
			vlay->addLayout(hlay);
			w->setLayout(vlay);
			w->setMaximumWidth(200);
			panel->addWidget(w, SARibbonPannelItem::Large);
		}

		auto* act = new QAction(QIcon(":/icons/trainline.png"), tr("运行线"), this);
		act->setToolTip(tr("高亮列车运行线\n在所有运行图窗口中，高亮本次列车运行线"));
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actShowTrainLine()));

		panel = page->addPannel(tr("分析"));

		act = new QAction(QIcon(":/icons/clock.png"), tr("事件表"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(showTrainEvents()));
		panel->addLargeAction(act);
		mw->diaActions.eventList = act;

		act = new QAction(QIcon(":/icons/line-manage.png"), tr("运行线一览"), this);
		act->setToolTip(tr("运行线一览表\n显示本次列车所有运行线基础信息及其铺画情况"));
		connect(act, SIGNAL(triggered()), this, SLOT(actShowTrainLineDialog()));
		panel->addLargeAction(act);

		act = new QAction(QIcon(":/icons/ruler.png"), tr("标尺对照"), this);
		act->setToolTip(tr("标尺对照\n将本次列车运行情况与指定线路、标尺进行对照，"
			"以确定本次列车是否符合指定标尺，或与指定标尺差异如何。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actRulerRef()));
		panel->addLargeAction(act);
		mw->diaActions.rulerRef = act;

		act = new QAction(QIcon(":/icons/identify.png"), tr("时刻诊断"), this);
		act->setToolTip(tr("列车时刻表诊断\n检查本次列车时刻表可能存在的问题，"
			"例如到开时刻填反等。"));
		connect(act, &QAction::triggered, this, &TrainContext::actDiagnose);
		panel->addLargeAction(act);

		panel = page->addPannel(tr("交路"));
		edRouting = new SARibbonLineEdit;
		edRouting->setAlignment(Qt::AlignCenter);
		edRouting->setFocusPolicy(Qt::NoFocus);
		edRouting->setFixedWidth(100);

		auto* w = new QWidget;
		auto* vlay = new QVBoxLayout(w);
		auto* hlay = new QHBoxLayout();
		hlay->addWidget(edRouting);
		act = new QAction(tr("转到交路"), this);
		connect(act, &QAction::triggered, this, &TrainContext::actToRouting);
		btnToRouting = new SARibbonToolButton(act);
		btnToRouting->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowRight));
		btnToRouting->setText("");
		hlay->addWidget(btnToRouting);
		vlay->addLayout(hlay);

		hlay = new QHBoxLayout;
		btnCreateRouting = new SARibbonToolButton();
		btnCreateRouting->setIcon(QIcon(":/icons/new-file.png"));
		btnCreateRouting->setText(tr("创建"));
		btnCreateRouting->setToolTip(tr("创建交路\n"
			"从当前列车创建新交路。默认以当前列车车次名称为交路名，仅包含当前列车。"));
		connect(btnCreateRouting, &SARibbonToolButton::clicked,
			this, &TrainContext::actCreateRouting);
		hlay->addWidget(btnCreateRouting);

		btnAddToRouting = new SARibbonToolButton;
		btnAddToRouting->setIcon(QIcon(":/icons/add.png"));
		btnAddToRouting->setText(tr("添加到.."));
		btnAddToRouting->setToolTip(tr("添加到交路\n"
			"将当前列车添加（后缀）到指定交路"));
		hlay->addWidget(btnAddToRouting);
		connect(btnAddToRouting, &SARibbonToolButton::clicked,
			this, &TrainContext::actAddToRouting);

		vlay->addLayout(hlay);
		panel->addWidget(w, SARibbonPannelItem::Large);
	}

	//编辑
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("列车编辑(&7)"));
		auto* panel = page->addPannel(tr("基本信息"));

		auto* w = new QWidget;
		auto* vlay = new QVBoxLayout;
		edNamem = new SARibbonLineEdit;
		edNamem->setAlignment(Qt::AlignCenter);
		edNamem->setToolTip(tr("列车全车次\n用以唯一识别列车，不能与其他车次重复"));
		vlay->addWidget(edNamem);

		auto* hlay = new QHBoxLayout;
		edNameDown = new SARibbonLineEdit;
		edNameDown->setAlignment(Qt::AlignCenter);
		edNameDown->setToolTip(tr("下行车次"));
		hlay->addWidget(edNameDown);
		hlay->addWidget(new QLabel("/"));
		edNameUp = new SARibbonLineEdit;
		edNameUp->setToolTip(tr("上行车次"));
		hlay->addWidget(edNameUp);
		vlay->addLayout(hlay);
		w->setLayout(vlay);
		w->setFixedWidth(200);
		panel->addWidget(w, SARibbonPannelItem::Large);
		connect(edNamem, SIGNAL(editingFinished()), this,
			SLOT(onFullNameChanged()));

		panel->addSeparator();

		w = new QWidget;
		vlay = new QVBoxLayout;
		hlay = new QHBoxLayout;
		edStartm = new SARibbonLineEdit;
		edStartm->setAlignment(Qt::AlignCenter);
		edStartm->setToolTip(tr("始发站"));
		hlay->addWidget(edStartm);
		hlay->addWidget(new QLabel("-"));
		edEndm = new SARibbonLineEdit;
		edEndm->setAlignment(Qt::AlignCenter);
		edEndm->setToolTip(tr("终到站"));
		hlay->addWidget(edEndm);
		vlay->addLayout(hlay);

		hlay = new QHBoxLayout;
		comboType = new SARibbonComboBox;
		comboType->setEditable(true);
		comboType->setToolTip(tr("列车类型\n下拉列表给出运行图目前存在的列车类型。"
			"允许编辑，如果新类型不存在，将自动添加。如果留空，则按车次判定类型。"));
		hlay->addWidget(comboType);
		checkPassen = new SARibbonCheckBox();
		checkPassen->setText(tr("旅客列车"));
		checkPassen->setTristate(true);
		checkPassen->setToolTip(tr("是否为旅客列车\n如果为半选中状态，则由列车种类判定"));
		hlay->addWidget(checkPassen);
		vlay->addLayout(hlay);

		w->setLayout(vlay);
		w->setFixedWidth(200);
		panel->addWidget(w, SARibbonPannelItem::Large);

		panel->addSeparator();

		auto* act = new QAction(tr("自动运行线"), this);
		act->setCheckable(true);
		panel->addMediumAction(act);
		btnAutoUI = panel->actionToRibbonToolButton(act);
		act->setToolTip(tr("自动配置运行线\n如果启用，则采用列车所属类型的运行线颜色、线形、宽度，"
			"否则可针对本车次单独设置。"));
		connect(act, SIGNAL(triggered(bool)), this, SLOT(onAutoUIChanged(bool)));

		act = new QAction(tr("颜色"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(actSelectColor()));
		panel->addMediumAction(act);
		btnColor = panel->actionToRibbonToolButton(act);

		w = new QWidget;
		vlay = new QVBoxLayout;

		auto* spin = new QDoubleSpinBox;
		spin->setSingleStep(0.5);
		spin->setToolTip(tr("运行线宽度"));
		spWidth = spin;
		vlay->addWidget(spin);
		comboLs = new PenStyleCombo;
		comboLs->setToolTip(tr("线型\n设置运行线线型"));

		vlay->addWidget(comboLs);
		w->setLayout(vlay);
		w->setFixedWidth(120);
		panel->addWidget(w, SARibbonPannelItem::Large);

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton), 
			tr("应用"), this);
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actApply()));

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton),
			tr("还原"), this);
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(refreshData()));

		if constexpr (true) {
			panel = page->addPannel(tr("列车径路"));
			auto* w1 = new QWidget;
			auto* hlay1 = new QHBoxLayout(w1);
			hlay1->setContentsMargins(0, 0, 0, 0);
			edPaths = new SARibbonLineEdit;
			edPaths->setAlignment(Qt::AlignCenter);
			edPaths->setReadOnly(true);
			hlay1->addWidget(edPaths);
			auto* btn1 = new SARibbonToolButton;
			connect(btn1, &SARibbonToolButton::clicked, this, &TrainContext::actFocusInPath);
			btn1->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowRight));
			btn1->setToolTip(tr("转到列车径路"));
			hlay1->addWidget(btn1);

			panel->addWidget(w1, SARibbonPannelItem::Medium);

			auto* btn = new SARibbonToolButton();
			btn->setIcon(QIcon(":/icons/add.png"));
			btn->setText(tr("添加"));
			connect(btn, &SARibbonToolButton::clicked, this, &TrainContext::actAddPaths);
			w = new QWidget;
			hlay = new QHBoxLayout(w);
			hlay->setContentsMargins(0, 0, 0, 0);
			hlay->addWidget(btn);

			act = new QAction(qApp->style()->standardIcon(QStyle::SP_TrashIcon), tr("移除"), this);
			connect(act, &QAction::triggered, this, &TrainContext::actRemovePaths);
			auto* menu = new SARibbonMenu(mw);
			menu->addAction(tr("清空"), this, &TrainContext::actClearPaths);
			act->setMenu(menu);

			btn = new SARibbonToolButton(act);
			hlay->addWidget(btn);
			panel->addWidget(w, SARibbonPannelItem::Medium);

		}

		panel = page->addPannel(tr("调整"));
		act = new QAction(QIcon(":/icons/exchange.png"), tr("区间换线"), this);
		act->setToolTip(tr("区间换线\n交换本次列车和另一列车在所选区间的运行线"));
		connect(act, SIGNAL(triggered()), this, SLOT(actExchangeInterval()));
		panel->addMediumAction(act);
		mw->diaActions.intervalExchange = act;

		act = new QAction(QIcon(":/icons/adjust.png"), tr("时刻平移"), this);
		act->setToolTip(tr("时刻平移\n将本车次部分或全部车站时刻前移或后移一段时间"));
		connect(act, SIGNAL(triggered()), this, SLOT(actAdjustTimetable()));
		panel->addMediumAction(act);
		mw->diaActions.timeAdjust = act;

		act = new QAction(QIcon(":/icons/settings.png"), tr("时刻修正"), this);
		act->setToolTip(tr("时刻修正\n修正本次列车时刻表中的一些常见类型错误，"
			"例如到发时刻排反、顺序错排等。"));
		connect(act, &QAction::triggered, this, &TrainContext::actCorrection);
		panel->addLargeAction(act);

		act = new QAction(QIcon(":/icons/add.png"), tr("快速推定"), this);
		act->setToolTip(tr("快速推定通过站时刻\n根据里程信息，快速推定本次列车在当前线路的通过站时刻，"
			"不做外插且不考虑起停附加时分。"));
		connect(act, &QAction::triggered, this, &TrainContext::actSimpleInterpolation);
		panel->addLargeAction(act);
		mw->diaActions.simpleInterp = act;

		panel = page->addPannel(tr(""));


		act = new QAction(QIcon(":/icons/timetable.png"), tr("时刻表"), this);
		act->setToolTip(tr("时刻表编辑\n显示简洁的、仅包含时刻表的编辑页面"));
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actShowBasicWidget()));

		act = new QAction(QIcon(":/icons/edit.png"), tr("编辑"), this);
		act->setToolTip(tr("列车编辑\n显示（pyETRC风格的）完整列车编辑页面"));
		panel->addLargeAction(act);
		connect(act, &QAction::triggered, this, &TrainContext::actShowEditWidget);

		act = new QAction(QIcon(":/icons/copy.png"), tr("副本"), this);
		act->setToolTip(tr("创建列车副本\n以新输入的全车次创建当前车次的副本。"));
		panel->addLargeAction(act);
		connect(act, &QAction::triggered, this, &TrainContext::actDulplicateTrain);

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
			tr("删除"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(actRemoveCurrentTrain()));
		panel->addLargeAction(act);

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
			tr("关闭面板"), this);
		act->setToolTip(tr("关闭面板\n关闭当前的列车上下文工具栏页面"));
		connect(act, &QAction::triggered, mw, &MainWindow::focusOutTrain);
		panel->addLargeAction(act);
		
	}

}

void TrainContext::setupTypeCombo()
{
	comboType->clear();
	const auto& types = diagram.trainCollection().typeCount();
	for (auto p = types.begin(); p != types.end(); ++p) {
		if (p.value() >= 0) {
			//qDebug() << "TrainContext::setupTypeCombo: type " << p.key()->name()
			//	<< " @ " << p.key().get() << " count " << p.value();
			comboType->addItem(p.key()->name());
		}
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

int TrainContext::getEditWidgetIndex(std::shared_ptr<Train> t)
{
	for (int i = 0; i < editWidgets.size(); i++) {
		auto p = editWidgets.at(i);
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

void TrainContext::updateTrainWidget(std::shared_ptr<Train> t)
{
	for (auto p : basicWidgets) {
		if (p->train() == t)
			p->refreshData();
	}
	for (auto p : editWidgets) {
		if (p->train() == t)
			p->refreshData();
	}
}

void TrainContext::updateTrainWidgetTitles(std::shared_ptr<Train> t)
{
	for (int i = 0; i < basicWidgets.size(); i++) {
		auto p = basicWidgets.at(i);
		if (p->train() == t) {
			basicDocks.at(i)->setWindowTitle(tr("时刻表编辑 - %1").arg(t->trainName().full()));
		}
	}

	for (int i = 0; i < editWidgets.size(); i++) {
		auto p = editWidgets.at(i);
		if (p->train() == t) {
			editDocks.at(i)->setWindowTitle(tr("列车编辑 - %1").arg(t->trainName().full()));
			p->refreshBasicData();
		}
	}
}

void TrainContext::refreshAllData()
{
	refreshData();
	foreach (auto p , basicWidgets) {
		p->refreshData();
	}
	foreach (auto p , editWidgets) {
		p->refreshData();
	}
}

void TrainContext::removeTrainWidget(std::shared_ptr<Train> train)
{
	removeBasicDockAt(getBasicWidgetIndex(train));
	removeEditDockAt(getEditWidgetIndex(train));
}

void TrainContext::onTrainTimetableChanged(std::shared_ptr<Train> train, std::shared_ptr<Train> table)
{
	mw->getUndoStack()->push(new qecmd::ChangeTimetable(train, table, this));
}

void TrainContext::onTrainInfoChanged(std::shared_ptr<Train> train, std::shared_ptr<Train> info)
{
	mw->getUndoStack()->push(new qecmd::UpdateTrainInfo(train, info, this));
}

void TrainContext::commitTimetableChange(std::shared_ptr<Train> train, std::shared_ptr<Train> table)
{
	//以前的adapters，拿出来做删除的索引
	//注意不能用引用，因为后面数据会被搞掉
	QVector<std::shared_ptr<TrainAdapter>> adps = std::move(train->adapters());
	train->swapTimetable(*table);
	diagram.updateTrain(train);
	updateTrainWidget(train);
	mw->updateTrainLines(train, std::move(adps));
	if (mw->timetableQuickWidget->getTrain() == train) {
		mw->timetableQuickWidget->refreshData();
	}
	if (mw->trainInfoWidget->getTrain() == train) {
		mw->trainInfoWidget->refreshData();
	}
	emit timetableChanged(train);
}

void TrainContext::onTrainStationTimeChanged(std::shared_ptr<Train> train, bool repaint)
{
	updateTrainWidget(train);
	if (repaint) {
		mw->repaintTrainLines(train);
		// 2024.03.19: repaint also the next train in the routing, if link line exists
		if (train->hasRouting()) {
			auto routw = train->routing();
			if (!routw.expired()) [[likely]] {
				auto rout = routw.lock();
				if (auto* node = rout->postLinkedByName(*train)) {
					// has post train
					mw->repaintTrainLinkLine(node->train());
				}
			}
		}
	}
	emit timetableChanged(train);
}

void TrainContext::afterTimetableChanged(std::shared_ptr<Train> train)
{
	QVector<std::shared_ptr<TrainAdapter>> adps = std::move(train->adapters());
	diagram.updateTrain(train);
	updateTrainWidget(train);
	mw->updateTrainLines(train, std::move(adps));
	emit timetableChanged(train);
}

void TrainContext::commitTraininfoChange(std::shared_ptr<Train> train, std::shared_ptr<Train> info)
{
	train->swapBaseInfo(*info);
	//2021.10.17：如果车次修改，需要处理车次映射表
	diagram.trainCollection().updateTrainInfo(train, info);
	if (train == this->train) {
		refreshData();
	}
	mw->repaintTrainLines(train);
	updateTrainWidgetTitles(train);
	emit timetableChanged(train);
}

void TrainContext::removeAllTrainWidgets()
{
	for (auto p : basicDocks) {
		p->deleteDockWidget();
	}
	basicDocks.clear();
	basicWidgets.clear();
}

void TrainContext::commitExchangeTrainInterval(std::shared_ptr<Train> train1, std::shared_ptr<Train> train2)
{
	afterTimetableChanged(train1);
	afterTimetableChanged(train2);
}

void TrainContext::actBatchChangeStartingTerminal(qecmd::StartingTerminalData& data)
{
	mw->getUndoStack()->push(new qecmd::AutoStartingTerminal(std::move(data), this));
}

void TrainContext::actBatchAutoTrainType(qecmd::AutoTrainType::data_t& data)
{
	mw->getUndoStack()->push(new qecmd::AutoTrainType(std::move(data), this));
}

void TrainContext::batchAutoTrainPen(std::deque<std::shared_ptr<Train>>& trains)
{
	mw->getUndoStack()->push(new qecmd::AutoTrainPen(std::move(trains), this));
}

void TrainContext::commitAutoStartingTerminal(qecmd::StartingTerminalData& data)
{
	data.commit();
	for (auto& p : data.startings) {
		mw->repaintTrainLines(p.first);
	}
	for (auto& p : data.terminals) {
		mw->repaintTrainLines(p.first);
	}
	mw->trainListWidget->getModel()->updateAllTrainStartingTerminal();
	refreshCurrentTrainWidgets();
}

void TrainContext::commitAutoType(std::deque<std::pair<std::shared_ptr<Train>, std::shared_ptr<TrainType>>>& data)
{
	for (auto& p : data) {
		std::swap(p.first->typeRef(), p.second);
	}
	for (const auto& p : data) {
		mw->repaintTrainLines(p.first);
	}
	mw->trainListWidget->getModel()->updateAllTrainTypes();
	diagram.trainCollection().refreshTypeCount();
	refreshCurrentTrainWidgets();
}

void TrainContext::commitAutoPenOrUndo(const std::deque<std::shared_ptr<Train>>& trains)
{
	for (const auto& p : trains) {
		mw->repaintTrainLines(p);
	}
	refreshCurrentTrainWidgets();
}

void TrainContext::actInterpolation(const QVector<std::shared_ptr<Train>>& trains, const QVector<std::shared_ptr<Train>>& data)
{
	mw->getUndoStack()->push(new qecmd::TimetableInterpolation(trains, data, this));
}

void TrainContext::commitInterpolation(const QVector<std::shared_ptr<Train>>& trains, 
	const QVector<std::shared_ptr<Train>>& data)
{
	for (int i = 0; i < trains.size(); i++) {
		auto train = trains.at(i);
		train->swapTimetable(*data.at(i));
		QVector<std::shared_ptr<TrainAdapter>> adps = std::move(train->adapters());
		diagram.updateTrain(train);
		mw->updateTrainLines(train, std::move(adps));
	}
	mw->trainListWidget->getModel()->updateAllMileSpeed();
	refreshCurrentTrainWidgets();
}

void TrainContext::actRemoveInterpolation()
{
	auto flag = QMessageBox::question(mw, tr("提示"), tr("撤销所有推定结果：此功能删除"
		"所有列车时刻表中备注为“推定”的项目，无论是否为时刻插值功能标记的。\n"
		"此操作可撤销。是否继续？"));
	if (flag != QMessageBox::Yes)
		return;
	QVector<std::shared_ptr<Train>> modified, data;
	foreach(auto train, diagram.trains()) {
		auto t = std::make_shared<Train>(*train);
		bool flag = t->removeDetected();
		if (flag) {
			modified.push_back(train);
			data.push_back(t);
		}
	}
	if (modified.isEmpty()) {
		QMessageBox::information(mw, tr("提示"), tr("操作完成，没有列车受到影响。"));
	}
	else {
		QMessageBox::information(mw, tr("提示"), tr("操作完成，共%1个车次受到影响。")
			.arg(modified.size()));
		mw->getUndoStack()->push(new qecmd::RemoveInterpolation(modified, data, this));
	}
	
}

void TrainContext::locateToBoundStation(const QVector<TrainStationBounding>& boudings, 
	const QTime& time)
{
	// 入参已经保证非空
	if (boudings.size() == 1) {
		// 没得选，考虑直接执行
		const auto& b = boudings.front();
		auto line = b.line.lock();
		if (!line) {
			QMessageBox::warning(mw, tr("错误"), tr("非法状态：意外的空TrainLine指针"));
			return;
		}
		auto rail = line->railway();
		if (!rail) {
			QMessageBox::warning(mw, tr("错误"), tr("非法状态：意外的空Railway指针"));
			return;
		}
		auto st = b.railStation.lock();
		if (!st) {
			QMessageBox::warning(mw, tr("错误"), tr("非法状态：意外的空RailStation指针"));
		}
		int idx = PageComboForRail::dlgGetPageIndex(diagram, rail, mw, tr("选择运行图"),
			tr("当前铺画点可能存在于多张运行图中。请选择要定位到的运行图："));
		if (idx != -1) {
			mw->locateDiagramOnStation(idx, rail, st, time);
		}
	}
	else  {
		if (!dlgLocate) {
			dlgLocate = new LocateBoundingDialog(diagram, mw);
			connect(dlgLocate, &LocateBoundingDialog::locateOnStation,
				mw, &MainWindow::locateDiagramOnStation);
		}
		dlgLocate->showForStation(boudings, time);
	}

}

void TrainContext::actAutoBusiness()
{
	actAutoBusinessBat(diagram.trains());
}

void TrainContext::actAutoBusinessBat(const QList<std::shared_ptr<Train>>& trainRange)
{
	QVector<std::shared_ptr<Train>> modified, data;
	foreach(auto train, trainRange) {
		auto t = std::make_shared<Train>(*train);
		diagram.updateTrain(t);
		bool flag = t->autoBusiness();
		if (flag) {
			modified.push_back(train);
			data.push_back(t);
		}
		else {
			// 析构掉
			t->clearBoundRailways();
			t.reset();
		}
	}
	if (modified.empty()) {
		QMessageBox::information(mw, tr("提示"), tr("应用完成，没有更改被执行"));
	}
	else {
		int sz = modified.size();
		mw->getUndoStack()->push(new qecmd::AutoBusiness(std::move(modified), std::move(data),
			this));
		QMessageBox::information(mw, tr("提示"), tr("应用完成，共%1个车次受到影响。").arg(sz));
	}
}

void TrainContext::actAutoCorrectionAll()
{
	actAutoCorrectionBat(diagram.trains());
}

void TrainContext::actAutoCorrectionBat(const QList<std::shared_ptr<Train>>& trainRange)
{
	auto res = QMessageBox::question(mw, tr("自动批量更正"),
		tr("此功能以内置算法，尝试自动更正时刻表中可能的顺序错误问题。\n"
			"请注意此功能未经过充分测试，不一定能解决问题。建议做好数据保存和备份。\n"
			"是否继续？"));
	if (res != QMessageBox::Yes)
		return;
	QVector<std::shared_ptr<Train>> modified, data;

	auto clk_beg = std::chrono::system_clock::now();

	foreach(auto train, trainRange) {
		auto t = std::make_shared<Train>(*train);
		bool flag = TimetableCorrector::autoCorrectSafe(t);
		if (flag) {
			//qDebug() << "Auto correct " << train->trainName().full()<<" "<< train.get() << Qt::endl;
			//qDebug() << "Data at: " << t.get();
			//train->show();

			modified.push_back(train);
			data.push_back(t);
		}
		else {
			// 析构掉
			t.reset();
		}
	}


	auto clk_end = std::chrono::system_clock::now();

	using namespace std::chrono_literals;
	mw->showStatus(QObject::tr("自动时刻表更正  用时 %1 毫秒").arg((clk_end - clk_beg) / 1ms));

	if (modified.empty()) {
		QMessageBox::information(mw, tr("提示"), tr("应用完成，没有更改被执行"));
	}
	else {
		int sz = modified.size();
		mw->getUndoStack()->push(new qecmd::BatchAutoCorrection(std::move(modified), std::move(data),
			this));
		QMessageBox::information(mw, tr("提示"), tr("应用完成，共%1个车次受到影响。").arg(sz));
	}
}

void TrainContext::actRemoveNonBound()
{
	auto res = QMessageBox::question(mw, tr("删除未铺画车站"),
		tr("删除所有列车时刻表中，没有对应到本运行图文件中线路上的车站。\n"
			"是否继续？"));
	if (res != QMessageBox::Yes)
		return;

	QVector<std::shared_ptr<Train>> modified, data;

	foreach(auto train, diagram.trains()) {
		auto t = std::make_shared<Train>(*train);
		diagram.updateTrain(t);
		bool flag = t->removeNonBound();
		if (flag) {
			modified.push_back(train);
			data.push_back(t);
		}
		else {
			// 析构掉
			t.reset();
		}
	}


	if (modified.empty()) {
		QMessageBox::information(mw, tr("提示"), tr("应用完成，没有更改被执行"));
	}
	else {
		int sz = modified.size();
		mw->getUndoStack()->push(new qecmd::BatchAutoCorrection(std::move(modified), std::move(data),
			this));
		QMessageBox::information(mw, tr("提示"), tr("应用完成，共%1个车次受到影响。").arg(sz));
	}

}

void TrainContext::actRemoveNonBoundTrains()
{
	auto res = QMessageBox::question(mw, tr("删除未铺画车次"),
		tr("删除本运行图中所有没有铺画任何运行线的车次。\n"
			"是否继续？"));
	if (res != QMessageBox::Yes)
		return;

	QList<std::shared_ptr<Train>> trains;
	QList<int> indexes;
	for (int i = 0; i < diagram.trains().size(); i++) {
		auto train = diagram.trains().at(i);
		if (train->adapters().empty()) {
			trains.push_back(train);
			indexes.push_back(i);
		}
	}

	if (trains.empty()) {
		QMessageBox::information(mw, tr("提示"), tr("没有更改被执行"));
	}
	else {
		mw->getUndoStack()->push(new qecmd::RemoveTrains(trains, indexes, diagram.trainCollection(),
			mw->trainListWidget->getModel(), this));
		QMessageBox::information(mw, tr("提示"), tr("已删除%1个车次").arg(trains.size()));
	}
}

void TrainContext::actRemoveEmptyTrains()
{
	auto res = QMessageBox::question(mw, tr("删除空白车次"),
		tr("删除本运行图中所有时刻表为空白的车次。\n"
			"是否继续？"));
	if (res != QMessageBox::Yes)
		return;

	QList<std::shared_ptr<Train>> trains;
	QList<int> indexes;
	for (int i = 0; i < diagram.trains().size(); i++) {
		auto train = diagram.trains().at(i);
		if (train->timetable().empty()) {
			trains.push_back(train);
			indexes.push_back(i);
		}
	}

	if (trains.empty()) {
		QMessageBox::information(mw, tr("提示"), tr("没有更改被执行"));
}
	else {
		mw->getUndoStack()->push(new qecmd::RemoveTrains(trains, indexes, diagram.trainCollection(),
			mw->trainListWidget->getModel(), this));
		QMessageBox::information(mw, tr("提示"), tr("已删除%1个车次").arg(trains.size()));
	}
}

#if 0
void TrainContext::commitAutoBusiness(const QVector<std::shared_ptr<Train>>& trains)
{
	//refreshCurrentTrainWidgets();
	// 2022.05.28：swapTimetable操作导致TrainStation地址变了，所以必须重新铺画，才是安全的
	commitAutoCorrection(trains);
}

void TrainContext::commitAutoCorrection(const QVector<std::shared_ptr<Train>>& trains)
{
	refreshCurrentTrainWidgets();
	// 2022.05.28：由于Adapters的地址实际上变了，
	// 删除旧运行线的算法是错误的。干脆全部重新铺画算了
	mw->updateAllDiagrams();
}
#endif

#include "navi/navitree.h"

void TrainContext::actImportTrainFromCsv()
{
	auto flag = QMessageBox::question(mw, tr("导入CSV时刻表"), tr("此功能提供从CSV（逗号分隔值）"
		"格式导入时刻表。所给文件应当有4至6列，分别为车次、站名、到达时刻、出发时刻、"
		"股道（可选）、备注（可选），不需要表头；文件应当采用UTF-8编码。\n"
		"每行视为一个时刻数据。对既有车次，无条件、按顺序追加到原有时刻表之后；对新增车次，"
		"以所给时刻数据创建新车次。是否确认？"));
	if (flag != QMessageBox::Yes)return;
	QString filename = QFileDialog::getOpenFileName(mw, tr("从CSV导入时刻表"), {},
		tr("逗号分隔值 (*.csv)\n所有文件 (*)"));
	if (filename.isEmpty()) return;

	QFile file(filename);
	file.open(QFile::ReadOnly);
	if (!file.isOpen()) {
		QMessageBox::warning(mw, tr("错误"), tr("打开文件失败"));
		return;
	}
	QTextStream ts(&file);
	//ts.setCodec("utf-8");

	QVector<std::shared_ptr<Train>> newTrains, modifiedTrains, modifiedData;
	QMap<QString, std::shared_ptr<Train>> trainMap;   // 这里存的是要更新的对象，副本或新增的

	auto& coll = diagram.trainCollection();
	
	int valid = 0;
	while (!ts.atEnd()) {
		QString line = ts.readLine();
		if (line.isEmpty()) continue;
		auto splitted = line.split(',');
		if (splitted.size() < 4)
			continue;
		QString trainName = splitted.at(0);
		std::shared_ptr<Train> toAddTrain{};

		if (auto itr = trainMap.find(trainName); itr != trainMap.end()) {
			toAddTrain = itr.value();
		}
		else {
			// 第一次读到这个车次；分为新建和修改两种情况。
			if (auto train = coll.findFullName(trainName)) {
				// 修改既有车次  添加
				toAddTrain = std::make_shared<Train>(*train);   // deep copy construct

				modifiedTrains.push_back(train);
				modifiedData.push_back(toAddTrain);
			}
			else {
				// 新增的车次
				toAddTrain = std::make_shared<Train>(trainName);
				newTrains.push_back(toAddTrain);
			}
			trainMap.insert(trainName, toAddTrain);
		}

		// 现在：新读取的时刻添加到toAddTrain所示对象中
		QString stationName = splitted.at(1);
		QTime arrive = qeutil::parseTime(splitted.at(2));
		QTime depart = qeutil::parseTime(splitted.at(3));
		if (!arrive.isValid() || !depart.isValid())
			continue;
		QString track, note;
		if (splitted.size() >= 5)
			track = splitted.at(4);
		if (splitted.size() >= 6)
			note = splitted.at(5);
		toAddTrain->appendStation(stationName, arrive, depart, true, track, note);
		valid++;
	}

	for (auto p = newTrains.begin(); p != newTrains.end();) {
		if ((*p)->empty()) {
			p = newTrains.erase(p);
		}
		else {
			diagram.updateTrain(*p);
			++p;
		}
	}

	if (valid) {
		// 新增的
		if(!newTrains.empty())
			mw->naviView->actBatchAddTrains(newTrains);
		// 修订的
		if (!modifiedTrains.empty()) {
			auto* cmd = new qecmd::TimetableInterpolation(modifiedTrains, modifiedData, this);
			cmd->setText(tr("批量修改列车时刻表"));
			mw->getUndoStack()->push(cmd);
		}

	}
	else {
		QMessageBox::information(mw, tr("导入CSV时刻表"), tr("无有效数据导入。"));
	}
}

void TrainContext::actImportTrainFromTrf()
{
	auto flag = QMessageBox::question(mw, tr("从trf导入列车"), tr("此功能提供从ETRC的列车描述文件(*.trf)中导入车次。"
		"请使用新版ETRC (版本号3.0以后) 所使用的的文件格式。"
		"每个文件仅包含一个车次，可以批量选择多个文件同时导入。请自行确保所导入的车次中，全车次互不重复，否则可能引发" 
		"未定义行为。\n" 
		"目前暂不支持交路读取。\n是否继续？"));
	if (flag != QMessageBox::Yes)return;
	QStringList files = QFileDialog::getOpenFileNames(mw, tr("从trf导入时刻表"), {},
		tr("ETRC列车描述文件 (*.trf)\n所有文件 (*)"));
	if (files.isEmpty()) return;

	QVector<std::shared_ptr<Train>> newTrains;
	foreach(const QString & filename, files) {
		auto train = Train::fromTrf(filename);
		if (train) {
			const auto& t = diagram.trainCollection().validTrainFullName(train->trainName().full());
			train->trainName().setFull(t.full());
			newTrains.push_back(train);
			diagram.updateTrain(train);
			train->setType(diagram.trainCollection().typeManager().fromRegex(train->trainName()));
		}
	}

	if (!newTrains.empty()) {
		mw->naviView->actBatchAddTrains(newTrains);
		QMessageBox::information(mw, tr("导入trf文件"), tr("成功读入%1个车次。").arg(newTrains.size()));
	}
	else {
		QMessageBox::information(mw, tr("导入trf文件"), tr("无有效数据读入"));
	}
		
}

void TrainContext::batchExportTrainEvents(const QList<std::shared_ptr<Train>>& trains)
{
	auto flag = QMessageBox::question(mw, tr("批量导出列车事件表"), tr("将所选车次事件表导出到"
		"指定的一个CSV文件中，所有数据顺次排列在同一张表中。\n"
		"是否确认？"));
	if (flag != QMessageBox::Yes)return;
	QString filename = QFileDialog::getSaveFileName(mw, tr("批量导出事件表"), {},
		tr("逗号分隔值 (*.csv)\n所有文件 (*)"));
	if (filename.isEmpty()) return;

	QFile file(filename);
	file.open(QFile::WriteOnly);
	if (!file.isOpen())return;
	QTextStream s(&file);

	// 标题: 
	// 车次  线名  时间  地点  里程  事件  客体  备注
	s << tr("车次,线名,时间,地点,里程,事件,客体,备注\n");

	using namespace std::chrono_literals;

	auto clk_s = std::chrono::system_clock::now();

	foreach(auto train, trains) {
		TrainEventList evlst = diagram.listTrainEvents(*train);
		TrainEventModel::exportToCsvBatch(s, evlst);
	}

	file.close();

	auto clk_e = std::chrono::system_clock::now();

	mw->showStatus(tr("批量导出事件表 用时 %1 ms").arg((clk_e - clk_s) / 1ms));
}

void TrainContext::actSimpleInterpolation()
{
	if (!train) {
		QMessageBox::warning(mw, tr("快速推定"), tr("错误：当前没有选中车次！"));
		return;
	}
	auto rail = mw->getRailContext()->getRailway();
	if (!rail) {
		QMessageBox::warning(mw, tr("快速推定"), tr("错误：当前没有选中线路！"));
	}

	auto adp = train->adapterFor(*rail);
	if (adp) {
		auto tab = std::make_shared<Train>(*train);
		diagram.updateTrain(tab);
		auto nadp = tab->adapterFor(*rail);

		int cnt = nadp->timetableInterpolationSimple();
		if (cnt) {
			mw->getUndoStack()->push(new qecmd::TimetableInterpolationSimple(train, tab, this,
				rail.get()));
			mw->showStatus(tr("快速推定列车[%1]在线路[%2]上的通过站时刻 添加%3个时刻信息").arg(train->trainName().full(),
				rail->name(), QString::number(cnt)));
		}
		else {
			mw->showStatus(tr("快速推定: 列车[%1]在线路[%2]上没有需要推定的车站").arg(train->trainName().full(),
				rail->name()));
		}
	}
	else {
		mw->showStatus(tr("当前车次[%1]在线路[%2]上没有运行线，无法推定时刻").arg(train->trainName().full(),
			rail->name()));
	}
}

void TrainContext::actDragTimeSingle(std::shared_ptr<Train> train, int station_id, const TrainStation& data, 
	Qt::KeyboardModifiers mod)
{
	mw->getUndoStack()->push(new qecmd::DragTrainStationTime(train, station_id, mod, data, this));
}

void TrainContext::actDragTimeNonLocal(std::shared_ptr<Train> train, int station_id, std::shared_ptr<Train> data, 
	Qt::KeyboardModifiers mod)
{
	mw->getUndoStack()->push(new qecmd::DragNonLocalTime(train, data, station_id, mod, this));
}

void TrainContext::afterChangeTrainPaths(std::shared_ptr<Train> train, const std::vector<TrainPath*>& paths)
{
	// rebind the train, and repaint train line
	afterTimetableChanged(train);
	if (train == this->train) {
		refreshPath();
	}

	// update path related data (currently nothing)
}

void TrainContext::afterChangeTrainPaths(std::shared_ptr<Train> train, const std::vector<qecmd::PathInfoInTrain>& paths)
{
	// rebind the train, and repaint train line
	afterTimetableChanged(train);
	if (train == this->train) {
		refreshPath();
	}

	// update path related data (currently nothing)
}

void TrainContext::actRemoveTrains(const QList<std::shared_ptr<Train>>& trains, const QList<int>& indexes)
{
	auto* s = sender();
	if (s == mw->trainListWidget) {
		mw->getUndoStack()->push(new qecmd::RemoveTrains(trains, indexes, diagram.trainCollection(),
			mw->trainListWidget->getModel(), this));
	}
	else [[unlikely]] {
		qCritical() << "unexpected sender " << s;
	}
}

void TrainContext::actRemoveSingleTrain(int index)
{
	mw->getUndoStack()->push(new qecmd::RemoveSingleTrain(this, mw->naviModel, diagram.trains().at(index),
		index));
}

void TrainContext::afterTrainsReboundByPath(const std::vector<std::shared_ptr<Train>>& trains,
	const std::vector<QVector<std::shared_ptr<TrainAdapter>>>& adapters)
{
	for (size_t i = 0; i < trains.size(); i++) {
		auto adpi = adapters.at(i);    // copy
		mw->updateTrainLines(trains.at(i), std::move(adpi));
	}
}

void TrainContext::actShowTrainLine()
{
	//强制显示列车运行线。如果已经全部显示了也没关系，没有任何效果
	mw->getViewCategory()->actChangeSingleTrainShow(train, true);
	emit highlightTrainLine(train);
}

void TrainContext::actShowBasicWidget()
{
	showBasicWidget(this->train);
}

void TrainContext::showBasicWidget(std::shared_ptr<Train> train)
{
	int idx = getBasicWidgetIndex(train);
	if (idx == -1) {
		//创建
		auto* w = new BasicTrainWidget(diagram.trainCollection(), false);
		w->setTrain(train);
		auto* dock = new ads::CDockWidget(tr("时刻表编辑 - %1").arg(train->trainName().full()));
		dock->setWidget(w);
		//dock->setAttribute(Qt::WA_DeleteOnClose);
		connect(dock, SIGNAL(closed()), this, SLOT(onTrainDockClosed()));
		connect(w->timetableModel(), &TimetableStdModel::timetableChanged,
			this, &TrainContext::onTrainTimetableChanged);
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

void TrainContext::actShowEditWidget()
{
	if (train)
		showEditWidget(train);
}

void TrainContext::showEditWidget(std::shared_ptr<Train> train)
{
	int idx = getEditWidgetIndex(train);
	if (idx == -1) {
		//创建
		auto* w = new EditTrainWidget(diagram.trainCollection(), train);
		w->setTrain(train);
		auto* dock = new ads::CDockWidget(tr("列车编辑 - %1").arg(train->trainName().full()));
		dock->setWidget(w);
		dock->resize(600, 800);
		connect(dock, &ads::CDockWidget::closed, this, &TrainContext::onEditDockClosed);
		connect(w->getModel(), &TimetableStdModel::timetableChanged,
			this, &TrainContext::onTrainTimetableChanged);
		connect(w, &EditTrainWidget::trainInfoChanged,
			this, &TrainContext::onTrainInfoChanged);
		connect(w, &EditTrainWidget::switchToRouting,
			mw, &MainWindow::focusInRouting);
		connect(w, &EditTrainWidget::removeTrain,
			this, &TrainContext::actRemoveTrainFromEdit);
		mw->getManager()->addDockWidgetFloating(dock);
		editWidgets.append(w);
		editDocks.append(dock);
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

void TrainContext::onEditDockClosed()
{
	auto* dock = static_cast<ads::CDockWidget*>(sender());
	if (dock) {
		for (int i = 0; i < editDocks.size(); i++) {
			if (editDocks.at(i) == dock) {
				removeEditDockAt(i);
				break;
			}
		}
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

void TrainContext::removeEditDockAt(int idx)
{
	if (idx == -1)
		return;
	auto* dock = editDocks.takeAt(idx);
	editWidgets.removeAt(idx);
	dock->deleteDockWidget();
}

void TrainContext::onFullNameChanged()
{
	TrainName tn(edNamem->text());
	edNameDown->setText(tn.down());
	edNameUp->setText(tn.up());
}

void TrainContext::onAutoUIChanged(bool on)
{
	btnColor->setEnabled(!on);
	comboLs->setEnabled(!on);
	spWidth->setEnabled(!on);
}

void TrainContext::actSelectColor()
{
	QColor color(tmpColor);
	if (!color.isValid() && train)
		color = train->pen().color();
	color = QColorDialog::getColor(color, mw, tr("选择运行线颜色"));
	if (color.isValid()) {
		tmpColor = color;
		btnColor->setText(tmpColor.name());
	}
}

void TrainContext::actApply()
{
	if (!train)return;
	//生成新的列车对象，新对象不包含时刻表信息，专门作为装基础信息的容器
	TrainName name(edNamem->text(), edNameDown->text(), edNameUp->text());
	if (!diagram.trainCollection().trainNameIsValid(name, train)) {
		QMessageBox::warning(mw, tr("错误"),
			tr("全车次不能为空或与其他车次冲突，请重新编辑。"));
		return;
	}
	auto t = std::make_shared<Train>(name);
	t->setStarting(StationName::fromSingleLiteral(edStartm->text()));
	t->setTerminal(StationName::fromSingleLiteral(edEndm->text()));

	auto& mana = diagram.trainCollection().typeManager();
	const QString& tps = comboType->currentText();
	std::shared_ptr<TrainType> tp;
	if (tps.isEmpty()) {
		tp = mana.fromRegex(name);
	}
	else {
		tp = mana.findOrCreate(tps);
	}
	t->setType(tp);
	t->setPassenger(static_cast<TrainPassenger>(checkPassen->checkState()));

	bool autopen = btnAutoUI->isChecked();
	if (autopen) {
		t->resetPen();
	}
	else {
		QColor color = tmpColor;
		if (!color.isValid())
			color = train->pen().color();
		QPen pen = QPen(color, spWidth->value(), static_cast<Qt::PenStyle>(comboLs->currentIndex()));
		t->setPen(pen);
	}
	mw->getUndoStack()->push(new qecmd::UpdateTrainInfo(train, t, this));
}

void TrainContext::refreshData()
{
	if (train) {
		edName->setText(train->trainName().full());
		edStart->setText(train->starting().toSingleLiteral());
		edEnd->setText(train->terminal().toSingleLiteral());
		if (train->hasRouting()) {
			edRouting->setText(train->routing().lock()->name());
			btnToRouting->setEnabled(true);
			btnCreateRouting->setEnabled(false);
			btnAddToRouting->setEnabled(false);
		}
		else {
			edRouting->setText(tr("(无交路)"));
			btnToRouting->setEnabled(false);
			btnCreateRouting->setEnabled(true);
			btnAddToRouting->setEnabled(true);
		}
		refreshPath();
		edNamem->setText(train->trainName().full());
		edStartm->setText(train->starting().toSingleLiteral());
		edEndm->setText(train->terminal().toSingleLiteral());
		edNameDown->setText(train->trainName().down());
		edNameUp->setText(train->trainName().up());
		setupTypeCombo();
		comboType->setCurrentText(train->type()->name());
		checkPassen->setCheckState(static_cast<Qt::CheckState>(train->passenger()));
		btnAutoUI->setChecked(train->autoPen());
		onAutoUIChanged(train->autoPen());
		const QPen& pen = train->pen();
		btnColor->setText(pen.color().name());
		spWidth->setValue(pen.widthF());
		comboLs->setCurrentIndex(static_cast<int>(pen.style()));
	}
}

void TrainContext::refreshPath()
{
	if (!train)return;
	if (train->paths().empty()) {
		edPaths->setText(tr("(自动)"));
	}
	else if (train->paths().size() == 1) {
		edPaths->setText(train->paths().front()->name());
	}
	else {
		edPaths->setText(tr("%1 等%2项").arg(train->paths().front()->name()).arg(train->paths().size()));
	}
}

void TrainContext::actRemoveCurrentTrain()
{
	if (train) {
		int idx = diagram.trainCollection().getTrainIndex(train);
		if (idx != -1) {
			actRemoveSingleTrain(idx);
		}
	}
}


void TrainContext::actRemoveTrainFromEdit(std::shared_ptr<Train> train)
{
	if (train) {
		int idx = diagram.trainCollection().getTrainIndex(train);
		if (idx != -1) {
			actRemoveSingleTrain(idx);
		}
	}
}

void TrainContext::actShowTrainLineDialog()
{
	auto* dialog = new TrainLineDialog(train, mw);
	dialog->show();
}

void TrainContext::actRulerRef()
{
	if (!train) {
		QMessageBox::warning(mw, tr("错误"), tr("标尺对照：当前没有选中车次！"));
		return;
	}
	auto* dialog = new RulerRefDialog(train, mw);
	dialog->open();
}

void TrainContext::actExchangeInterval()
{
	if (!train) {
		QMessageBox::warning(mw, tr("错误"), tr("区间换线：当前没有选中车次！"));
		return;
	}
	auto* dialog = new ExchangeIntervalDialog(diagram.trainCollection(), train, mw);
	connect(dialog, &ExchangeIntervalDialog::exchangeApplied,
		this, &TrainContext::actApplyExchangeInterval);
	dialog->open();
}

void TrainContext::actAutoPenAll()
{
	std::deque<std::shared_ptr<Train>> trains{};
	foreach(auto t, diagram.trainCollection().trains()) {
		if (!t->autoPen()) {
			trains.emplace_back(t);
		}
	}
	batchAutoTrainPen(trains);
}

void TrainContext::actApplyExchangeInterval(
	std::shared_ptr<Train> train1, std::shared_ptr<Train> train2, 
	Train::StationPtr start1, Train::StationPtr end1,
	Train::StationPtr start2, Train::StationPtr end2, 
	bool includeStart, bool includeEnd)
{
	mw->getUndoStack()->push(new qecmd::ExchangeTrainInterval(train1, train2,
		start1, end1, start2, end2, includeStart, includeEnd, this));
}

void TrainContext::actAdjustTimetable()
{
	if (!train) {
		QMessageBox::warning(mw, tr("错误"), tr("时刻表调整：当前没有选中车次！"));
		return;
	}
	auto* dialog = new ModifyTimetableDialog(train, mw);
	connect(dialog, &ModifyTimetableDialog::trainUpdated,
		this, &TrainContext::onTrainTimetableChanged);
	dialog->show();
}

void TrainContext::actDiagnose()
{
	auto* d = new DiagnosisDialog(diagram, train, mw);
	connect(d, &DiagnosisDialog::showStatus,
		mw, &MainWindow::showStatus);
	connect(d->getModel(), &DiagnosisModel::locateToRailMile,
		mw, &MainWindow::locateToRailMile);
	d->show();
}

void TrainContext::actToRouting()
{
	if (train && train->hasRouting()) {
		emit focusInRouting(train->routing().lock());
	}
}

void TrainContext::refreshCurrentTrainWidgets()
{
	refreshAllData();
	mw->trainInfoWidget->refreshData();
	mw->timetableQuickWidget->refreshData();
}

void TrainContext::actCorrection()
{
	if (!train)return;
	auto* dlg = new CorrectTimetableDialog(train, mw);
	connect(dlg, &CorrectTimetableDialog::correctionApplied,
		this, &TrainContext::onTrainTimetableChanged);
	dlg->show();
}

void TrainContext::actDulplicateTrain()
{
	if (!train)return;
	emit dulplicateTrain(train);
}

void TrainContext::actAddPaths()
{
	auto paths = SelectPathDialog::getPaths(mw, tr("请选择要添加到本次列车的列车径路"),
		diagram.pathCollection().unassignedPaths(*train));
	mw->getUndoStack()->push(new qecmd::AssignPathsToTrain(train, std::move(paths), this));
}

void TrainContext::actRemovePaths()
{
	auto indexes = SelectPathDialog::getPathIndexes(mw, tr("请选择要移除的列车径路"),
		train->paths());
	std::vector<qecmd::PathInfoInTrain> data{};
	for (int idx : indexes) {
		auto* p = train->paths().at(idx);
		int idx_train = p->getTrainIndex(train);
		if (idx_train == -1) [[unlikely]] {
			qCritical() << "idx_train==-1  " << train->trainName().full();
			continue;
		}
		data.emplace_back(p, idx, idx_train);
	}

	mw->getUndoStack()->push(new qecmd::RemovePathsFromTrain(train, std::move(data), this));
}

void TrainContext::actClearPaths()
{
	mw->getUndoStack()->push(new qecmd::ClearPathsFromTrain(train, this));
}

void TrainContext::actFocusInPath()
{
	const auto& paths = train->paths();
	if (paths.empty()) {
		return;
	}
	else if (paths.size() == 1) {
		mw->focusInPath(paths.front());
	}
	else {
		auto indexes = SelectPathDialog::getPathIndexes(mw, tr("请选择要转到的列车径路"),
			train->paths());
		if (!indexes.empty()) {
			mw->focusInPath(train->paths().at(indexes.front()));
		}
	}
}

void TrainContext::actCreateRouting()
{
	if (!train || train->hasRouting()) {
		return;
	}
	mw->contextRouting->createRoutingByTrain(train);
}

void TrainContext::actAddToRouting()
{
	if (!train || train->hasRouting()) {
		return;
	}
	auto o = SelectRoutingDialog::selectRouting(diagram.trainCollection(), true, mw);
	if (!o.isAccepted) {
		return;
	}
	else if (o.createNew) {
		actCreateRouting();
	}
	else {
		auto rout = o.routing;
		mw->contextRouting->addTrainToRouting(o.routing, train);
	}
}

void TrainContext::setTrain(std::shared_ptr<Train> train_)
{
	train = train_;
	refreshData();

	// 2023.01.24: also show train line  
	if (SystemJson::instance.auto_highlight_on_selected)
		actShowTrainLine();
}

void TrainContext::showTrainEvents()
{
	using namespace std::chrono_literals;
	if (!train) {
		QMessageBox::warning(mw, tr("错误"), tr("列车事件表：没有选中车次！"));
		return;
	}
	auto start = std::chrono::system_clock::now();
	auto* dialog = new TrainEventDialog(diagram, train, mw);
	connect(dialog, &TrainEventDialog::locateToEvent,
		mw, &MainWindow::locateDiagramOnMile);
	auto end = std::chrono::system_clock::now();
	mw->showStatus(tr("%1 列车事件表  用时%2毫秒").arg(train->trainName().full())
		.arg((end - start) / 1ms));
	dialog->show();
}

qecmd::ChangeTimetable::ChangeTimetable(std::shared_ptr<Train> train_,
	std::shared_ptr<Train> newtable, TrainContext* context, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("更新时刻表: ") + train_->trainName().full(), parent),
	train(train_), table(newtable), cont(context)
{
}

void qecmd::ChangeTimetable::undo()
{
	cont->commitTimetableChange(train, table);
}

void qecmd::ChangeTimetable::redo()
{
	cont->commitTimetableChange(train, table);
}

void qecmd::ExchangeTrainInterval::undo()
{
	commit();
}

void qecmd::ExchangeTrainInterval::redo()
{
	commit();
}

void qecmd::ExchangeTrainInterval::commit()
{
	train1->intervalExchange(*train2, start1, end1, start2, end2, includeStart, includeEnd);
	std::swap(start1, start2);
	std::swap(end1, end2);
	cont->commitExchangeTrainInterval(train1, train2);
}

void qecmd::StartingTerminalData::commit()
{
	for (auto& p : startings) {
		std::swap(p.first->startingRef(), p.second);
	}
	for (auto& p : terminals) {
		std::swap(p.first->terminalRef(), p.second);
	}
}

void qecmd::AutoStartingTerminal::commit()
{
	cont->commitAutoStartingTerminal(data);
}

void qecmd::AutoTrainType::undo()
{
	commit();
}
void qecmd::AutoTrainType::redo()
{
	commit();
}

void qecmd::AutoTrainType::commit()
{
	cont->commitAutoType(data);
}

qecmd::AutoTrainPen::AutoTrainPen(std::deque<std::shared_ptr<Train>>&& trains, 
	TrainContext* context, QUndoCommand* parent):
	QUndoCommand(QObject::tr("自动设置运行线"), parent), _trains(std::move(trains)),
	_pens(), cont(context)
{
	for (const auto& t : _trains) {
		assert(!t->autoPen());
		_pens.emplace_back(t->pen());
	}
}

void qecmd::AutoTrainPen::undo()
{
	for (size_t i = 0; i < _trains.size(); i++) {
		_trains.at(i)->setPen(_pens.at(i));
	}
	cont->commitAutoPenOrUndo(_trains);
}

void qecmd::AutoTrainPen::redo()
{
	for (size_t i = 0; i < _trains.size(); i++) {
		_trains.at(i)->resetPen();
	}
	cont->commitAutoPenOrUndo(_trains);
}

void qecmd::TimetableInterpolation::undo()
{
	cont->commitInterpolation(trains, data);
}

void qecmd::TimetableInterpolation::redo()
{
	cont->commitInterpolation(trains, data);
}

void qecmd::RemoveInterpolation::undo()
{
	cont->commitInterpolation(trains, data);
}
void qecmd::RemoveInterpolation::redo()
{
	cont->commitInterpolation(trains, data);
}

void qecmd::AutoBusiness::undo()
{
	commit();
}

void qecmd::AutoBusiness::redo()
{
	commit();
}

void qecmd::AutoBusiness::commit()
{
	//for (int i = 0; i < trains.size(); i++) {
	//	auto train = trains.at(i);
	//	auto d = data.at(i);
	//	train->swapTimetableWithAdapters(*d);
	//}
	//cont->commitAutoBusiness(trains);
	cont->commitInterpolation(trains, data);
}

void qecmd::BatchAutoCorrection::undo()
{
	commit();
}

void qecmd::BatchAutoCorrection::redo()
{
	commit();
}

void qecmd::BatchAutoCorrection::commit()
{
	cont->commitInterpolation(trains, data);
	//for (int i = 0; i < trains.size(); i++) {
	//	auto train = trains.at(i);
	//	auto d = data.at(i);
	//	train->swapTimetableWithAdapters(*d);
	//}
	//cont->commitAutoCorrection(trains);
}

qecmd::TimetableInterpolationSimple::TimetableInterpolationSimple(std::shared_ptr<Train> train,
	std::shared_ptr<Train> newtable, TrainContext* context, const Railway* rail, QUndoCommand* parent):
	ChangeTimetable(train,newtable,context,parent)
{
	setText(QObject::tr("快速推定: %1 @ %2").arg(train->trainName().full(), rail->name()));
}

qecmd::DragTrainStationTime::DragTrainStationTime(std::shared_ptr<Train> train, int station_id,
	Qt::KeyboardModifiers mod, const TrainStation& data, TrainContext* cont, QUndoCommand* parent):
	QUndoCommand(QObject::tr("拖动时刻: %1 @ %2").arg(data.name.toSingleLiteral(), train->trainName().full()),parent),
	train(train),station_id(station_id), mod(mod), data(data),cont(cont)
{
}

void qecmd::DragTrainStationTime::undo()
{
	commit();
}

void qecmd::DragTrainStationTime::redo()
{
	if (first) {
		first = false;
		cont->onTrainStationTimeChanged(train, true);
		return;
	}
	commit();
}

bool qecmd::DragTrainStationTime::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())return false;
	auto cmd = static_cast<const DragTrainStationTime*>(other);
	// 2024.03.26  new requirement: same modifiers
	if (train == cmd->train && station_id == cmd->station_id && mod == cmd->mod) {
		// 抛弃中间状态即可
		return true;
	}
	else return false;
}

void qecmd::DragTrainStationTime::commit()
{
	auto itr = train->timetable().begin();
	std::advance(itr, station_id);
	std::swap(*itr, data);
	cont->onTrainStationTimeChanged(train, true);
}

qecmd::DragNonLocalTime::DragNonLocalTime(std::shared_ptr<Train> train, std::shared_ptr<Train> data,
	int station_id, Qt::KeyboardModifiers mod, TrainContext* cont, QUndoCommand* parent):
	QUndoCommand(QObject::tr("拖动平移: %1").arg(train->trainName().full()), parent), 
	train(train), data(data),
	station_id(station_id), mod(mod), cont(cont)
{
}

void qecmd::DragNonLocalTime::undo()
{
	std::ranges::swap_ranges(train->timetable(), data->timetable());
	cont->onTrainStationTimeChanged(train, true);
}

void qecmd::DragNonLocalTime::redo()
{
	if (first) {
		first = false;
	}
	else {
		std::ranges::swap_ranges(train->timetable(), data->timetable());
	}
	cont->onTrainStationTimeChanged(train, true);
}

bool qecmd::DragNonLocalTime::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())return false;
	auto cmd = static_cast<const DragNonLocalTime*>(other);
	// 2024.03.26  new requirement: same modifiers
	if (train == cmd->train && station_id == cmd->station_id && mod == cmd->mod) {
		// simply discard interval status
		return true;
	}
	else return false;
}

qecmd::AssignPathsToTrain::AssignPathsToTrain(std::shared_ptr<Train> train, std::vector<TrainPath*>&& paths,
	TrainContext* cont, QUndoCommand* parent):
	QUndoCommand(QObject::tr("添加%1列车径路至%2").arg(paths.size()).arg(train->trainName().full())), 
	train(train), paths(std::move(paths)), cont(cont)
{
}

void qecmd::AssignPathsToTrain::undo()
{
	for (auto* p : paths) {
		p->removeTrainFromBack(train);
	}
	cont->afterChangeTrainPaths(train, paths);
}

void qecmd::AssignPathsToTrain::redo()
{
	for (auto* p : paths) {
		p->addTrain(train);
	}
	cont->afterChangeTrainPaths(train, paths);
}

qecmd::RemovePathsFromTrain::RemovePathsFromTrain(std::shared_ptr<Train> train, std::vector<PathInfoInTrain>&& data, 
	TrainContext* cont, QUndoCommand* parent):
	QUndoCommand(QObject::tr("从列车%1移除%2径路").arg(train->trainName().full()).arg(data.size()), parent),
	train(train),data(std::move(data)),cont(cont)
{
}

void qecmd::RemovePathsFromTrain::undo()
{
	for (const auto& d : data) {
		d.path->insertTrainWithIndex(train, d.train_index_in_path, d.path_index_in_train);
	}
	cont->afterChangeTrainPaths(train, data);
}

void qecmd::RemovePathsFromTrain::redo()
{
	for (auto itr = data.rbegin(); itr != data.rend(); ++itr) {
		itr->path->removeTrainWithIndex(train, itr->train_index_in_path, itr->path_index_in_train);
	}
	cont->afterChangeTrainPaths(train, data);
}

qecmd::ClearPathsFromTrain::ClearPathsFromTrain(std::shared_ptr<Train> train_, TrainContext*cont, QUndoCommand* parent):
	QUndoCommand(QObject::tr("清空列车径路: %1").arg(train_->trainName().full()), parent),
	train(train_), indexes_in_path(), cont(cont)
{
	for (auto p : train->paths()) {
		int idx = p->getTrainIndex(train);
		if (idx >= 0) {
			indexes_in_path.emplace_back(idx);
		}
	}
}

void qecmd::ClearPathsFromTrain::undo()
{
	train->paths() = std::move(this->paths);
	for (int i = 0; i < train->paths().size(); i++) {
		auto* p = train->paths().at(i);
		int idx = indexes_in_path.at(i);

		p->trains().insert(p->trains().begin() + idx, train);
	}
	cont->afterChangeTrainPaths(train, train->paths());
}

void qecmd::ClearPathsFromTrain::redo()
{
	paths = std::move(train->paths());
	for (int i = 0; i < paths.size(); i++) {
		auto* p = paths.at(i);
		int idx = indexes_in_path.at(i);

		p->trains().erase(p->trains().begin() + idx);
	}
	cont->afterChangeTrainPaths(train, paths);
}


// non-API commands
namespace qecmd {
	/**
	  * 删除一组列车。在TrainListWidget中调用。
	  * 暂定持有TrainCollection的引用，undo/redo有权限执行添加删除操作。
	  * 注意indexes应当排好序，并且和trains一一对应！
	  * --------
	  * 2023.08.16  modify this class to RemoveTrainsSimple;
	  * The real (valid) interface class is RemoveTrains, which contains
	  * clearing TrainPaths as children.
	  */
	class RemoveTrainsSimple :
		public QUndoCommand
	{
		QList<std::shared_ptr<Train>> _trains;
		QList<int> _indexes;
		TrainCollection& coll;
		TrainListModel* const model;
	public:
		RemoveTrainsSimple(const QList<std::shared_ptr<Train>>& trains,
			const QList<int>& indexes, TrainCollection& coll_,
			TrainListModel* model_,
			QUndoCommand* parent = nullptr);

		virtual void undo()override;

		/**
		 * 注意push操作会执行这个函数！
		 * 因为TrainListWidget必须保证删除按钮是有效的（无论是否有Slot接受这个CMD）
		 * 所以第一次的redo不能在这里做。置标志位。
		 */
		virtual void redo()override;

		const auto& trains()const { return _trains; }
		auto& trains() { return _trains; }
	};

	/**
	 * 2023.08.16  moved from navitree to traincontext; also change name to ~Simple
	 */
	class RemoveSingleTrainSimple :public QUndoCommand {
		DiagramNaviModel* const navi;
		std::shared_ptr<Train> train;
		int index;
	public:
		RemoveSingleTrainSimple(DiagramNaviModel* navi_, std::shared_ptr<Train> train_, int index_,
			QUndoCommand* parent = nullptr);
		virtual void undo()override;
		virtual void redo()override;
	};
}

qecmd::RemoveTrainsSimple::RemoveTrainsSimple(const QList<std::shared_ptr<Train>>& trains,
	const QList<int>& indexes, TrainCollection& coll_, TrainListModel* model_,
	QUndoCommand* parent) :
	QUndoCommand(QObject::tr("删除") + QString::number(trains.size()) + QObject::tr("个车次"), parent),
	_trains(trains), _indexes(indexes), coll(coll_), model(model_)
{
}

void qecmd::RemoveTrainsSimple::undo()
{
	model->undoRemoveTrains(_trains, _indexes);
}

void qecmd::RemoveTrainsSimple::redo()
{
	model->redoRemoveTrains(_trains, _indexes);
}

qecmd::RemoveTrains::RemoveTrains(const QList<std::shared_ptr<Train>>& trains,
	const QList<int>& indexes, TrainCollection& coll_, TrainListModel* model_,
	TrainContext* cont, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("删除%1个车次").arg(trains.size()), parent)
{
	foreach(auto train, trains) {
		if (!train->paths().empty()) {
			new ClearPathsFromTrain(train, cont, this);
		}
	}
	new RemoveTrainsSimple(trains, indexes, coll_, model_, this);
}


qecmd::RemoveSingleTrainSimple::RemoveSingleTrainSimple(DiagramNaviModel* navi_,
	std::shared_ptr<Train> train_, int index_, QUndoCommand* parent) :
	QUndoCommand(parent),
	navi(navi_), train(train_), index(index_) {}

void qecmd::RemoveSingleTrainSimple::undo()
{
	navi->undoRemoveSingleTrain(index, train);
}

void qecmd::RemoveSingleTrainSimple::redo()
{
	navi->commitRemoveSingleTrain(index);
}

qecmd::RemoveSingleTrain::RemoveSingleTrain(TrainContext* cont, DiagramNaviModel* navi_, 
	std::shared_ptr<Train> train_, int index_, QUndoCommand* parent):
	QUndoCommand(QObject::tr("删除列车: ") + train_->trainName().full(), parent)
{
	if (!train_->paths().empty()) {
		new ClearPathsFromTrain(train_, cont, this);
	}
	new RemoveSingleTrainSimple(navi_, train_, index_, this);
}

qecmd::RebindTrainsByPaths::RebindTrainsByPaths(std::vector<std::shared_ptr<Train>>&& trains_, 
	TrainContext* cont, QUndoCommand* parent):
	QUndoCommand(QObject::tr("重新铺画%1列车").arg(trains_.size()), parent),
	trains(std::move(trains_)), cont(cont)
{
	qDebug() << "RebindTrainsByPaths: " << trains.size() << " trains affected";
}

void qecmd::RebindTrainsByPaths::undo()
{
	for (size_t i = 0; i < trains.size(); i++) {
		std::swap(trains.at(i)->adapters(), adapters.at(i));
	}
	// context call
	cont->afterTrainsReboundByPath(trains, adapters);
}

void qecmd::RebindTrainsByPaths::redo()
{
	if (first) {
		assert(adapters.empty());
		adapters.reserve(trains.size());

		for (auto train : trains) {
			assert(!train->paths().empty());
			adapters.emplace_back(std::move(train->adapters()));
			train->bindWithPath();
		}

		first = false;
	}
	else {
		for (size_t i = 0; i < trains.size(); i++) {
			std::swap(trains.at(i)->adapters(), adapters.at(i));
		}
	}
	// call
	cont->afterTrainsReboundByPath(trains, adapters);
}



#endif
