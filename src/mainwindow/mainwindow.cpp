#include "mainwindow.h"
#include "data/rail/railway.h"
#include "data/rail/ruler.h"

#ifndef QETRC_MOBILE_2
#include <QMessageBox>
#include <QInputDialog>
#include <QString>
#include <QUndoView>
#include <QApplication>
#include <QFileDialog>
#include <QSpinBox>
#include <QStatusBar>
#include <QLabel>
#include <QStyleFactory>
#include <chrono>
#include <SARibbonActionsManager.h>
#include <SARibbonCustomizeDialog.h>
#include <QXmlStreamWriter>
#include <QMimeData>
#include <QTextBrowser>

#include "model/train/trainlistmodel.h"
#include "editors/trainlistwidget.h"
#include "model/diagram/diagramnavimodel.h"
#include "data/common/qesystem.h"
#include "data/diagram/diagrampage.h"
#include "navi/navitree.h"
#include "navi/addpagedialog.h"
#include "editors/configdialog.h"
#include "wizards/rulerpaint/rulerpaintwizard.h"
#include "util/railrulercombo.h"
#include "dialogs/outputsubdiagramdialog.h"
#include "dialogs/batchcopytraindialog.h"
#include "viewers/traindiffdialog.h"
#include "wizards/readruler/readrulerwizard.h"
#include "editors/routing/batchparseroutingdialog.h"
#include "editors/routing/detectroutingdialog.h"
#include "viewers/diagnosisdialog.h"
#include "viewers/timetablequickwidget.h"
#include "viewers/traininfowidget.h"
#include "model/train/timetablequickmodel.h"

#include "traincontext.h"
#include "railcontext.h"
#include "viewcategory.h"
#include "pagecontext.h"
#include "rulercontext.h"
#include "routingcontext.h"
#include "pathcontext.h"

#include "version.h"

#include "SARibbonBar.h"
#include "SARibbonCategory.h"
#include "SARibbonMenu.h"
#include "SARibbonPannel.h"
#include "SARibbonQuickAccessBar.h"
#include <SARibbonToolButton.h>

#include "DockAreaWidget.h"
#include <DockManager.h>
#include <AutoHideDockContainer.h>

#include "editors/railstationwidget.h"
#include "editors/routing/routingwidget.h"

#include <dialogs/changestationnamedialog.h>
#include "data/train/routing.h"

#include <model/rail/railstationmodel.h>
#include "dialogs/locatedialog.h"
#include "wizards/timeinterp/timeinterpwizard.h"
#include "railnet/raildb/raildbnavi.h"
#include "railnet/raildb/raildbwindow.h"
#include "railnet/raildb/raildbcontext.h"
#include "viewers/compare/diagramcomparedialog.h"
#include "wizards/greedypaint/greedypaintwizard.h"
#include "viewers/stats/intervaltraindialog.h"
#include "viewers/stats/intervalcountdialog.h"
#include "editors/train/predeftrainfiltermanager.h"
#include "viewers/stats/trainintervalstatdialog.h"
#include "log/GlobalLogger.h"
#include "log/IssueWidget.h"
#include "editors/trainpath/pathlistwidget.h"
#include "model/trainpath/pathlistmodel.h"
#include "log/IssueManager.h"
#include "defines/icon_specs.h"
#include "editors/ribbonconfigdialog.h"


MainWindow::MainWindow(QWidget* parent)
	: SARibbonMainWindow(parent, true),
	undoStack(new QUndoStack(this)),
	naviModel(new DiagramNaviModel(_diagram, this))
{
	qApp->setStyle(QStyleFactory::create(SystemJson::instance.app_style));
	auto start = std::chrono::system_clock::now();
	ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
	ads::CDockManager::setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);
	ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
	ads::CDockManager::setAutoHideConfigFlags(ads::CDockManager::DefaultAutoHideConfig);
	manager = new ads::CDockManager(this);
	_diagram.readDefaultConfigs();
	undoStack->setUndoLimit(200);
	connect(undoStack, SIGNAL(indexChanged(int)), this, SLOT(markChanged()));

	initUI();

	QString cmdFile;
	if (QCoreApplication::arguments().size() > 1) {
		cmdFile = QCoreApplication::arguments().at(1);
	}

	loadInitDiagram(cmdFile);
	updateWindowTitle();
	setAcceptDrops(true);

	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	auto secs = static_cast<double>(duration.count()) * std::chrono::microseconds::period::num /
		std::chrono::microseconds::period::den;
	qInfo() << "MainWindow init consumes: " << secs << Qt::endl;
	showStatus(tr("主窗口初始化用时: %1 秒").arg(secs));
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
	initDockWidgets();
	initToolbar();
	//sa_apply_customize_from_xml_file("customize.xml", this, actMgr);
	sa_apply_customize_from_xml_file("customize.xml", ribbonBar(), actMgr);   // seems changed in SARibbon v1.0.7
}

void MainWindow::undoRemoveTrains(const QList<std::shared_ptr<Train>>& trains)
{
	//重新添加运行线
	for (auto p : trains) {
		addTrainLine(*p);
	}
	//现在TrainListModel的信号直接发给NaviModel了
	//informTrainListChanged();
}

void MainWindow::redoRemoveTrains(const QList<std::shared_ptr<Train>>& trains)
{
	for (auto t : trains)
		removeTrain(t);
	//informTrainListChanged();
}

void MainWindow::removeTrain(std::shared_ptr<Train> train)
{
	for (auto p : diagramWidgets) {
		p->removeTrain(*train);
	}
	contextTrain->removeTrainWidget(train);
	if (train == contextTrain->getTrain()) {
		contextTrain->resetTrain();
		focusOutTrain();
	}
}

void MainWindow::undoRemoveTrain(std::shared_ptr<Train> train)
{
	addTrainLine(*train);
}

void MainWindow::onStationTableChanged(std::shared_ptr<Railway> rail, [[maybe_unused]] bool equiv)
{
	_diagram.updateRailway(rail);
	trainListWidget->getModel()->updateAllMileSpeed();
	pathListWidget->model()->updateAllValidity();
	updateRailwayDiagrams(rail);
	//2022.06.02：如果非equiv变化，贪心推线的要更新！
	//2022.11.19：删除!equiv条件；单双线变化现在似乎不被认为是non-equiv，但它确实影响推线。
	if (greedyWidget) {
		greedyWidget->refreshData(rail);
	}
}

void MainWindow::updateRailwayDiagrams(std::shared_ptr<Railway> rail)
{
	for (int i = 0; i < _diagram.pages().size(); i++) {
		if (_diagram.pages().at(i)->containsRailway(rail)) {
			diagramWidgets.at(i)->paintGraph();
		}
	}
}

void MainWindow::updateRailwayDiagrams(Railway& rail)
{
	for (int i = 0; i < _diagram.pages().size(); i++) {
		if (_diagram.pages().at(i)->containsRailway(rail)) {
			diagramWidgets.at(i)->paintGraph();
		}
	}
}

void MainWindow::updateAllDiagrams()
{
	for (auto p : diagramWidgets)
		p->paintGraph();
}

void MainWindow::updatePageDiagram(std::shared_ptr<DiagramPage> pg)
{
	foreach(auto w, diagramWidgets) {
		if (w->page() == pg) {
			w->paintGraph();
			break;
		}
	}
}

void MainWindow::onTrainsImported()
{
	updateAllDiagrams();
	//informTrainListChanged();
	//手动更新TrainList
	trainListWidget->refreshData();   //这个自动调用naviTree的更新操作！
	naviModel->refreshRoutings();
	undoStack->clear();  //不支持撤销
	markChanged();
	showStatus(tr("导入列车完成"));
}

void MainWindow::removeAllTrains()
{
	auto flag = QMessageBox::question(this, tr("警告"),
		tr("此操作删除所有车次，且不可撤销，是否继续？\n"
			"此操作通常用于导入新的车次之前。"));
	if (flag != QMessageBox::Yes)
		return;

	_diagram.trainCollection().clearTrains();
	_diagram.pathCollection().clearTrains();    // 2024.03.20
	onTrainsImported();   //后操作是一样的
	contextTrain->resetTrain();
	timetableQuickWidget->resetTrain();
	focusOutTrain();
}

void MainWindow::removeAllTrainsAndRoutings()
{
	auto flag = QMessageBox::question(this, tr("警告"),
		tr("此操作删除所有车次和所有交路，且不可撤销，是否继续？\n"
			"此操作通常用于导入新的车次之前。"));
	if (flag != QMessageBox::Yes)
		return;

	_diagram.trainCollection().clearTrainsAndRoutings();
	_diagram.pathCollection().clearTrains();  // 2024.03.20
	onTrainsImported();   //后操作是一样的
	contextTrain->resetTrain();
	timetableQuickWidget->resetTrain();
	focusOutTrain();
}

void MainWindow::undoAddNewRailway(std::shared_ptr<Railway> rail)
{
	removeRailStationWidget(rail);
}

void MainWindow::onNewTrainAdded(std::shared_ptr<Train> train)
{
	// 2021.10.17：新车次添加改为显示EditWidget
	if (train->empty())
		contextTrain->showEditWidget(train);
	addTrainLine(*train);
}

void MainWindow::undoAddNewTrain(std::shared_ptr<Train> train)
{
	contextTrain->removeTrainWidget(train);
	removeTrainLine(*train);
	if (train == contextTrain->getTrain())
		focusOutTrain();
}

#if 0
[[deprecated]]
void MainWindow::onActRailwayRemoved(std::shared_ptr<Railway> rail)
{
	removeRailStationWidget(rail);
	contextRail->removeRulerWidgetsForRailway(rail);
	naviModel->resetPageList();
	checkPagesValidity();
	updateAllDiagrams();
	trainListWidget->getModel()->updateAllMileSpeed();
	undoStack->clear();
}
#endif

void MainWindow::onActRailwayRemovedU(std::shared_ptr<Railway> rail)
{
	removeRailStationWidget(rail);
	contextRail->removeRulerWidgetsForRailway(rail);
	trainListWidget->getModel()->updateAllMileSpeed();
	pathListWidget->model()->updateAllValidity();
}

void MainWindow::removeRailStationWidget(std::shared_ptr<Railway> rail)
{
	for (int i = railStationWidgets.size() - 1; i >= 0; i--) {
		auto p = railStationWidgets.at(i);
		if (p->getRailway() == rail) {
			removeRailStationWidgetAt(i);
		}
	}
	if (rail == contextRail->getRailway()) {
		contextRail->resetRailway();
		focusOutRailway();
	}
}

void MainWindow::removeRailStationWidgetAt(int i)
{
	auto* dock = railStationDocks.takeAt(i);
	railStationWidgets.takeAt(i);
	dock->deleteDockWidget();
}

void MainWindow::actOpenNewRailWidget()
{
	QStringList lst;
	for (auto p : _diagram.railways()) {
		lst << p->name();
	}
	bool ok;
	auto res = QInputDialog::getItem(this, tr("基线编辑"), tr("请选择需要编辑的基线名称。"
		"系统将创建新的基线编辑窗口。"), lst, 0, false, &ok);
	if (ok && !res.isEmpty()) {\
		// 2023.10.15: the rail may be empty even res is not empty (input by user).
		// e.g. in an emtpy diagram, the rail is always nullptr...   see Github issue #2
		auto rail = _diagram.railwayByName(res);
		if (rail)
			actOpenRailStationWidget(rail);
	}
}

void MainWindow::checkPagesValidity()
{
	for (int i = diagramWidgets.size() - 1; i >= 0; i--) {
		auto w = diagramWidgets.at(i);
		auto pgInDia = w->page();
		if (!_diagram.pages().contains(pgInDia)) {
			//REMOVE THIS!!
			diagramWidgets.removeAt(i);
			auto* dock = diagramDocks.takeAt(i);
			dock->deleteDockWidget();
		}
	}
}

void MainWindow::actChangePassedStations()
{
	int n = spPassedStations->value();
	int old = _diagram.config().max_passed_stations;
	if (n != old) {
		undoStack->push(new qecmd::ChangePassedStation(old, n, this));
	}
}

void MainWindow::showHelpDialog()
{
	QString text = tr("请参见qETRC在线文档：<a href=\"%1\">%1</a>").arg(qespec::DOC_URL.data());
	QMessageBox::about(this, tr("帮助"), text);
}

