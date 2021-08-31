#include "traincontext.h"

#include "viewers/traineventdialog.h"
#include "mainwindow.h"
#include "viewcategory.h"
#include "data/train/train.h"
#include "viewers/trainlinedialog.h"
#include "viewers/rulerrefdialog.h"
#include "dialogs/exchangeintervaldialog.h"
#include "dialogs/modifytimetabledialog.h"
#include "viewers/diagnosisdialog.h"

#include <QtWidgets>

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
		auto* page = cont->addCategoryPage(tr("列车审阅"));

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
		mw->diaActions.eventList = act;

		act = new QAction(QIcon(":/icons/line-manage.png"), tr("运行线一览"), this);
		act->setToolTip(tr("运行线一览表\n显示本次列车所有运行线基础信息及其铺画情况"));
		connect(act, SIGNAL(triggered()), this, SLOT(actShowTrainLineDialog()));
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(80);

		act = new QAction(QIcon(":/icons/ruler.png"), tr("标尺对照"), this);
		act->setToolTip(tr("标尺对照\n将本次列车运行情况与指定线路、标尺进行对照，"
			"以确定本次列车是否符合指定标尺，或与指定标尺差异如何。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actRulerRef()));
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(80);
		mw->diaActions.rulerRef = act;

		act = new QAction(QIcon(":/icons/identify.png"), tr("时刻诊断"), this);
		act->setToolTip(tr("列车时刻表诊断\n检查本次列车时刻表可能存在的问题，"
			"例如到开时刻填反等。"));
		connect(act, &QAction::triggered, this, &TrainContext::actDiagnose);
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(80);

		panel = page->addPannel(tr("交路"));
		edRouting = new SARibbonLineEdit;
		edRouting->setAlignment(Qt::AlignCenter);
		edRouting->setFocusPolicy(Qt::NoFocus);
		edRouting->setFixedWidth(120);

		auto* w = new QWidget;
		auto* vlay = new QVBoxLayout(w);
		vlay->addWidget(edRouting);
		act = new QAction(tr("转到交路"), this);
		connect(act, &QAction::triggered, this, &TrainContext::actToRouting);
		btnToRouting = new SARibbonToolButton(act);
		vlay->addWidget(btnToRouting);
		panel->addWidget(w, SARibbonPannelItem::Large);
	}

	//编辑
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("列车编辑"));
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
		w->setFixedWidth(240);
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
		w->setFixedWidth(240);
		panel->addWidget(w, SARibbonPannelItem::Large);

		panel->addSeparator();

		auto* act = new QAction(tr("自动运行线"), this);
		act->setCheckable(true);
		auto* btn = panel->addMediumAction(act);
		btnAutoUI = btn;
		btnAutoUI->setFixedWidth(80);
		act->setToolTip(tr("自动配置运行线\n如果启用，则采用列车所属类型的运行线颜色、线形、宽度，"
			"否则可针对本车次单独设置。"));
		connect(act, SIGNAL(triggered(bool)), this, SLOT(onAutoUIChanged(bool)));

		act = new QAction(tr("颜色"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(actSelectColor()));
		btnColor = panel->addMediumAction(act);
		btnColor->setFixedWidth(80);

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
		w->setFixedWidth(150);
		panel->addWidget(w, SARibbonPannelItem::Large);

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton), 
			tr("应用"), this);
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actApply()));

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton),
			tr("还原"), this);
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(refreshData()));

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

		panel = page->addPannel(tr(""));


		act = new QAction(QIcon(":/icons/timetable.png"), tr("时刻表"), this);
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(80);
		connect(act, SIGNAL(triggered()), this, SLOT(actShowBasicWidget()));

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
			tr("删除"), this);
		connect(act, SIGNAL(triggered()), this, SLOT(actRemoveCurrentTrain()));
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(70);

		act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
			tr("关闭面板"), this);
		act->setToolTip(tr("关闭面板\n关闭当前的列车上下文工具栏页面"));
		connect(act, &QAction::triggered, mw, &MainWindow::focusOutTrain);
		btn = panel->addLargeAction(act);
		btn->setMinimumWidth(80);
		
	}

}

void TrainContext::setupTypeCombo()
{
	comboType->clear();
	const auto& types = diagram.trainCollection().typeCount();
	for (auto p = types.begin(); p != types.end(); ++p) {
		if (p.value() >= 0) {
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
}

void TrainContext::updateTrainWidgetTitles(std::shared_ptr<Train> t)
{
	for (int i = 0; i < basicWidgets.size(); i++) {
		auto p = basicWidgets.at(i);
		if (p->train() == t) {
			basicDocks.at(i)->setWindowTitle(tr("时刻表编辑 - %1").arg(t->trainName().full()));
		}
	}
}

void TrainContext::refreshAllData()
{
	refreshData();
	for (auto p : basicWidgets) {
		p->refreshData();
	}
}

void TrainContext::removeTrainWidget(std::shared_ptr<Train> train)
{
	removeBasicDockAt(getBasicWidgetIndex(train));
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
		}
		else {
			edRouting->setText(tr("(无交路)"));
			btnToRouting->setEnabled(false);
		}
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

void TrainContext::actRemoveCurrentTrain()
{
	if (train) {
		int idx = diagram.trainCollection().getTrainIndex(train);
		if (idx != -1) {
			emit actRemoveTrain(idx);
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
	d->show();
}

void TrainContext::actToRouting()
{
	if (train && train->hasRouting()) {
		emit focusInRouting(train->routing().lock());
	}
}

void TrainContext::setTrain(std::shared_ptr<Train> train_)
{
	train = train_;
	refreshData();
}

void TrainContext::showTrainEvents()
{
	if (!train) {
		QMessageBox::warning(mw, tr("错误"), tr("列车事件表：没有选中车次！"));
		return;
	}
	auto* dialog = new TrainEventDialog(diagram, train, mw);
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
	data.commit();
	for (auto& p : data.startings) {
		mw->repaintTrainLines(p.first);
	}
	for (auto& p : data.terminals) {
		mw->repaintTrainLines(p.first);
	}
}