void MainWindow::showAboutDialog()
{
	QString text = tr("%1_%2 Release R%3\n%4\n")
		.arg(qespec::TITLE.data())
		.arg(qespec::VERSION.data())
		.arg(qespec::RELEASE_CODE)
		.arg(qespec::DATE.data());
	text += tr("主要开发者：萧迩珀  \nmxy0268@qq.com\n");
	text += tr("英文版翻译：WenSimEHRP  萧迩珀\n");
	text += tr("许可证：GPLv3\n");
	text += tr("QQ群：865211882\nhttps://github.com/CDK6182CHR/qETRC\n"
		"https://gitee.com/xep0268/qETRC");
	QMessageBox::about(this, tr("关于"), text);
}

#if 0
void MainWindow::useWpsStyle()
{
	ribbonBar()->setRibbonStyle(SARibbonBar::WpsLiteStyle);
	SystemJson::instance.ribbon_style = SARibbonBar::WpsLiteStyle;
}

void MainWindow::useOfficeStyle()
{
	ribbonBar()->setRibbonStyle(SARibbonBar::OfficeStyle);
	SystemJson::instance.ribbon_style = SARibbonBar::OfficeStyle;
}
#endif

void MainWindow::actCustomizeRibbon()
{
	SARibbonCustomizeDialog dlg(this);
	dlg.setupActionsManager(actMgr);
	dlg.fromXml("customize.xml");
	if (SARibbonCustomizeDialog::Accepted == dlg.exec()) {
		dlg.applys();
		QByteArray str;
		QXmlStreamWriter xml(&str);
		xml.setAutoFormatting(true);
		xml.setAutoFormattingIndent(2);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)  // QXmlStreamWriter always encodes XML in UTF-8.
		xml.setCodec("utf-8");
#endif
		xml.writeStartDocument();
		bool isok = dlg.toXml(&xml);
		xml.writeEndDocument();
		if (isok) {
			QFile f("customize.xml");
			if (f.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
				QTextStream s(&f);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)  // QTextStream always encodes XML in UTF-8.
				s.setCodec("utf-8");
#endif
				s << str;
				s.flush();
			}
		}
	}
}

void MainWindow::actRulerPaint()
{
	auto* wzd = new RulerPaintWizard(_diagram, this);
	connect(wzd, &RulerPaintWizard::removeTmpTrainLine,
		this, &MainWindow::removeTrainLine);
	connect(wzd, &RulerPaintWizard::paintTmpTrainLine,
		this, &MainWindow::addTrainLineTmp);
	connect(wzd, &RulerPaintWizard::trainAdded,
		naviView, &NaviTree::actAddPaintedTrain);
	connect(wzd, &RulerPaintWizard::trainTimetableModified,
		contextTrain, &TrainContext::onTrainTimetableChanged);
	connect(wzd, &RulerPaintWizard::trainInfoModified,
		contextTrain, &TrainContext::onTrainInfoChanged);
	connect(this, &MainWindow::paintingPointClicked,
		wzd, &RulerPaintWizard::onPaintingPointClicked);
	wzd->show();
}

#ifdef QETRC_GREEDYPAINT_TEST
#include <wizards/greedypaint/greedypaintfasttest.h>

void MainWindow::actGreedyPaintFast()
{
	auto* w = new GreedyPaintFastTest(_diagram, this);
	connect(w, &GreedyPaintFastTest::trainAdded,
		naviView, &NaviTree::actAddPaintedTrain);
	connect(w, &GreedyPaintFastTest::showStatus,
		this, &MainWindow::showStatus);
	w->show();
}
#endif

void MainWindow::actGreedyPaint()
{
	if (greedyWidget == nullptr) {
		greedyWidget = new GreedyPaintWizard(_diagram, this);
		greedyWidget->setWindowFlag(Qt::Dialog);
		greedyWidget->resize(600,600);
		connect(greedyWidget, &GreedyPaintWizard::removeTmpTrainLine,
			this, &MainWindow::removeTrainLine);
		connect(greedyWidget, &GreedyPaintWizard::paintTmpTrainLine,
			this, &MainWindow::addTrainLineTmp);
		connect(greedyWidget, &GreedyPaintWizard::trainAdded,
			naviView, &NaviTree::actAddPaintedTrain);
		connect(greedyWidget, &GreedyPaintWizard::showStatus,
			this, &MainWindow::showStatus);
		connect(this, &MainWindow::paintingPointClicked,
			greedyWidget, &GreedyPaintWizard::onPaintingPointClicked);
	}
	greedyWidget->show();
}

void MainWindow::actExitGreedyPaint()
{
	if (!greedyWidget) {
		QMessageBox::warning(this, tr("错误"), tr("当前没有进入贪心推线模式，无需退出。"));
	}
	else {
		auto t = QMessageBox::question(this, tr("提示"), tr("退出贪心推线模式，关闭对话框并释放资源。\n"
			"已经配置的排图约束数据将丢失，是否确认？"), QMessageBox::Yes | QMessageBox::No);
		if (t == QMessageBox::Yes) {
			greedyWidget->cleanUpTempData();   // 2023.10.04  add
			greedyWidget->close();
			greedyWidget->deleteLater();
			greedyWidget = nullptr;
		}
	}
}


//void MainWindow::undoAddPage(std::shared_ptr<DiagramPage> page)
//{
//    auto dock = diagramDocks.takeLast();
//    manager->removeDockWidget(dock);
//    delete dock;
//    diagramWidgets.removeLast();
//    informPageListChanged();
//}


void MainWindow::initDockWidgets()
{
	ads::CDockWidget* dock;

	//manager->setConfigFlag(ads::CDockManager::FocusHighlighting, true);

	// Central
	if (SystemJson::instance.use_central_widget) {
		dock = new ads::CDockWidget(tr("运行图窗口"));
		dock->setFeature(ads::CDockWidget::NoTab, true);
		auto* w = new QWidget;
		dock->setWidget(w);
		centralOccupyWidget = w;

		auto* c = manager->setCentralWidget(dock);
		centralArea = c;
		//c->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	//总导航
	if constexpr (true) {
		auto* tree = new NaviTree(naviModel, undoStack);
		naviView = tree;
		dock = new ads::CDockWidget(QObject::tr("运行图资源管理器"));
		naviDock = dock;
		dock->setWidget(tree);

		//context的显示条件：第一次触发时显示；后续一直在，该车次（线路，运行图）被删除则隐藏
		//各种focusout的slot保留，在需要隐藏的时候调用
		//connect(tree, &NaviTree::pageAdded, this, &MainWindow::actAddPage);

		connect(tree, &NaviTree::pageInserted, this, &MainWindow::insertPageWidget);
		connect(tree, &NaviTree::pageRemoved, this, &MainWindow::removePageAt);

		connect(tree, &NaviTree::focusInPage, this, &MainWindow::focusInPage);
		//connect(tree, &NaviTree::focusOutPage, this, &MainWindow::focusOutPage);
		connect(tree, &NaviTree::focusInTrain, this, &MainWindow::focusInTrain);
		//connect(tree, &NaviTree::focusOutTrain, this, &MainWindow::focusOutTrain);
		connect(tree, &NaviTree::focusInRailway, this, &MainWindow::focusInRailway);
		//connect(tree, &NaviTree::focusOutRailway, this, &MainWindow::focusOutRailway);
		connect(tree, &NaviTree::editRailway, this, &MainWindow::actOpenRailStationWidget);
		connect(tree, &NaviTree::trainsImported, this, &MainWindow::onTrainsImported);
		connect(tree, &NaviTree::focusInRuler, this, &MainWindow::focusInRuler);

		connect(naviModel, &DiagramNaviModel::newRailwayAdded,
			this, &MainWindow::actOpenRailStationWidget);
		connect(naviModel, &DiagramNaviModel::undoneAddRailway,
			this, &MainWindow::undoAddNewRailway);
		//Add train的connect放到toolbar里面，因为依赖于contextTrain


		connect(naviModel, &DiagramNaviModel::trainRemoved,
			this, &MainWindow::removeTrain);
		connect(naviModel, &DiagramNaviModel::undoneTrainRemove,
			this, &MainWindow::undoRemoveTrain);

		connect(naviModel, &DiagramNaviModel::railwayRemovedU,
			this, &MainWindow::onActRailwayRemovedU);

		connect(naviView, &NaviTree::focusInRouting,
			this, &MainWindow::focusInRouting);

		connect(naviView, &NaviTree::actBatchDetectRouting,
			this, &MainWindow::actBatchDetectRouting);
		connect(naviView, &NaviTree::actBatchParseRouting,
			this, &MainWindow::actBatchParseRouting);
		connect(naviView, &NaviTree::activatePageAt,
			this, &MainWindow::activatePageWidget);

		connect(naviView, &NaviTree::focusInPath,
			this, &MainWindow::focusInPath);
	}
	//auto* area = manager->addDockWidget(ads::LeftDockWidgetArea, dock);
	auto autoHideArea = manager->addAutoHideDockWidget(ads::SideBarLeft, dock);
	autoHideArea->setSize(240);

	// 历史记录
	if constexpr (true) {
		QUndoView* view = new QUndoView(undoStack);
		undoView = view;
		view->setCleanIcon(QEICN_undo_stack_clean);
		auto* dock = new ads::CDockWidget(tr("历史记录"));
		dock->setWidget(view);
		undoDock = dock;
		//area = manager->addDockWidget(ads::BottomDockWidgetArea, dock, area);
		autoHideArea = manager->addAutoHideDockWidget(ads::SideBarLeft, dock);
		autoHideArea->setSize(240);
}

#ifdef Q_OS_ANDROID
	naviDock->closeDockWidget();
	undoDock->closeDockWidget();
#endif

	//列车管理
	if constexpr (true) {
		auto* tw = new TrainListWidget(_diagram.trainCollection(), undoStack);
		dock = new ads::CDockWidget(tr("列车管理"));
		trainListWidget = tw;
		trainListDock = dock;
		dock->setWidget(tw);
		manager->addDockWidget(ads::LeftDockWidgetArea, dock);
		dock->closeDockWidget();
		connect(tw->getModel(), &TrainListModel::trainsRemovedUndone,
			this, &MainWindow::undoRemoveTrains);
		connect(tw->getModel(), &TrainListModel::trainsRemovedRedone,
			this, &MainWindow::redoRemoveTrains);
		connect(tw->getModel(), &QAbstractItemModel::modelReset,
			naviModel, &DiagramNaviModel::resetTrainList);
		connect(tw, &TrainListWidget::currentTrainChanged, this, &MainWindow::focusInTrain);
		connect(tw->getModel(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
			naviModel, SLOT(onTrainDataChanged(const QModelIndex&, const QModelIndex&)));
		connect(tw, &TrainListWidget::addNewTrain, naviView, &NaviTree::actAddTrain);

		connect(naviModel, &DiagramNaviModel::trainRowsAboutToBeInserted,
			tw->getModel(), &TrainListModel::onBeginInsertRows);
		connect(naviModel, &DiagramNaviModel::trainRowsInserted,
			tw->getModel(), &TrainListModel::onEndInsertRows);
		connect(naviModel, &DiagramNaviModel::trainRowsAboutToBeRemoved,
			tw->getModel(), &TrainListModel::onBeginRemoveRows);
		connect(naviModel, &DiagramNaviModel::trainRowsRemoved,
			tw->getModel(), &TrainListModel::onEndRemoveRows);
		connect(naviModel, &DiagramNaviModel::nonEmptyRailwayAdded,
			tw->getModel(), &TrainListModel::updateAllMileSpeed);
	}

	// 交路管理
	if constexpr (true) {
		auto* rw = new RoutingWidget(_diagram.trainCollection(), undoStack);
		dock = new ads::CDockWidget(tr("交路管理"));
		dock->setWidget(rw);
		manager->addDockWidget(ads::LeftDockWidgetArea, dock);
		dock->closeDockWidget();
		routingWidget = rw;
		routingDock = dock;

		connect(rw, &RoutingWidget::focusInRouting,
			this, &MainWindow::focusInRouting);
		connect(rw->getModel(), &RoutingCollectionModel::dataChanged,
			naviModel, &DiagramNaviModel::onRoutingChanged);
		connect(rw->getModel(), &RoutingCollectionModel::rowsInserted,
			naviModel, &DiagramNaviModel::onRoutingInserted);
		connect(rw->getModel(), &RoutingCollectionModel::rowsRemoved,
			naviModel, &DiagramNaviModel::onRoutingRemoved);
		connect(naviView, &NaviTree::removeRoutingNavi,
			rw, &RoutingWidget::onRemoveRoutingFromNavi);
		connect(naviView, &NaviTree::actAddRouting,
			rw, &RoutingWidget::actAdd);
	}

	// TrainPath 列车径路
	if constexpr (true) {
		auto* w = new PathListWidget(_diagram.railCategory(), _diagram.pathCollection(), undoStack);
		dock = new ads::CDockWidget(tr("列车径路管理"));
		dock->setWidget(w);
		manager->addDockWidget(ads::LeftDockWidgetArea, dock);
		dock->closeDockWidget();
		pathListWidget = w;
		pathListDock = dock;

		connect(pathListWidget->model(), &PathListModel::rowsInserted,
			naviModel, &DiagramNaviModel::onPathInserted);
		connect(pathListWidget->model(), &PathListModel::rowsRemoved,
			naviModel, &DiagramNaviModel::onPathRemoved);
		connect(pathListWidget->model(), &PathListModel::dataChanged,
			naviModel, &DiagramNaviModel::onPathChanged);
		connect(naviView, &NaviTree::actAddPath,
			pathListWidget, &PathListWidget::actAdd);
		connect(naviView, &NaviTree::duplicatePathNavi,
			pathListWidget, &PathListWidget::actDuplicate);
		connect(pathListWidget, &PathListWidget::focusInPath,
			this, &MainWindow::focusInPath);
		connect(naviModel, &DiagramNaviModel::nonEmptyRailwayAdded,
			pathListWidget->model(), &PathListModel::updateAllValidity);
	}


	// 速览信息
	if constexpr (true) {
		auto* w = new TrainInfoWidget();
		dock = new ads::CDockWidget(tr("速览信息"));
		dock->setWidget(w);
		trainInfoWidget = w;
		trainInfoDock = dock;
		connect(w, &TrainInfoWidget::showTimetable,
			this, &MainWindow::showQuickTimetable);
		connect(w, &TrainInfoWidget::switchToRouting,
			this, &MainWindow::focusInRouting);
		//dock->setMinimumSize(200, 200);
		dock->setBaseSize(200, 200);
	}
	auto* area = manager->addDockWidget(ads::RightDockWidgetArea, dock);
	//auto* container=manager->addAutoHideDockWidget(ads::SideBarRight, dock);
	//container->setSize(240);
	//area->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);

	// 速览时刻
	if constexpr (true) {
		auto* w = new TimetableQuickWidget(undoStack);
		dock = new ads::CDockWidget(tr("速览时刻"));
		dock->setWidget(w);
		timetableQuickWidget = w;
		timetableQuickDock = dock;
		//dock->setMinimumSize(200, 200);
		dock->setBaseSize(200, 200);
		manager->addDockWidgetTabToArea(dock, area);
		//container = manager->addAutoHideDockWidget(ads::SideBarRight, dock);
		//container->setSize(240);
	}

	// 输出窗口 (日志窗口)
	if constexpr (true) {
		auto* w = new QTextBrowser;
		dock = new ads::CDockWidget(tr("输出"));
		dock->setWidget(w);
		logDock = dock;
		logWidget = w;

		autoHideArea = manager->addAutoHideDockWidget(ads::SideBarBottom, dock);
		autoHideArea->setSize(150);
		GlobalLogger::get()->setOutWidget(w);
		
		qInstallMessageHandler(qeutil::qeLogHandler);
	}

	// 问题窗口
	if constexpr (true) {
		auto* w = new IssueWidget;
		dock = new ads::CDockWidget(tr("问题"));
		dock->setWidget(w);
		issueWidget = w;
		issueDock = dock;

		autoHideArea = manager->addAutoHideDockWidget(ads::SideBarBottom, dock);
		autoHideArea->setSize(200);

		connect(IssueManager::get(), &IssueManager::rowsInserted,
			[autoHideArea]() {if (!autoHideArea->isVisible()) autoHideArea->toggleCollapseState(); });
	}
}

void MainWindow::initToolbar()
{
	SARibbonBar* ribbon = ribbonBar();
	ribbon->applicationButton()->setText(QStringLiteral("文件"));
	ribbon->showMinimumModeButton();

	//顶上的工具条
	QAction* actGlobalConfig;
	if constexpr (true) {
		//撤销重做
		QAction* act = undoStack->createUndoAction(this, tr("撤销"));
		act->setObjectName(tr("撤销"));
		act->setIcon(QEICN_undo);
		act->setShortcut(Qt::CTRL | Qt::Key_Z);
		act->setShortcutContext(Qt::WindowShortcut);
		ribbon->quickAccessBar()->addAction(act);

		act = undoStack->createRedoAction(this, tr("重做"));
		act->setObjectName(tr("重做"));
		act->setIcon(QEICN_redo);
		act->setShortcut(Qt::CTRL | Qt::Key_Y);
		ribbon->quickAccessBar()->addAction(act);

		act = makeAction(QEICN_close_current, tr("关闭当前面面板 (Ctrl+W)"));
		act->setShortcut(Qt::CTRL | Qt::Key_W);
		connect(act, &QAction::triggered, this, &MainWindow::closeCurrentTab);
		ribbon->quickAccessBar()->addAction(act);

        auto* menu = new SARibbonMenu(this);
		act = makeAction(tr("全局配置选项菜单"));
		act->setObjectName(tr("全局选项菜单"));
		auto* actsub = menu->addAction(tr("全局配置选项"));
		actGlobalConfig = actsub;
		actsub = menu->addAction(tr("Ribbon选项"), this, &MainWindow::actRibbonConfig);
		act->setIcon(QEICN_global_config);
		act->setMenu(menu);
		ribbon->quickAccessBar()->addAction(act, Qt::ToolButtonIconOnly, QToolButton::InstantPopup);

		// Customize 似乎还不太对，先留在这
		// 2024.04.04: Enable it
#if 1
		act = makeAction(QEICN_customize, tr("自定义Ribbon"));
		act->setToolTip(tr("自定义Ribbon\n自定义工具栏按钮的排列组合"));
		connect(act, &QAction::triggered, this, &MainWindow::actCustomizeRibbon);
		ribbon->quickAccessBar()->addAction(act);
#endif

	}

    QAction* actTrainList;
	//开始
	if constexpr (true) {
		SARibbonCategory* cat = ribbon->addCategoryPage(QObject::tr("开始(&1)"));
		SARibbonPannel* panel = cat->addPannel(QObject::tr("文件"));

		QAction* act = makeAction(QEICN_new_file, QObject::tr("新建"));
		panel->addLargeAction(act);
		act->setShortcut(Qt::CTRL | Qt::Key_N);
		act->setToolTip(tr("新建 (Ctrl+N)\n关闭当前运行图文件，并新建空白运行图文件。"));
		addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actNewGraph()));
		sharedActions.newfile = act;

		act = makeAction(QEICN_open_file, QObject::tr("打开"));
		addAction(act);
		act->setToolTip(tr("打开 (Ctrl+O)\n关闭当前运行图文件，并打开新的既有运行图文件。"));
		act->setShortcut(Qt::CTRL | Qt::Key_O);
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actOpenGraph()));
		sharedActions.open = act;

		act = makeAction(QEICN_save_file, QObject::tr("保存"));
		act->setToolTip(tr("保存 (Ctrl+S)\n保存当前运行图。如果是新运行图，则需要先选择文件。"));
		act->setShortcut(Qt::CTRL | Qt::Key_S);
		addAction(act);
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actSaveGraph()));
		sharedActions.save = act;

		act = makeAction(QEICN_save_file_as, tr("另存为.."));
		act->setToolTip(tr("另存为\n将当前运行图另存为新建文件。"));
		panel->addMediumAction(act);
		connect(act, &QAction::triggered, this, &MainWindow::actSaveGraphAs);
		sharedActions.saveas = act;

		act = makeAction(QEICN_export_single_rail, tr("导出为.."));
		act->setToolTip(tr("导出为单线路运行图 (Ctrl+M)\n"
			"导出为单一线路的pyETRC或ETRC运行图文件格式。"));
		act->setShortcut(Qt::CTRL | Qt::Key_M);
		connect(act, SIGNAL(triggered()), this, SLOT(actToSingleFile()));
		addAction(act);
		panel->addMediumAction(act);

		panel = cat->addPannel(tr("窗口"));
		act = naviDock->toggleViewAction();
		act->setShortcut(Qt::Key_F3);
		addAction(act);
		act->setText(QObject::tr("导航"));
		act->setObjectName(tr("导航面板"));
		act->setToolTip(tr("导航面板 (F3)\n打开或关闭运行图总导航面板。"));
		act->setIcon(QEICN_navi);
        panel->addLargeAction(act);
		//btn->setMinimumWidth(80);

		act = trainListDock->toggleViewAction();
		act->setObjectName(tr("列车管理面板开关"));
		actTrainList = act;
		act->setText(tr("列车管理"));
		act->setToolTip(tr("列车管理\n打开或关闭（pyETRC风格的）列车管理列表面板。"));
		act->setIcon(QEICN_train_list);
        //panel->addLargeAction(act);   // 2024.04.04: not add to this page, to avoid dulplication
		//btn->setMinimumWidth(80);

		SARibbonMenu* menu = new SARibbonMenu(this);
		pageMenu = menu;
		menu->setToolTip(tr("运行图窗口\n导航、打开或关闭现有运行图窗口。"));
		menu->setTitle(QObject::tr("运行图窗口"));
		menu->setIcon(QEICN_diagram_widget);
        panel->addLargeMenu(menu);
		//btn->setMinimumWidth(80);

		//undo  试一下把dock也放在这里初始化...

		act = undoDock->toggleViewAction();
		act->setObjectName(tr("历史记录"));
		act->setText(tr("历史记录"));
		act->setIcon(QEICN_history);
		act->setToolTip(tr("历史记录\n打开或关闭历史记录面板。\n"
			"历史记录面板记录了可撤销的针对运行图文档的操作记录，可以查看、撤销、重做。"));
        panel->addLargeAction(act);
		//btn->setMinimumWidth(80);

		act = logDock->toggleViewAction();
		act->setIcon(QEICN_text_out_widget);
		act->setObjectName(tr("输出窗口开关"));
		act->setToolTip(tr("输出窗口\n打开或关闭文本输出窗口"));
		panel->addMediumAction(act);

		act = issueDock->toggleViewAction();
		act->setIcon(QEICN_issue_widget);
		act->setObjectName(tr("问题窗口开关"));
		act->setToolTip(tr("铺画问题窗口\n打开或关闭铺画问题窗口，显示运行线铺画过程中的错误报告"));
		panel->addMediumAction(act);

		act = makeAction(QEICN_add_diagram_page, tr("添加运行图"));
		act->setToolTip(tr("添加运行图\n选择既有基线数据，建立新的运行图页面。"));
		connect(act, SIGNAL(triggered()), naviView, SLOT(addNewPage()));
        panel->addLargeAction(act);
		//btn->setMinimumWidth(80);

		panel = cat->addPannel(tr("更新"));
		act = makeAction(QEICN_global_refresh, tr("刷新"));
		act->setToolTip(tr("刷新 (F5)\n重新铺画运行图，更新所有数据面板的信息。"));
		act->setShortcut(Qt::Key_F5);
		diaActions.refreshAll = act;
		addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(refreshAll()));

		menu = new SARibbonMenu(tr("刷新扩展"), this);
		menu->addAction(tr("重置状态"), this, &MainWindow::rebindAndRefresh);
		act->setMenu(menu);
        panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = makeAction(QEICN_rename_station, tr("更改站名"));
		act->setToolTip(tr("全局站名修改 (Ctrl+U)\n"
			"在整个运行图的所有线路、所有车次中，将旧站名改为新站名。"));
        //qactChangeStationName = act;
		act->setShortcut(Qt::CTRL | Qt::Key_U);
		addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actChangeStationName()));
        panel->addLargeAction(act);
		//btn->setMinimumWidth(80);

		panel = cat->addPannel(tr("系统"));
		act = makeAction(QEICN_help, tr("帮助"));
		connect(act, &QAction::triggered, this, &MainWindow::showHelpDialog);
		panel->addLargeAction(act);

		act = makeAction(QEICN_about, tr("关于"));
		connect(act, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
		menu = new SARibbonMenu(this);
		menu->addAction(QEICN_about_qt, tr("关于Qt"), QApplication::aboutQt);
		act->setMenu(menu);
        panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = makeAction(QEICN_exit_app, tr("退出"));
		connect(act, SIGNAL(triggered()), this, SLOT(close()));
		panel->addLargeAction(act);

	}

	//线路
	QAction* actQuickPath, * actSelector;
	if constexpr (true) {
		auto* cat = ribbon->addCategoryPage(tr("线路(&2)"));
		auto* panel = cat->addPannel(tr("基础数据"));

		auto* act = makeAction(QEICN_import_rails, tr("导入线路"));
		//connect(act, SIGNAL(triggered()), naviView, SLOT(importRailways()));
		connect(act, &QAction::triggered, naviView, &NaviTree::actImportRailways);
		act->setToolTip(tr("导入线路\n从既有运行图文件中导入（其中全部的）线路数据"));
		panel->addLargeAction(act);

		act = makeAction(QEICN_rail_edit, tr("基线编辑"));
		act->setToolTip(tr("基线编辑\n导航到、新建、打开或者关闭（pyETRC风格的）基线编辑面板。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actOpenNewRailWidget()));
		auto* menu = new SARibbonMenu(this);
		railMenu = menu;
		act->setMenu(menu);
		menu->setTitle(tr("线路编辑"));
		panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = pathListDock->toggleViewAction();
		act->setIcon(QEICN_train_path);
		act->setText(tr("列车径路"));
		act->setObjectName(tr("列车径路面板"));
		act->setToolTip(tr("列车径路\n打开/关闭列车径路管理面板。"));
		pathMenu = new SARibbonMenu(this);
		act->setMenu(pathMenu);
		panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = makeAction(QEICN_ruler_edit, tr("标尺编辑"));
		act->setToolTip(tr("标尺编辑 (Ctrl+B)\n"
			"打开全局任意线路中的任意一个标尺的编辑面板。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actNaviToRuler()));
		act->setShortcut(Qt::CTRL | Qt::Key_B);
		addAction(act);
		panel->addLargeAction(act);

		act = makeAction(QEICN_skylight_edit, tr("天窗编辑"));
		act->setToolTip(tr("天窗编辑\n打开或导航到任意一基线的天窗编辑面板。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actNaviToForbid()));
		forbidMenu = new SARibbonMenu(this);
		act->setMenu(forbidMenu);
		panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = makeAction(QEICN_new_rail, tr("新建线路"));
		connect(act, SIGNAL(triggered()), naviView, SLOT(actAddRailway()));
		act->setToolTip(tr("新建线路\n新建空白的铁路线路"));
		panel->addLargeAction(act);

		panel = cat->addPannel(tr("调整"));

		act = makeAction(QEICN_read_ruler_wizard, tr("标尺综合"));
		act->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_B);
		addAction(act);
		act->setToolTip(tr("标尺综合 (Ctrl+Shift+B)\n"
			"从一组选定的车次中，读取一套标尺数据，用于建立新标尺，或更新既有标尺。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actReadRulerWizard()));
		panel->addLargeAction(act);

		panel->addSeparator();
		act = makeAction(QEICN_locate_to_diagram, tr("定位"));
		act->setToolTip(tr("运行图定位 (Ctrl+G)\n输入时刻，线路及其里程，"
			"快速定位到运行图面板上。"));
		act->setShortcut(Qt::CTRL | Qt::Key_G);
		connect(act, &QAction::triggered, this, &MainWindow::actLocateDiagram);
		addAction(act);
		panel->addLargeAction(act);

		panel = cat->addPannel(tr("路网管理"));

		act = makeAction(QEICN_rail_db, tr("数据库"));
		act->setToolTip(tr("线路数据库 (Ctrl+H)\n"
			"查看、编辑或者导入线路数据库中的基线数据。"));
		connect(act, &QAction::triggered, this, &MainWindow::actRailDBDock);
		act->setShortcut(Qt::CTRL | Qt::Key_H);
		addAction(act);
		panel->addLargeAction(act);

		//menu = new SARibbonMenu(this);
		//auto actsub = menu->addAction(tr("线路数据库 (pyETRC风格)"));
		//addAction(actsub);
		////actsub->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_H);
		//connect(actsub, &QAction::triggered, this, &MainWindow::actRailDB);
		//act->setMenu(menu);

		act = makeAction(QEICN_fast_path, tr("快速径路"));
		act->setToolTip(tr("快速径路生成 (Ctrl+J)\n"
			"通过线路数据库中的数据，给出经由的关键点表，利用最短路算法生成新线路数据。"));
		act->setShortcut(Qt::CTRL | Qt::Key_J);
		addAction(act);
		panel->addMediumAction(act);
		actQuickPath = act;

		act = makeAction(QEICN_route_sel, tr("经由选择"), tr("经由选择-主页"));
		act->setToolTip(tr("交互式经由选择 (Ctrl+K)\n"
			"通过数据库中的数据，手动指定通过邻站、邻线或者区间最短路方式，"
			"生成径路可精确控制的新线路数据。"));
		act->setShortcut(Qt::CTRL | Qt::Key_K);
		addAction(act);
		panel->addMediumAction(act);
		actSelector = act;
	}

	QAction* actRemoveInterp, * actAutoBusiness, * actImportTimetableCsv, * actImportTrainTrf,
		* actAutoCorrection, * actRemoveNonBound, * actRemoveNonBoundTrains, * actRemoveEmptyTrains,
		* actAutoPen;
	//列车
	if constexpr (true) {
		auto* cat = ribbon->addCategoryPage(tr("列车(&3)"));
		auto* panel = cat->addPannel(tr("车次管理"));

		auto* act = actTrainList;
		panel->addLargeAction(act);

		act = makeAction(QEICN_train_batch_op, tr("批量操作"));

		auto* menu = new SARibbonMenu(tr("列车管理扩展"), this);
		act->setMenu(menu);

		auto* actsub = new QAction(tr("删除所有车次"), menu);
		connect(actsub, SIGNAL(triggered()), this, SLOT(removeAllTrains()));
		menu->addAction(actsub);

		actsub = new QAction(tr("删除所有车次和交路"), menu);
		connect(actsub, &QAction::triggered, this, &MainWindow::removeAllTrainsAndRoutings);
		menu->addAction(actsub);

		menu->addSeparator();

		actsub = new QAction(tr("自动始发终到站适配"), menu);
		menu->addAction(actsub);
		connect(actsub, SIGNAL(triggered()), this, SLOT(actAutoStartingTerminal()));

		menu->addAction(tr("自动始发终到站适配 (放宽)"), this,
			&MainWindow::actAutoStartingTerminalLooser);

		menu->addAction(tr("按时刻表重设始发终到站"), this,
			&MainWindow::actResetStartingTerminalFromTimetable);

		actsub = menu->addAction(tr("自动推断所有列车类型"));
		connect(actsub, &QAction::triggered, this, &MainWindow::actAutoTrainType);
		actAutoPen = menu->addAction(tr("自动设置所有列车运行线样式"));

		actAutoBusiness = menu->addAction(tr("自动设置所有营业站"));
		actAutoCorrection = menu->addAction(tr("自动更正时刻表 (测试)"));

		menu->addSeparator();
		actRemoveNonBound = menu->addAction(tr("删除未铺画车站"));

		actRemoveNonBoundTrains = menu->addAction(tr("删除未铺画车次"));
		actRemoveEmptyTrains = menu->addAction(tr("删除空白车次"));

		menu->addSeparator();
		menu->addAction(tr("批量导出列车事件表 (csv)"),this,
			&MainWindow::actBatchExportTrainEventsAll);
		menu->addAction(tr("批量导出列车时刻表 (csv)"), trainListWidget,
			&TrainListWidget::actExportTrainEventListAll);

		panel->addLargeAction(act);
		
		act = makeAction(QEICN_import_trains, tr("导入车次"));
		act->setToolTip(tr("导入车次 (Ctrl+D)\n从既有运行图或者车次数据库文件中导入部分或全部的车次。"));
		act->setShortcut(Qt::CTRL | Qt::Key_D);
		addAction(act);
		connect(act, SIGNAL(triggered()), naviView, SLOT(importTrains()));

		menu = new SARibbonMenu(this);
		actImportTimetableCsv = menu->addAction(tr("导入时刻表 (CSV)"));
		actImportTrainTrf = menu->addAction(tr("批量导入车次 (trf)"));
		act->setMenu(menu);
		panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = makeAction(QEICN_search_train, tr("搜索车次"));
		act->setToolTip(tr("搜索车次 (Ctrl+F)\n搜索车次，将车次设定为当前车次，并高亮运行线。"));
		connect(act, &QAction::triggered, this, &MainWindow::actSearchTrain);
		act->setShortcut(Qt::CTRL | Qt::Key_F);
		addAction(act);
		panel->addMediumAction(act);
		diaActions.search = act;

		act = makeAction(QEICN_new_train, tr("新建车次"));
		act->setToolTip(tr("新建车次 (Ctrl+Shift+C)\n新建空白车次"));
		act->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_C);
		addAction(act);
		diaActions.addTrain = act;
		panel->addMediumAction(act);
		connect(act, SIGNAL(triggered()), naviView, SLOT(actAddTrain()));

		panel = cat->addPannel(tr("工具"));

		act = timetableQuickDock->toggleViewAction();
		act->setIcon(QEICN_timetable_quick);
		act->setShortcut(Qt::CTRL | Qt::Key_I);
		addAction(act);
		act->setObjectName(tr("速览时刻"));
		act->setToolTip(tr("速览时刻 (Ctrl+I)\n"
			"显示或隐藏pyETRC风格的双行只读时刻表。"));
		panel->addLargeAction(act);

		act = trainInfoDock->toggleViewAction();
		act->setIcon(QEICN_train_info_quick);
		act->setShortcut(Qt::CTRL | Qt::Key_Q);
		act->setObjectName(tr("速览信息"));
		addAction(act);
		act->setToolTip(tr("速览信息 (Ctrl+Q)\n"
			"显示或隐藏pyETRC风格的列车信息栏。"));
		panel->addLargeAction(act);

		act = makeAction(QEICN_edit_filters, tr("预设筛选"));
		act->setToolTip(tr("预设筛选器\n编辑运行图预设的列车筛选器"));
		panel->addLargeAction(act);
		connect(act, &QAction::triggered, this, &MainWindow::actEditFilters);

		panel = cat->addPannel(tr("运行线控制"));

		if constexpr (true) {
			auto* sp = new QSpinBox;
			spPassedStations = sp;
			sp->setRange(0, 10000000);
			sp->setToolTip(tr("最大跨越站数\n设置连续运行线段允许跨越的最大站数。"
				"如果区间跨越站数超过指定数值，将被拆分成两段运行线。"));

			auto* w = new QWidget;
			auto* vlay = new QVBoxLayout;
			vlay->addWidget(new QLabel(tr("最大跨越站数")));
			vlay->addWidget(sp);
			w->setLayout(vlay);
			w->setObjectName(tr("最大跨越站数面板"));
			w->setWindowTitle(tr("最大跨越站数"));

			panel->addWidget(w, SARibbonPannelItem::Large);

			act = makeAction(QEICN_apply_pass_stations, tr("应用"), tr("应用最大跨越站数"));
			connect(act, SIGNAL(triggered()), this, SLOT(actChangePassedStations()));
			panel->addLargeAction(act);
		}

		panel = cat->addPannel(tr("选项"));
		act = makeAction(QEICN_weaken_unselect, tr("背景虚化"));
		act->setToolTip(tr("背景虚化\n若启用，则在选中列车运行线时，自动虚化其他列车运行线。"
			"点击空白处取消选择则取消虚化。\n"
			"若此功能出现问题，请刷新运行图。"));
		act->setCheckable(true);
		act->setChecked(SystemJson::instance.weaken_unselected);
		connect(act, &QAction::triggered, this, &MainWindow::actToggleWeakenUnselected);
		panel->addLargeAction(act);

		panel = cat->addPannel(tr("交路"));
		act = routingDock->toggleViewAction();
		act->setObjectName(tr("交路面板开关"));
		act->setIcon(QEICN_routing_edit);

		routingMenu = new SARibbonMenu(tr("打开的交路编辑"), this);
		act->setMenu(routingMenu);

		panel->addLargeAction(act, QToolButton::MenuButtonPopup);

		act = makeAction(QEICN_routing_batch_parse, tr("批量解析"));
		connect(act, &QAction::triggered, this, &MainWindow::actBatchParseRouting);
		act->setToolTip(tr("批量文本解析\n批量地从车次套用顺序文本中解析出交路数据"));
		panel->addMediumAction(act);

		act = makeAction(QEICN_routing_batch_identify, tr("批量识别"));
		connect(act, &QAction::triggered, this, &MainWindow::actBatchDetectRouting);
		act->setToolTip(tr("批量识别车次\n对所有交路，尝试识别车次，使得虚拟车次转变为实体车次。"));
		panel->addMediumAction(act);

		panel = cat->addPannel(tr("分析"));
		act = makeAction(QEICN_compare_trains, tr("车次对照"));
		act->setToolTip(tr("两车次运行对照\n在指定线路上，对比两个选定车次的运行情况。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actTrainDiff()));
		panel->addMediumAction(act);
		diaActions.trainRef = act;

		act = makeAction(QEICN_timetable_diagon, tr("时刻诊断"), tr("全局时刻诊断"));
		act->setToolTip(tr("车次时刻诊断\n诊断指定车次或所有车次的时刻表是否存在"
			"可能的问题，例如到开时刻填反。"));
		connect(act, &QAction::triggered, this, &MainWindow::actDiagnose);
		panel->addMediumAction(act);

		act = makeAction(QEICN_compare_diagram, tr("运行图对比"));
		act->setToolTip(tr("运行图对比\n对比本运行图和指定运行图文件的列车时刻表差异。"));
		connect(act, &QAction::triggered, this, &MainWindow::actDiagramCompare);
		panel->addLargeAction(act);

		act = makeAction(QEICN_section_count, tr("区间对数表"));
		act->setToolTip(tr("区间车次表\n查询指定线路从指定站出发（到达指定站）的列车对数。"));
		connect(act, &QAction::triggered, this, &MainWindow::actIntervalCount);
		panel->addMediumAction(act);

		act = makeAction(QEICN_section_trains, tr("区间车次表"));
		act->setToolTip(tr("区间车次表\n查询任意两站之间的列车表。"));
		connect(act, &QAction::triggered, this, &MainWindow::actIntervalTrains);
		panel->addMediumAction(act);

		act = makeAction(QEICN_interval_stat, tr("运行统计"));
		act->setToolTip(tr("列车区间运行统计\n计算时刻表中给定两站之间的运行时间、里程、速度等。"));
		connect(act, &QAction::triggered, this, &MainWindow::actTrainIntervalStat);
		panel->addLargeAction(act);

		panel = cat->addPannel(tr("排图"));

		act = makeAction(QEICN_ruler_paint, tr("标尺排图"));
		addAction(act);
		act->setShortcut(Qt::CTRL | Qt::Key_R);
		act->setToolTip(tr("标尺排图向导 (Ctrl+R)\n使用指定线路的指定标尺，"
			"铺画新的列车运行线，或者重新铺画既有列车运行线的一部分。"));
		panel->addLargeAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(actRulerPaint()));
		diaActions.rulerPaint = act;

		act = makeAction(QEICN_batch_copy_trains, tr("批量复制"));
		act->setToolTip(tr("批量复制运行线\n"
			"将指定车次的运行线按照其起始时刻复制若干个（车次不同的）副本。"));
		connect(act, SIGNAL(triggered()), this, SLOT(actBatchCopyTrain()));
		diaActions.batchCopy = act;
		panel->addMediumAction(act);

		act = makeAction(QEICN_timetable_interp, tr("时刻插值"));
		act->setToolTip(tr("时刻插值（推定通过站时刻）\n"
			"通过标尺数据，推定列车时刻表中未给出的通过站的时刻。"));
		connect(act, &QAction::triggered, this, &MainWindow::actInterpolation);

		menu = new SARibbonMenu(this);
		actRemoveInterp = menu->addAction(tr("删除所有推定结果"));
		act->setMenu(menu);
		panel->addMediumAction(act, QToolButton::MenuButtonPopup);
		//btn->setPopupMode(QToolButton::DelayedPopup);

		act = makeAction(QEICN_greedy_paint, tr("贪心排图"));
		addAction(act);
		act->setShortcut(Qt::CTRL | Qt::Key_T);
		act->setToolTip(tr("自动贪心排图向导 (Ctrl+T)\n使用指定线路的指定标尺，"
			"采用贪心算法自动计算运行线。"));
		diaActions.greedyPaint = act;

		menu = new SARibbonMenu(this);
		menu->addAction(tr("退出贪心排图状态"), this, &MainWindow::actExitGreedyPaint);
		act->setMenu(menu);

		panel->addLargeAction(act, QToolButton::MenuButtonPopup);
		connect(act, &QAction::triggered, this, &MainWindow::actGreedyPaint);

#ifdef QETRC_GREEDYPAINT_TEST
		act = new QAction(QIcon(":/icons/ruler_pen.png"), tr("贪心排图（测试）"), this);
		addAction(act);
		act->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_T);
		act->setToolTip(tr("自动贪心排图快速测试 (Ctrl+Shift+T)\n使用指定线路的指定标尺，"
			"采用贪心算法自动计算运行线。"));
		btn = panel->addLargeAction(act);
		connect(act, &QAction::triggered, this, &MainWindow::actGreedyPaintFast);
#endif

	}


	//显示
	if constexpr (true) {
		auto* cat = ribbon->addCategoryPage(tr("显示(&4)"));
		catView = new ViewCategory(this, cat, this);
		connect(trainListWidget->getModel(), &TrainListModel::trainShowChanged,
			catView, &ViewCategory::actChangeSingleTrainShow);
		connect(actGlobalConfig, &QAction::triggered, catView,
			&ViewCategory::actSystemJsonDialog);
	}

	//context: page 5
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextPage = new PageContext(_diagram, cat, this);
		connect(contextPage, &PageContext::pageRemoved,
			naviView, &NaviTree::removePage);
		connect(contextPage, &PageContext::pageNameChanged,
			naviModel, &DiagramNaviModel::onPageNameChanged);
		connect(naviView, &NaviTree::onEditRailwayFromPageContext,
			contextPage, &PageContext::actEditRailway);
		connect(naviView, &NaviTree::onSwitchToRailwayFromPageContext,
			contextPage, &PageContext::actSwitchToRailway);
		connect(contextPage, &PageContext::dulplicatePage,
			naviView, &NaviTree::actDulplicatePage);
	}

	//context: train 6 7
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextTrain = new TrainContext(_diagram, cat, this);

		connect(contextTrain, &TrainContext::timetableChanged,
			trainListWidget->getModel(), &TrainListModel::onTrainChanged);
		//connect(contextTrain, &TrainContext::actRemoveTrain,
		//	naviView, &NaviTree::removeSingleTrain);
		connect(naviView, &NaviTree::removeTrainNavi,
			contextTrain, &TrainContext::actRemoveSingleTrain);
		connect(naviView, &NaviTree::editTimetable,
			contextTrain, &TrainContext::actShowBasicWidget);
		connect(naviView, &NaviTree::editTrain,
			contextTrain, &TrainContext::actShowEditWidget);
		connect(trainListWidget, &TrainListWidget::editTrain,
			contextTrain, &TrainContext::actShowEditWidget);

		connect(naviModel, &DiagramNaviModel::newTrainAdded,
			this, &MainWindow::onNewTrainAdded);
		connect(naviModel, &DiagramNaviModel::undoneAddTrain,
			this, &MainWindow::undoAddNewTrain);
		connect(contextTrain, &TrainContext::focusInRouting,
			this, &MainWindow::focusInRouting);
		connect(trainInfoWidget, &TrainInfoWidget::editTimetable,
			contextTrain, &TrainContext::showBasicWidget);
		connect(trainInfoWidget, &TrainInfoWidget::editTrain,
			contextTrain, &TrainContext::showEditWidget);

		connect(timetableQuickWidget->getModel(),
			&TimetableQuickEditableModel::trainStationTimeUpdated,
			contextTrain, &TrainContext::onTrainStationTimeChanged);

		connect(actRemoveInterp, &QAction::triggered,
			contextTrain, &TrainContext::actRemoveInterpolation);
		connect(timetableQuickWidget, &TimetableQuickWidget::locateToBoundStation,
			contextTrain, &TrainContext::locateToBoundStation);
		connect(actAutoBusiness, &QAction::triggered,
			contextTrain, &TrainContext::actAutoBusiness);
		connect(actAutoPen, &QAction::triggered,
			contextTrain, &TrainContext::actAutoPenAll);
		connect(actAutoCorrection, &QAction::triggered,
			contextTrain, &TrainContext::actAutoCorrectionAll);
		connect(actRemoveNonBound, &QAction::triggered,
			contextTrain, &TrainContext::actRemoveNonBound);
		connect(actRemoveNonBoundTrains, &QAction::triggered,
			contextTrain, &TrainContext::actRemoveNonBoundTrains);
		connect(actRemoveEmptyTrains, &QAction::triggered,
			contextTrain, &TrainContext::actRemoveEmptyTrains);

		connect(actImportTimetableCsv, &QAction::triggered,
			contextTrain, &TrainContext::actImportTrainFromCsv);

		connect(actImportTrainTrf, &QAction::triggered,
			contextTrain, &TrainContext::actImportTrainFromTrf);

		connect(contextTrain, &TrainContext::dulplicateTrain,
			naviView, &NaviTree::actDulplicateTrain);

		connect(trainListWidget->getModel(), &TrainListModel::onTypeBatchChanged,
			contextTrain, &TrainContext::refreshData);

		// 原则上这种用到了局部变量的都必须得DirectConnection才内存安全，虽然这是默认行为
		connect(trainListWidget, &TrainListWidget::batchChangeStartingTerminal,
			contextTrain, &TrainContext::actBatchChangeStartingTerminal, Qt::DirectConnection);

		connect(trainListWidget, &TrainListWidget::batchAutoChangeType,
			contextTrain, &TrainContext::actBatchAutoTrainType, Qt::DirectConnection);

		connect(trainListWidget, &TrainListWidget::batchExportTrainEventList,
			contextTrain, &TrainContext::batchExportTrainEvents, Qt::DirectConnection);

		connect(trainListWidget, &TrainListWidget::batchAutoBusiness,
			contextTrain, &TrainContext::actAutoBusinessBat, Qt::DirectConnection);

		connect(trainListWidget, &TrainListWidget::batchAutoPen,
			contextTrain, &TrainContext::batchAutoTrainPen);

		connect(trainListWidget, &TrainListWidget::batchAutoCorrect,
			contextTrain, &TrainContext::actAutoCorrectionBat, Qt::DirectConnection);

		connect(trainListWidget, &TrainListWidget::removeTrains,
			contextTrain, &TrainContext::actRemoveTrains);
	}

	//context: rail 8
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextRail = new RailContext(_diagram, cat, this, this);
		connect(contextRail, &RailContext::railNameChanged,
			naviModel, &DiagramNaviModel::onRailNameChanged);
		//connect(contextRail, &RailContext::stationTableChanged,
		//	this, &MainWindow::onStationTableChanged);
		connect(contextRail, &RailContext::selectRuler,
			this, &MainWindow::focusInRuler);
		connect(contextRail, &RailContext::rulerInsertedAt,
			naviModel, &DiagramNaviModel::insertRulerAt);
		connect(contextRail, &RailContext::rulerRemovedAt,
			naviModel, &DiagramNaviModel::removeRulerAt);
		connect(naviView, &NaviTree::editForbid,
			contextRail, &RailContext::openForbidWidgetTab);
		connect(naviView, &NaviTree::actRemoveRailwayAt,
			contextRail, &RailContext::removeRailwayAtU);
		connect(contextRail, &RailContext::dulplicateRailway,
			naviView, &NaviTree::actDulplicateRailway);
		connect(naviView, &NaviTree::importRailways,
			contextRail, &RailContext::actImportRailways);
	}

	//context: ruler 9
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextRuler = new RulerContext(_diagram, cat, this);
		connect(contextRuler, &RulerContext::ordinateRulerModified,
			this, qOverload<std::shared_ptr<Railway>>(&MainWindow::updateRailwayDiagrams));
		connect(contextRuler, &RulerContext::focusOutRuler,
			this, &MainWindow::focusOutRuler);
		connect(contextRuler, &RulerContext::rulerNameChanged,
			naviModel, &DiagramNaviModel::onRulerNameChanged);
		connect(naviView, &NaviTree::editRuler,
			contextRail, &RailContext::openRulerWidget);
		connect(naviView, &NaviTree::removeRulerNavi,
			contextRuler, &RulerContext::actRemoveRulerNavi);
		connect(naviView, &NaviTree::dulplicateRuler,
			contextRuler, &RulerContext::dulplicateRuler);
		connect(naviView, &NaviTree::mergeRuler,
			contextRuler, &RulerContext::mergeWith);
	}

	//context: routing 0
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextRouting = new RoutingContext(_diagram, cat, this);

		connect(naviView, &NaviTree::editRouting,
			contextRouting, &RoutingContext::openRoutingEditWidget);
		connect(routingWidget, &RoutingWidget::editRouting,
			contextRouting, &RoutingContext::openRoutingEditWidget);
		connect(routingWidget->getModel(), &RoutingCollectionModel::routingHighlightChanged,
			contextRouting, &RoutingContext::onRoutingHighlightChanged);
		connect(contextRouting, &RoutingContext::routingHighlightChanged,
			routingWidget->getModel(), &RoutingCollectionModel::onHighlightChangedByContext);
		connect(contextRouting, &RoutingContext::routingInfoChanged,
			routingWidget->getModel(), &RoutingCollectionModel::onRoutingInfoChanged);
		connect(routingWidget, &RoutingWidget::nonEmptyRoutingAdded,
			this, &MainWindow::repaintRoutingTrainLines);
		connect(routingWidget, &RoutingWidget::routingRemoved,
			contextRouting, &RoutingContext::onRoutingRemoved);
		connect(contextRouting, &RoutingContext::removeRouting,
			routingWidget, &RoutingWidget::onRemoveRoutingFromContext);
	}

	// context: path (A)
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextPath = new PathContext(_diagram, cat, this, this);

		connect(naviView, &NaviTree::editPathNavi,
			contextPath, &PathContext::openPathEditIndex);
		connect(pathListWidget, &PathListWidget::editPath,
			contextPath, &PathContext::openPathEditIndex);
		connect(pathListWidget, &PathListWidget::removePath,
			contextPath, &PathContext::removePath);
		connect(naviView, &NaviTree::removePathNavi,
			contextPath, &PathContext::removePath);
		connect(pathListWidget, &PathListWidget::emptyPathAdded,
			contextPath, &PathContext::openPathEdit);
	}

	// context: DB
	if constexpr (true) {
		auto* cat = ribbon->addContextCategory(tr(""));
		contextDB = new RailDBContext(cat, this);
		connect(contextDB->getNavi(), &RailDBNavi::exportRailwayToDiagram,
			naviView, &NaviTree::importRailwayFromDB);
		connect(actQuickPath, &QAction::triggered,
			contextDB, &RailDBContext::activateQuickSelector);
		connect(contextDB, &RailDBContext::exportRailToDiagram,
			naviView, &NaviTree::importRailwayFromDB);
		connect(actSelector, &QAction::triggered, contextDB,
			&RailDBContext::actPathSelector);
	}

	if constexpr (true) {
		auto* w = centralOccupyWidget;
		auto* vlay = new QVBoxLayout(w);
		auto* btn = new SARibbonToolButton(sharedActions.open, this);
		vlay->addWidget(btn);

		btn = new SARibbonToolButton(sharedActions.newfile, this);
		vlay->addWidget(btn);
	}

	//ApplicationMenu的初始化放在最后，因为可能用到前面的..
	initAppMenu();
	connect(ribbon->applicationButton(), SIGNAL(clicked()), this,
		SLOT(actPopupAppButton()));

	ribbon->setRibbonStyle(
		static_cast<SARibbonBar::RibbonStyles>(SystemJson::instance.ribbon_style));
	// 2024.03.28: theme and align
	ribbon->setRibbonAlignment(SystemJson::instance.ribbon_align_center ? 
		SARibbonAlignment::AlignCenter : SARibbonAlignment::AlignLeft);
	setRibbonTheme(static_cast<SARibbonMainWindow::RibbonTheme>(SystemJson::instance.ribbon_theme));

	// 2022.04.24：测试ActionManager
	// 2023.12.20: seems changed in SARibbon V1.0.7
	actMgr = new SARibbonActionsManager(this->ribbonBar());
	//actMgr->autoRegisteActions(this->ribbonBar());   // 2024.04.01  SARibbonBar V2.0.3: already registed in ctor
}

void MainWindow::initAppMenu()
{
	auto* menu = new SARibbonMenu(tr("qETRC"), this);

	menu->addAction(sharedActions.newfile);
	menu->addAction(sharedActions.open);
	menu->addAction(sharedActions.save);
	menu->addAction(sharedActions.saveas);

	menu->addSeparator();

	appMenu = menu;

	resetRecentActions();
}

void MainWindow::loadInitDiagram(const QString& cmdFile)
{
	do {
		if (!cmdFile.isEmpty() && openGraph(cmdFile))
			break;
		if (openGraph(SystemJson::instance.last_file))
			break;
		if (openGraph(SystemJson::instance.default_file))
			break;
	} while (false);
	markUnchanged();
}

bool MainWindow::clearDiagram()
{
	clearDiagramUnchecked();
	return true;
}

void MainWindow::clearDiagramUnchecked()
{
	//删除打开的所有运行图面板
	pageMenu->clear();
	for (auto p : diagramDocks) {
		manager->removeDockWidget(p);
		p->setParent(nullptr);
		delete p;
	}
	diagramDocks.clear();
	diagramWidgets.clear();
	focusOutPage();

	//删除所有打开的基线编辑面板
	for (auto p : railStationDocks) {
		p->deleteDockWidget();
	}
	railStationDocks.clear();
	railStationWidgets.clear();
	focusOutRailway();

	contextTrain->removeAllTrainWidgets();
	focusOutTrain();
	contextRail->removeAllDocks();
	contextPath->removePathDocks();
	contextRouting->removeAllRoutingDocks();

	// 2024.04.13: clear the two quick widgets
	timetableQuickWidget->resetTrain();
	trainInfoWidget->setTrain(nullptr);

	undoStack->clear();
	undoStack->resetClean();

	// 2022.05.19 新增
	focusOutRuler();

	//2022.04.10：如果有贪心推线窗口在运行，取消掉
	if (greedyWidget) {
		greedyWidget->close();
		greedyWidget->deleteLater();
		greedyWidget = nullptr;

	}

	//最后：清理数据
	_diagram.clear();
}

void MainWindow::beforeResetGraph()
{
}

void MainWindow::actNewGraph()
{
	if (changed && !saveQuestion())
		return;
	clearDiagram();
	_diagram.setFilename("");
	endResetGraph();
	showStatus(tr("新建空白运行图"));
}

void MainWindow::endResetGraph()
{
	resetDiagramPages();

	//导航窗口
	naviModel->resetModel();
	trainListWidget->refreshData();
	routingWidget->refreshData();
	pathListWidget->refreshData();
	catView->refreshTypeGroup();
	catView->refreshFilters();

	spPassedStations->setValue(_diagram.config().max_passed_stations);

	// 2023.12.30  predef train filter
	if (filterManager) {
		filterManager->refreshData();
	}

	updateWindowTitle();
}

void MainWindow::resetDiagramPages()
{
	if (_diagram.pages().empty() && !_diagram.isNull())
		_diagram.createDefaultPage();
	for (auto p : _diagram.pages()) {
		addPageWidget(p);
	}
}

bool MainWindow::openGraph(const QString& filename)
{
	// 2022.09.11: 把打开的计时和提示都放到这里来。
	using namespace std::chrono_literals;
	auto start = std::chrono::system_clock::now();
	qInfo() << "Opening diagram file " << filename;

	clearDiagramUnchecked();

	Diagram dia;
	dia.readDefaultConfigs();   //暂定这里读取一次默认配置，防止move时丢失数据
	bool flag = dia.fromJson(filename);

	if (flag && !dia.isNull()) {
		beforeResetGraph();
		_diagram = std::move(dia);   //move assign
		endResetGraph();
		addRecentFile(filename);
		auto end = std::chrono::system_clock::now();
		showStatus(tr("打开运行图文件%1成功  用时%2毫秒").arg(filename)
			.arg((end - start) / 1ms));
		// 这里面可能有输出提示的
		checkOpenFile();
		return true;
	}
	else {
		qWarning() << "open failed: " << filename;
		return false;
	}
}

void MainWindow::addTrainLine(Train& train)
{
	for (auto p : diagramWidgets)
		p->paintTrain(train);
}

void MainWindow::removeTrainLine(const Train& train)
{
	for (auto p : diagramWidgets)
		p->removeTrain(train);
}

void MainWindow::addTrainLineTmp(std::shared_ptr<Train> train)
{
	for (auto p : diagramWidgets)
		p->paintTrainTmp(train);
}

void MainWindow::actChangeStationName()
{
	auto* dialog = new ChangeStationNameDialog(_diagram, this);
	connect(dialog, &ChangeStationNameDialog::nameChangeApplied,
		this, &MainWindow::applyChangeStationName);
	dialog->show();
}

void MainWindow::actNaviToRuler()
{
	auto r = RailRulerCombo::dialogGetRuler(_diagram.railCategory(), this,
		tr("导航到标尺"), tr("请选择标尺，系统将打开或切换到该标尺编辑页面"));
	if (r) {
		contextRail->openRulerWidget(r);
	}
}

void MainWindow::actNaviToForbid()
{
	auto r = SelectRailwayCombo::dialogGetRailway(_diagram.railCategory(), this,
		tr("选择线路"), tr("请选择要编辑天窗的基线数据"));
	if (r) {
		contextRail->openForbidWidget(r);
	}
}

void MainWindow::actToSingleFile()
{
	auto* dialog = new OutputSubDiagramDialog(_diagram, this);
	dialog->show();
}

void MainWindow::actBatchCopyTrain()
{
	auto* dialog = new BatchCopyTrainDialog(_diagram,
		contextTrain->getTrain(), this);
	connect(dialog, &BatchCopyTrainDialog::applied,
		naviView, &NaviTree::actBatchAddTrains);
	dialog->show();
}

void MainWindow::actResetStartingTerminalFromTimetable()
{
	trainListWidget->actResetStartingTerminalFromTimetableAll();
}

void MainWindow::actAutoStartingTerminal()
{
	trainListWidget->actAutoStartingTerminalAll();
}

void MainWindow::actAutoStartingTerminalLooser()
{
	trainListWidget->actAutoStartingTerminalLooserAll();
}

void MainWindow::actBatchExportTrainEventsAll()
{
	contextTrain->batchExportTrainEvents(_diagram.trainCollection().trains());
}

void MainWindow::actTrainDiff()
{
	auto* dialog = new TrainDiffDialog(_diagram, this);
	dialog->open();
}

void MainWindow::actDiagramCompare()
{
	auto* dialog = new DiagramCompareDialog(_diagram, this);
	connect(dialog, &DiagramCompareDialog::showStatus, this, &MainWindow::showStatus);
	dialog->resize(600, 600);
	dialog->open();
}

void MainWindow::actIntervalTrains()
{
	auto* dlg = new IntervalTrainDialog(_diagram, this);
	dlg->show();
}

void MainWindow::actIntervalCount()
{
	auto* dlg = new IntervalCountDialog(_diagram, this);
	dlg->show();
}

void MainWindow::actTrainIntervalStat()
{
	auto* dlg = new TrainIntervalStatDialog(_diagram, this);
	dlg->show();
}

void MainWindow::actReadRulerWizard()
{
	auto* wzd = new ReadRulerWizard(_diagram, this);
	connect(wzd, &ReadRulerWizard::rulerAdded,
		contextRail, &RailContext::actAddNamedNewRuler, Qt::DirectConnection);
	connect(wzd, &ReadRulerWizard::rulerUpdated,
		contextRuler, &RulerContext::actChangeRulerData);
	wzd->show();
}

void MainWindow::actBatchParseRouting()
{
	auto* dialog = new BatchParseRoutingDialog(_diagram.trainCollection(), this);
	connect(dialog, &BatchParseRoutingDialog::routingsParsed,
		routingWidget, &RoutingWidget::onRoutingsAdded);
	dialog->show();
}

void MainWindow::actBatchDetectRouting()
{
	auto* dialog = new BatchDetectRoutingDialog(_diagram.trainCollection(), this);
	connect(dialog, &BatchDetectRoutingDialog::detectApplied,
		contextRouting, &RoutingContext::actBatchRoutingUpdate);
	dialog->open();
}

void MainWindow::actDiagnose()
{
	auto* d = new DiagnosisDialog(_diagram, this);
	connect(d, &DiagnosisDialog::showStatus, this, &MainWindow::showStatus);
	connect(d->getModel(), &DiagnosisModel::locateToRailMile,
		this, &MainWindow::locateToRailMile);
	d->show();
}

void MainWindow::actToggleWeakenUnselected(bool on)
{
	SystemJson::instance.weaken_unselected = on;
}

void MainWindow::showQuickTimetable(std::shared_ptr<Train> train)
{
	if (timetableQuickDock->isClosed()) {
		timetableQuickDock->toggleView(true);
		timetableQuickWidget->setTrain(train);
	}
	else {
		timetableQuickDock->setAsCurrentTab();
	}
}

void MainWindow::actAutoTrainType()
{
	trainListWidget->actAutoTrainTypeAll();
}

void MainWindow::actLocateDiagram()
{
	if (!locateDialog) {
		initLocateDialog();
	}
	locateDialog->showDialog();
}

void MainWindow::initLocateDialog()
{
	locateDialog = new LocateDialog(_diagram, this);
	connect(locateDialog, &LocateDialog::locateOnMile,
		this, &MainWindow::locateDiagramOnMile);
	connect(locateDialog, &LocateDialog::locateOnStation,
		this, &MainWindow::locateDiagramOnStation);
}

void MainWindow::locateToRailMile(std::shared_ptr<const Railway> rail, double mile, const QTime& time)
{
	if (!locateDialog) {
		initLocateDialog();
	}
	locateDialog->locateToRail(rail, mile, time);
}


void MainWindow::actInterpolation()
{
	auto* dlg = new TimeInterpWizard(_diagram, this);
	connect(dlg, &TimeInterpWizard::interpolationApplied,
		contextTrain, &TrainContext::actInterpolation);
	dlg->show();
}

void MainWindow::actRailDB()
{
	QMessageBox::information(this, tr("注意"), tr("线路数据库功能尚未完成，现在只提供"
		"基线数据浏览和导入线路到当前运行图。对数据库所做的更改可能无法正确保存。\n"
		"提供了入口并且没有显式未实现提示的功能是已经实现了的。\n"
		"导入线路到运行图操作，请在右键菜单中完成。"
	));
	auto* window = new RailDBWindow(this);
	window->getNavi()->openDB(SystemJson::instance.default_raildb_file);
	connect(window, &RailDBWindow::exportRailwayToDiagram,
		naviView, &NaviTree::importRailwayFromDB);
	window->show();
}

void MainWindow::actRailDBDock()
{
	contextDB->activateDB();
}

void MainWindow::closeCurrentTab()
{
	if (auto* w = manager->focusedDockWidget()) {
		w->closeDockWidget();
	}
}

void MainWindow::actRibbonConfig()
{
	auto* d = new RibbonConfigDialog(this);
	connect(d, &RibbonConfigDialog::configApplied, this, &MainWindow::onRibbonConfigChanged);
	d->show();
}

void MainWindow::onRibbonConfigChanged(bool theme_changed)
{
	const auto& d = SystemJson::instance;
	ribbonBar()->setRibbonStyle(static_cast<SARibbonBar::RibbonStyles>(d.ribbon_style));
	ribbonBar()->setRibbonAlignment(
		d.ribbon_align_center ? SARibbonAlignment::AlignCenter : SARibbonAlignment::AlignLeft);
	if (theme_changed) {
		setRibbonTheme(static_cast<SARibbonMainWindow::RibbonTheme>(d.ribbon_theme));
	}
}

void MainWindow::updateWindowTitle()
{
	QString filename = _diagram.filename();
	if (filename.isNull())
		filename = "新运行图";
	QString s = QString(qespec::TITLE.data()) + "_" + qespec::VERSION.data() + " - ";
	//暂定用自己维护的标志位来判定修改
	if (changed)
		s += "* ";
	s += filename;
	setWindowTitle(s);
}

bool MainWindow::saveQuestion()
{
	auto flag = QMessageBox::question(this, tr(qespec::TITLE.data()),
		tr("是否保存对运行图文件[%1]的更改？").arg(_diagram.filename()),
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
	if (flag == QMessageBox::Yes) {
		actSaveGraph();
		return true;
	}
	else if (flag == QMessageBox::No) {
		return true;
	}
	else {
		//包括Cancel和NoButton
		return false;
	}
}

bool MainWindow::checkOpenFile()
{
	QString text;
	int level = 0;

	// check rail
	foreach(auto rail, _diagram.railways()) {

		auto& sts = rail->stations();
		auto itr = std::is_sorted_until(sts.begin(), sts.end(), RailStationMileLess());
		if (itr != sts.end()) {
			text.append(
				tr("[ERROR] 线路[%1]不符合里程标单调规则，请前往线路编辑页面调整线路数据。"
					"请检查[%2]站附近（第%3行）\n").arg(rail->name(), (*itr)->name.toSingleLiteral())
				.arg(std::distance(sts.begin(), itr))
			);
			level = 2;
		}

		auto it = rail->firstNegativeInterval();
		if (it) {
			text.append(
				tr("[ERROR] 线路[%1]存在非法区间：区间[%2]里程[%3] km为负值。\n")
				.arg(rail->name(), it->toString()).arg(it->mile())
			);
			level = 2;
		}

		int idx = rail->firstInvalidNameIndex();
		if (idx != -1) {
			text.append(
				tr("[ERROR] 线路[%1]存在非法站名：与前序站名重复或为空 [%2] （第%3位）\n")
				.arg(rail->name(), rail->stations().at(idx)->name.toSingleLiteral())
				.arg(idx + 1)
			);
			level = 2;
		}

	}

	if (_diagram.releaseCode() > qespec::RELEASE_CODE) {
		text.append(
			tr("[INFO] 运行图打开成功，但生成当前运行图所用的软件版本[%1]高于当前软件版本[%2]，"
				"保存数据时请注意防止数据丢失。\n").arg(_diagram.version(),
					QString(qespec::VERSION.data())));
		level = 1;
	}

	if (level) {
		auto* box = new QMessageBox(this);
		box->setAttribute(Qt::WA_DeleteOnClose);
		box->setWindowTitle(tr("提示"));
		text.prepend(tr("程序检查发现当前打开的运行图存在下列问题，请妥善处理以防止程序出现意外：\n"));
		box->setText(text);
		box->setWindowModality(Qt::NonModal);
		if (level == 2) {
			box->setIcon(QMessageBox::Warning);
		}
		box->show();
	}
	
	return level < 2;
}

QAction* MainWindow::makeAction(const QIcon& icon, const QString& text)
{
	auto* act = new QAction(icon, text, this);
	act->setObjectName(text);
	return act;
}

QAction* MainWindow::makeAction(const QIcon& icon, const QString& text, const QString& objName)
{
	auto* act = new QAction(icon, text, this);
	act->setObjectName(objName);
	return act;
}

QAction* MainWindow::makeAction(const QString& text)
{
	auto* act = new QAction(text, this);
	act->setObjectName(text);
	return act;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	if (!contextDB->deactiveOnClose()) {
		e->ignore();
		return;
	}

	if (changed && !saveQuestion())
		e->ignore();
	else
		e->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
	bool flag = false;
	const auto* mime = e->mimeData();
	if (mime->hasUrls()) {
		auto t = mime->urls()[0];
		if (t.isLocalFile()) {
			flag = true;
		}
	}
	if (flag) {
		e->acceptProposedAction();
	}
	else {
		e->ignore();
	}
}

void MainWindow::dropEvent(QDropEvent* e)
{
	bool flag = false;
	const auto* mime = e->mimeData();
	if (mime->hasUrls()) {
		auto t = mime->urls()[0];
		if (t.isLocalFile()) {
			flag = true;
			openFileChecked(t.toLocalFile());
			e->acceptProposedAction();
			qDebug() << "[Drop] Filename: " << t.toLocalFile() << Qt::endl;
		}
		else {
			qDebug() << "[Drop] Non local file!" << Qt::endl;
		}
	}
	if (!flag) {
		e->ignore();
	}
}

void MainWindow::informPageListChanged()
{
	naviModel->resetModel();
	markChanged();
}

void MainWindow::actOpenRailStationWidget(std::shared_ptr<Railway> rail)
{
	for (int i = 0; i < railStationWidgets.count(); i++) {
		if (railStationWidgets.at(i)->getRailway() == rail) {
			manager->setDockWidgetFocused(railStationDocks.at(i));
			auto* dock = railStationDocks.at(i);
			if (dock->isClosed())
				dock->toggleView(true);
			else
				dock->setAsCurrentTab();
			return;
		}
	}
	//创建
	auto* w = new RailStationWidget(_diagram.railCategory(), false);
	auto* dock = new ads::CDockWidget(tr("基线编辑 - %1").arg(rail->name()));
	auto* act = dock->toggleViewAction();
	railMenu->addAction(act);
	dock->setWidget(w);
	w->setRailway(rail);
	connect(w, &RailStationWidget::railNameChanged,
		contextRail, &RailContext::actChangeRailName);
	connect(w->getModel(), &RailStationModel::actStationTableChanged,
		contextRail, &RailContext::actUpdateTimetable);
	connect(w, &RailStationWidget::focusInRailway,
		this, &MainWindow::focusInRailway);
	connect(w, &RailStationWidget::railNoteChanged,
		contextRail, &RailContext::actUpdateRailNotes);
	railStationWidgets.append(w);
	railStationDocks.append(dock);

	manager->addDockWidgetFloating(dock);
}

void MainWindow::commitPassedStationChange(int n)
{
	_diagram.config().max_passed_stations = n;
	_diagram.rebindAllTrains();
	trainListWidget->getModel()->updateAllMileSpeed();
	updateAllDiagrams();
}

void MainWindow::refreshAll()
{
	auto start = std::chrono::system_clock::now();
	//qInfo() << "Refreshing diagram";
	//_diagram.refreshAll();
	updateAllDiagrams();
	//直接由Main管理的子页面
	naviModel->resetModel();
	trainListWidget->refreshData();
	pathListWidget->refreshData();
	for (auto p : railStationWidgets) {
		p->refreshData();
	}
	catView->refreshTypeGroup();
	catView->refreshFilters();
	//更新各个context，由context负责更新自己管理的widget的数据
	contextTrain->refreshAllData();
	contextRail->refreshAllData();
	contextRuler->refreshAllData();
	contextPage->refreshAllData();
	updateWindowTitle();
	auto end = std::chrono::system_clock::now();
	showStatus(tr("全部刷新完毕  用时 %1 毫秒").arg((end - start) / std::chrono::milliseconds(1)));
}

void MainWindow::rebindAndRefresh()
{
	auto ret = QMessageBox::question(this, tr("重置状态"),
		tr("重置状态：此操作重新绑定列车与线路并刷新所有面板状态。\n"
			"此操作不可撤销，且将使得以往所有操作不可撤销，请谨慎操作！是否继续？"));
	if (ret != QMessageBox::Yes)
		return;
	ret = QMessageBox::question(this, tr("确认"),
		tr("再次确认：此操作不可撤销，且将使得以往所有操作都不可撤销！\n"
			"除非线路与列车绑定数据出现明显问题，否则建议使用[刷新]功能。\n"
			"是否确认？"));
	if (ret != QMessageBox::Yes)
		return;

	auto start = std::chrono::steady_clock::now();
	undoStack->clear();
	_diagram.refreshAll();
	qInfo() << tr("重新绑定所有列车 用时 %1 毫秒").arg((std::chrono::steady_clock::now() - start) / std::chrono::milliseconds(1));

	refreshAll();
}

void MainWindow::applyChangeStationName(const ChangeStationNameData& data)
{
	undoStack->push(new qecmd::ChangeStationNameGlobal(data, this));
}

void MainWindow::setRoutingHighlight(std::shared_ptr<Routing> routing, bool on)
{
	if (on) {
		foreach(auto * w, diagramWidgets) {
			w->highlightRouting(routing);
		}
	}

	else {
		foreach(auto * w, diagramWidgets) {
			w->unhighlightRouting(routing);
		}
	}

}

void MainWindow::updateTrainLines(std::shared_ptr<Train> train,
	QVector<std::shared_ptr<TrainAdapter>>&& adps)
{
	for (auto p : diagramWidgets) {
		p->updateTrain(train, std::move(adps));
	}
}

void MainWindow::repaintTrainLines(std::shared_ptr<Train> train)
{
	for (auto p : diagramWidgets) {
		p->repaintTrain(train);
	}
}

void MainWindow::repaintTrainLinkLine(std::shared_ptr<Train> train)
{
	for (auto p : diagramWidgets) {
		p->repaintTrainLinkLine(train);
	}
}

int MainWindow::railStationWidgetIndex(std::shared_ptr<Railway> rail) const
{
	for (int i = 0; i < railStationWidgets.size(); i++) {
		if (railStationWidgets.at(i)->getRailway() == rail) {
			return i;
		}
	}
	return -1;
}

void MainWindow::actOpenGraph()
{
	using namespace std::chrono_literals;
	if (changed && !saveQuestion())
		return;
	QString res = QFileDialog::getOpenFileName(this, QObject::tr("打开"), QString(),
		QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
	if (res.isNull())
		return;
	
	bool flag = openGraph(res);
	if (!flag)
		QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
	else {
		markUnchanged();
	}
}


void MainWindow::openFileChecked(const QString& filename)
{
	if (!changed || saveQuestion()) {
		bool flag = openGraph(filename);
		if (!flag)
			QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
		else {
			markUnchanged();
			resetRecentActions();
			updateWindowTitle();
		}
	}
}

void MainWindow::actSaveGraph()
{
	if (_diagram.filename().isEmpty())
		actSaveGraphAs();
	else {
		using namespace std::chrono_literals;
		auto start = std::chrono::system_clock::now();
		_diagram.save();
		markUnchanged();
		undoStack->setClean();
		auto end = std::chrono::system_clock::now();
		showStatus(tr("保存成功  用时%1毫秒").arg((end - start) / 1ms));
	}
}

void MainWindow::actSaveGraphAs()
{
	QString res = QFileDialog::getSaveFileName(this, QObject::tr("另存为"), "",
		tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
	if (res.isNull())
		return;
	auto start = std::chrono::system_clock::now();
	bool flag = _diagram.saveAs(res);
	if (flag) {
		using namespace std::chrono_literals;
		markUnchanged();
		undoStack->setClean();
		addRecentFile(res);
		updateWindowTitle();
		auto end = std::chrono::system_clock::now();
		showStatus(tr("保存成功  用时%1毫秒").arg((end - start) / 1ms));
	}
}

void MainWindow::actPopupAppButton()
{
	auto* btn = ribbonBar()->applicationButton();
	appMenu->popup(btn->mapToGlobal(QPoint(0, btn->height())));
}

void MainWindow::addRecentFile(const QString& filename)
{
	SystemJson::instance.addHistoryFile(filename);
}

void MainWindow::resetRecentActions()
{
	for (auto a : actRecent) {
		appMenu->removeAction(a);
	}
	actRecent.clear();

	int i = 0;
	for (const auto& p : SystemJson::instance.history) {
		auto* act = new QAction(tr("%1. %2").arg(++i).arg(p), appMenu);
		appMenu->addAction(act);
		actRecent.append(act);
		act->setData(p);
		connect(act, SIGNAL(triggered()), this, SLOT(openRecentFile()));
	}

}

void MainWindow::openRecentFile()
{

	QAction* act = qobject_cast<QAction*>(sender());
	if (act) {
		const QString& filename = act->data().toString();
		openFileChecked(filename);
	}
}


void MainWindow::addPageWidget(std::shared_ptr<DiagramPage> page)
{
	int idx = diagramDocks.size();
	insertPageWidget(page, idx);
}

void MainWindow::insertPageWidget(std::shared_ptr<DiagramPage> page, int index)
{
	using namespace std::chrono_literals;
	auto start = std::chrono::system_clock::now();
	DiagramWidget* dw = new DiagramWidget(_diagram, page);
	auto* dock = new ads::CDockWidget(page->name());
	dock->setIcon(QEICN_diagram_page_title);
	dock->setWidget(dw);

	if (SystemJson::instance.use_central_widget) {
		manager->addDockWidget(ads::CenterDockWidgetArea, dock, centralArea);
	}
	else {
		manager->addDockWidget(ads::RightDockWidgetArea, dock);
	}

	QAction* act = dock->toggleViewAction();
	pageMenu->addAction(act);
	diagramDocks.insert(index, dock);
	diagramWidgets.insert(index, dw);
	dw->setupMenu(diaActions);
	//informPageListChanged();
	connect(dw, &DiagramWidget::trainSelected, this, &MainWindow::focusInTrain);
	connect(dw, &DiagramWidget::railFocussedIn, this, &MainWindow::focusInRailway);
	connect(contextTrain, &TrainContext::highlightTrainLine, dw, &DiagramWidget::highlightTrain);
	connect(dw, &DiagramWidget::pageFocussedIn, this, &MainWindow::focusInPage);
	connect(dw, &DiagramWidget::showNewStatus, this, &MainWindow::showStatus);
	connect(dw, &DiagramWidget::timeDraggedSingle, contextTrain, &TrainContext::actDragTimeSingle);
	connect(dw, &DiagramWidget::timeDraggedNonLocal, contextTrain, &TrainContext::actDragTimeNonLocal);
	connect(dw, &DiagramWidget::paintingPointClicked, this, &MainWindow::paintingPointClicked);   // forward
	auto end = std::chrono::system_clock::now();
	showStatus(tr("添加运行图 [%1] 用时%2毫秒").arg(page->name()).arg((end - start) / 1ms));
}

void MainWindow::activatePageWidget(int index)
{
	auto* d = diagramDocks.at(index);
	if (d->isClosed()) {
		d->toggleView(true);
	}
	else {
		d->setAsCurrentTab();
	}
}

//void MainWindow::actAddPage(std::shared_ptr<DiagramPage> page)
//{
//    undoStack->push(new qecmd::AddPage(_diagram, page, this));
//}

void MainWindow::removePageAt(int index)
{
	auto dw = diagramWidgets.takeAt(index);
	auto dock = diagramDocks.takeAt(index);
	//借用dw来索引到被删除的Page 
	if (contextPage->getPage() == dw->page()) {
		contextPage->resetPage();
		focusOutPage();
	}
	manager->removeDockWidget(dock);
	delete dock;
}


void MainWindow::markChanged()
{
	if (!changed) {
		changed = true;
		updateWindowTitle();
	}
}

void MainWindow::markUnchanged()
{
	if (changed) {
		changed = false;
		updateWindowTitle();
	}
}

void MainWindow::focusInPage(std::shared_ptr<DiagramPage> page)
{
	contextPage->setPage(page);
	ribbonBar()->showContextCategory(contextPage->context());
}

void MainWindow::focusOutPage()
{
	ribbonBar()->hideContextCategory(contextPage->context());
}

void MainWindow::focusInTrain(std::shared_ptr<Train> train)
{
	// 2024.03.27: reject trains that on painting to avoid some risks
	if (train->isOnPainting()) {
		qInfo() << "Focusing on train " << train->trainName().full() << " which is now paiting is rejected";
		return;
	}
	ribbonBar()->showContextCategory(contextTrain->context());
	contextTrain->setTrain(train);
	if (!timetableQuickDock->isClosed()) {
		timetableQuickWidget->setTrain(train);
	}
	if (!trainInfoDock->isClosed()) {
		trainInfoWidget->setTrain(train);
	}
}

void MainWindow::focusOutTrain()
{
	ribbonBar()->hideContextCategory(contextTrain->context());
}

void MainWindow::focusInRailway(std::shared_ptr<Railway> rail)
{
	ribbonBar()->showContextCategory(contextRail->context());
	contextRail->setRailway(rail);
}

void MainWindow::focusOutRailway()
{
	ribbonBar()->hideContextCategory(contextRail->context());
}

void MainWindow::focusInRuler(std::shared_ptr<Ruler> ruler)
{
	contextRuler->setRuler(ruler);
	ribbonBar()->showContextCategory(contextRuler->context());
	showStatus(tr("切换到标尺 [%1]  可至工具栏标尺上下文菜单进一步操作").arg(ruler->name()));
}

void MainWindow::focusOutRuler()
{
	ribbonBar()->hideContextCategory(contextRuler->context());
}

void MainWindow::focusInRouting(std::shared_ptr<Routing> routing)
{
	contextRouting->setRouting(routing);
	ribbonBar()->showContextCategory(contextRouting->context());
	showStatus(tr("切换到交路 [%1]  可至工具栏交路上下文菜单进一步操作").arg(routing->name()));
}

void MainWindow::focusOutRouting()
{
	ribbonBar()->hideContextCategory(contextRouting->context());
}

void MainWindow::focusInPath(TrainPath* path)
{
	contextPath->setPath(path);
	ribbonBar()->showContextCategory(contextPath->context());
	showStatus(tr("切换到列车径路 [%2]  可至工具栏列车径路上下文菜单进一步操作").arg(path->name()));
}

void MainWindow::focusOutPath()
{
	ribbonBar()->hideContextCategory(contextPath->context());
}

void MainWindow::repaintRoutingTrainLines(std::shared_ptr<Routing> routing)
{
	for (const auto& p : routing->order()) {
		if (!p.isVirtual()) {
			auto t = p.train();
			repaintTrainLines(t);
			// 2024.03.23: also update corresponding widgets
			if (t == contextTrain->getTrain()) {
				contextTrain->refreshCurrentTrainWidgets();
			}
		}
	}
}
void MainWindow::repaintTrainLines(const QSet<std::shared_ptr<Train>> trains)
{
	foreach(auto p, trains) {
		repaintTrainLines(p);
	}
}

void MainWindow::actSearchTrain()
{
	auto train = SelectTrainCombo::dialogGetTrain(_diagram.trainCollection(), this, tr("搜索车次"),
		tr("请先输入搜索内容，然后按Tab键选择车次"));
	if (train) {
		focusInTrain(train);
        emit contextTrain->highlightTrainLine(train);
	}
}


void MainWindow::actEditFilters()
{
	if (!filterManager) {
		filterManager = new PredefTrainFilterManager(_diagram.trainCollection());
		filterManager->setWindowFlag(Qt::Dialog);
		filterManager->resize(800, 700);
		filterManager->refreshData();

		connect(filterManager, &PredefTrainFilterManager::addFilter,
			catView, &ViewCategory::actAddFilter);
		connect(filterManager, &PredefTrainFilterManager::removeFilter,
			catView, &ViewCategory::actRemoveFilter);
		connect(filterManager, &PredefTrainFilterManager::updateFilter,
			catView, &ViewCategory::actUpdateFilter, Qt::UniqueConnection);
	}
	filterManager->show();
}

void MainWindow::saveDefaultConfig()
{
	_diagram.saveDefaultConfigs();
}

void MainWindow::locateDiagramOnMile(int pageIndex, std::shared_ptr<const Railway> railway, double mile, const QTime& time)
{
	if (pageIndex < 0) {
		QMessageBox::warning(this, tr("错误"), tr("线路[%1]没有包含在任何运行图中，"
			"无法进行定位。").arg(railway->name()));
		return;
	}
	activatePageWidget(pageIndex);
	auto* w = diagramWidgets.at(pageIndex);
	w->locateToMile(railway, mile, time);
}

void MainWindow::locateDiagramOnStation(int pageIndex, std::shared_ptr<const Railway> railway,
	std::shared_ptr<const RailStation> station, const QTime& time)
{
	if (pageIndex < 0) {
		QMessageBox::warning(this, tr("错误"), tr("线路[%1]没有包含在任何运行图中，"
			"无法进行定位。").arg(railway->name()));
		return;
	}
	activatePageWidget(pageIndex);
	auto* w = diagramWidgets.at(pageIndex);
	w->locateToStation(railway, station, time);
}

void MainWindow::showStatus(const QString& msg)
{
	statusBar()->showMessage(tr("%1 | %2").arg(QTime::currentTime().toString("hh:mm:ss"), msg));
}


#endif
