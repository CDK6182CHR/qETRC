#include "mainwindow.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QString>
#include <QUndoView>
#include <QApplication>
#include <QFileDialog>
#include <QSpinBox>
#include <QStatusBar>
#include <QLabel>
#include <chrono>

#include "model/train/trainlistmodel.h"
#include "editors/trainlistwidget.h"
#include "model/diagram/diagramnavimodel.h"
#include "data/common/qesystem.h"
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

#include "version.h"

#include "SARibbonBar.h"
#include "SARibbonCategory.h"
#include "SARibbonMenu.h"
#include "SARibbonPannel.h"
#include "SARibbonQuickAccessBar.h"
#include <SARibbonToolButton.h>

#include "DockAreaWidget.h"
#include <DockManager.h>

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


MainWindow::MainWindow(QWidget* parent)
    : SARibbonMainWindow(parent),
    undoStack(new QUndoStack(this)),
    naviModel(new DiagramNaviModel(_diagram, this))
{
    auto start = std::chrono::system_clock::now();
    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    manager = new ads::CDockManager(this);
    _diagram.readDefaultConfigs();
    undoStack->setUndoLimit(100);
    connect(undoStack, SIGNAL(indexChanged(int)), this, SLOT(markChanged()));

    initUI();

    QString cmdFile;
    if (QCoreApplication::arguments().size() > 1) {
        cmdFile = QCoreApplication::arguments().at(1);
    }

    loadInitDiagram(cmdFile);
    updateWindowTitle();

    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    auto secs = static_cast<double>(duration.count()) * std::chrono::microseconds::period::num /
        std::chrono::microseconds::period::den;
    qDebug() << "MainWindow init consumes: " << secs << Qt::endl;
    showStatus(tr("主窗口初始化用时: %1 秒").arg(secs));
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
    initDockWidgets();
    initToolbar();
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

void MainWindow::onStationTableChanged(std::shared_ptr<Railway> rail, bool equiv)
{
    Q_UNUSED(equiv);
    _diagram.updateRailway(rail);
    trainListWidget->getModel()->updateAllMileSpeed();
    updateRailwayDiagrams(rail);
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
    if(train->empty())
        contextTrain->showBasicWidget(train);
    addTrainLine(*train);
}

void MainWindow::undoAddNewTrain(std::shared_ptr<Train> train)
{
    contextTrain->removeTrainWidget(train);
    removeTrainLine(*train);
    if (train == contextTrain->getTrain())
        focusOutTrain();
}

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
    if (ok && !res.isEmpty()) {
        actOpenRailStationWidget(_diagram.railwayByName(res));
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

void MainWindow::showAboutDialog()
{
    QString text = tr("%1_%2 Release %3\n%4\n")
        .arg(qespec::TITLE.data())
        .arg(qespec::VERSION.data())
        .arg(qespec::RELEASE.data())
        .arg(qespec::DATE.data());
    text += tr("六方车迷会谈  萧迩珀  保留一切权利\nmxy0268@qq.com\n");
    text += tr("QQ群：865211882\nhttps://github.com/CDK6182CHR/qETRC\n"
        "https://gitee.com/xep0268/qETRC");
    QMessageBox::about(this, tr("关于"), text);
}

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

void MainWindow::actRulerPaint()
{
    auto* wzd = new RulerPaintWizard(_diagram, this);
    connect(wzd, &RulerPaintWizard::removeTmpTrainLine,
        this, &MainWindow::removeTrainLine);
    connect(wzd, &RulerPaintWizard::paintTmpTrainLine,
        this, &MainWindow::addTrainLine);
    connect(wzd, &RulerPaintWizard::trainAdded,
        naviView, &NaviTree::actAddPaintedTrain);
    connect(wzd, &RulerPaintWizard::trainTimetableModified,
        contextTrain, &TrainContext::onTrainTimetableChanged);
    connect(wzd, &RulerPaintWizard::trainInfoModified,
        contextTrain, &TrainContext::onTrainInfoChanged);
    wzd->show();
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
    if (SystemJson::instance.use_central_widget || true) {
        dock = new ads::CDockWidget(tr("运行图窗口"));
        dock->setFeature(ads::CDockWidget::NoTab, true);
        auto* c = manager->setCentralWidget(dock);
        centralArea = c;
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

        connect(naviModel, &DiagramNaviModel::railwayRemoved,
            this, &MainWindow::onActRailwayRemoved);

        connect(naviView, &NaviTree::focusInRouting,
            this, &MainWindow::focusInRouting);

        connect(naviView, &NaviTree::actBatchDetectRouting,
            this, &MainWindow::actBatchDetectRouting);
        connect(naviView, &NaviTree::actBatchParseRouting,
            this, &MainWindow::actBatchParseRouting);
    }
    auto* area = manager->addDockWidget(ads::LeftDockWidgetArea, dock);

    // 历史记录
    if constexpr (true) {
        QUndoView* view = new QUndoView(undoStack);
        undoView = view;
        auto* dock = new ads::CDockWidget(tr("历史记录"));
        dock->setWidget(view);
        undoDock = dock;
        manager->addDockWidget(ads::BottomDockWidgetArea, dock, area);
    }

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

    
    

    // 速览信息
    if constexpr (true) {
        auto* w = new TrainInfoWidget();
        dock = new ads::CDockWidget(tr("速览信息"));
        dock->setWidget(w);
        trainInfoWidget = w;
        trainInfoDock = dock;
        connect(w, &TrainInfoWidget::showTimetable,
            this, &MainWindow::showQuickTimetable);
    }
    area = manager->addDockWidget(ads::RightDockWidgetArea, dock);

    // 速览时刻
    if constexpr (true) {
        auto* w = new TimetableQuickWidget(undoStack);
        dock = new ads::CDockWidget(tr("速览时刻"));
        dock->setWidget(w);
        timetableQuickWidget = w;
        timetableQuickDock = dock;
        manager->addDockWidgetTabToArea(dock, area);
    }
}

void MainWindow::initToolbar()
{
    SARibbonBar* ribbon = ribbonBar();
    ribbon->applicationButton()->setText(QStringLiteral("文件"));
    ribbon->showMinimumModeButton();

    //顶上的工具条
    if constexpr (true) {
        //撤销重做
        QAction* act = undoStack->createUndoAction(this, tr("撤销"));
        act->setIcon(QIcon(":/icons/undo.png"));
        act->setShortcut(Qt::CTRL + Qt::Key_Z);
        act->setShortcutContext(Qt::WindowShortcut);
        ribbon->quickAccessBar()->addAction(act);

        act = undoStack->createRedoAction(this, tr("重做"));
        act->setIcon(QIcon(":/icons/redo.png"));
        act->setShortcut(Qt::CTRL + Qt::Key_Y);
        ribbon->quickAccessBar()->addAction(act);

        auto* menu = new QMenu();
        act = menu->addAction(tr("使用Office风格Ribbon"));
        connect(act, SIGNAL(triggered()), this, SLOT(useOfficeStyle()));
        act = menu->addAction(tr("使用WPS风格Ribbon"));
        connect(act, SIGNAL(triggered()), this, SLOT(useWpsStyle()));
        ribbon->quickAccessBar()->addMenu(menu);

    }

    QAction* actTrainList, * qactChangeStationName;
    //开始
    if constexpr (true) {
        SARibbonCategory* cat = ribbon->addCategoryPage(QObject::tr("开始(&1)"));
        SARibbonPannel* panel = cat->addPannel(QObject::tr("文件"));

        QAction* act = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileIcon),
            QObject::tr("新建"), this);
        panel->addLargeAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_N);
        act->setToolTip(tr("新建 (Ctrl+N)\n关闭当前运行图文件，并新建空白运行图文件。"));
        addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actNewGraph()));
        sharedActions.newfile = act;

        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton),
            QObject::tr("打开"), this);
        addAction(act);
        act->setToolTip(tr("打开 (Ctrl+O)\n关闭当前运行图文件，并打开新的既有运行图文件。"));
        act->setShortcut(Qt::CTRL + Qt::Key_O);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actOpenGraph()));
        sharedActions.open = act;

        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton),
            QObject::tr("保存"), this);
        act->setToolTip(tr("保存 (Ctrl+S)\n保存当前运行图。如果是新运行图，则需要先选择文件。"));
        act->setShortcut(Qt::CTRL + Qt::Key_S);
        addAction(act);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actSaveGraph()));
        sharedActions.save = act;

        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogSaveAllButton),
            tr("另存为.."), this);
        act->setToolTip(tr("另存为\n将当前运行图另存为新建文件。"));
        panel->addMediumAction(act);
        connect(act, &QAction::triggered, this, &MainWindow::actSaveGraphAs);
        sharedActions.saveas = act;

        act = new QAction(QIcon(":/icons/ETRC-dynamic.png"), tr("导出为.."), this);
        act->setToolTip(tr("导出为单线路运行图 (Ctrl+M)\n"
            "导出为单一线路的pyETRC或ETRC运行图文件格式。"));
        act->setShortcut(Qt::CTRL + Qt::Key_M);
        connect(act, SIGNAL(triggered()), this, SLOT(actToSingleFile()));
        addAction(act);
        panel->addMediumAction(act);

        panel = cat->addPannel(tr("窗口"));
        act = naviDock->toggleViewAction();
        act->setShortcut(Qt::Key_F3);
        addAction(act);
        act->setText(QObject::tr("导航"));
        act->setToolTip(tr("导航面板 (F3)\n打开或关闭运行图总导航面板。"));
        act->setIcon(QIcon(":/icons/Graph-add.png"));
        auto* btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = trainListDock->toggleViewAction();
        actTrainList = act;
        act->setText(tr("列车管理"));
        act->setToolTip(tr("列车管理\n打开或关闭（pyETRC风格的）列车管理列表面板。"));
        act->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogListView));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        SARibbonMenu* menu = new SARibbonMenu(this);
        pageMenu = menu;
        menu->setToolTip(tr("运行图窗口\n导航、打开或关闭现有运行图窗口。"));
        menu->setTitle(QObject::tr("运行图窗口"));
        menu->setIcon(QIcon(":/icons/diagram.png"));
        btn = panel->addLargeMenu(menu);
        btn->setMinimumWidth(80);

        //undo  试一下把dock也放在这里初始化...

        act = undoDock->toggleViewAction();
        act->setText(tr("历史记录"));
        act->setIcon(QIcon(":/icons/clock.png"));
        act->setToolTip(tr("历史记录\n打开或关闭历史记录面板。\n" 
            "历史记录面板记录了可撤销的针对运行图文档的操作记录，可以查看、撤销、重做。"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/add.png"), tr("添加运行图"), this);
        act->setToolTip(tr("添加运行图\n选择既有基线数据，建立新的运行图页面。"));
        connect(act, SIGNAL(triggered()), naviView, SLOT(addNewPage()));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        panel = cat->addPannel(tr("更新"));
        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
            tr("刷新"), this);
        act->setToolTip(tr("刷新 (F5)\n重新铺画运行图，更新所有数据面板的信息。"));
        act->setShortcut(Qt::Key_F5);
        diaActions.refreshAll = act;
        addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(refreshAll()));
        btn = panel->addLargeAction(act);

        act = new QAction(QIcon(":/icons/brush.png"), tr("更改站名"), this);
        act->setToolTip(tr("全局站名修改 (Ctrl+U)\n"
            "在整个运行图的所有线路、所有车次中，将旧站名改为新站名。"));
        qactChangeStationName = act;
        act->setShortcut(Qt::CTRL + Qt::Key_U);
        addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actChangeStationName()));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        panel = cat->addPannel(tr("系统"));
        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation),
            tr("关于"), this);
        connect(act, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
        btn->setPopupMode(QToolButton::InstantPopup);
        menu = new SARibbonMenu(this);
        menu->addAction(QApplication::style()->standardIcon(QStyle::SP_TitleBarMenuButton),
            tr("关于Qt"), QApplication::aboutQt);
        act->setMenu(menu);
        btn = panel->addLargeAction(act);

        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserStop),
            tr("退出"), this);
        connect(act, SIGNAL(triggered()), this, SLOT(close()));
        panel->addLargeAction(act);

    }

    //线路
    QAction* actQuickPath, * actSelector;
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("线路(&2)"));
        auto* panel = cat->addPannel(tr("基础数据"));

        auto* act = new QAction(QIcon(":/icons/add.png"), tr("导入线路"), this);
        connect(act, SIGNAL(triggered()), naviView, SLOT(importRailways()));
        act->setToolTip(tr("导入线路\n从既有运行图文件中导入（其中全部的）线路数据"));
        auto* btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/rail.png"), tr("基线编辑"), this);
        act->setToolTip(tr("基线编辑\n导航到、新建、打开或者关闭（pyETRC风格的）基线编辑面板。"));
        connect(act, SIGNAL(triggered()), this, SLOT(actOpenNewRailWidget()));
        auto* menu = new SARibbonMenu(this);
        railMenu = menu;
        act->setMenu(menu);
        menu->setTitle(tr("线路编辑"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/ruler.png"), tr("标尺编辑"), this);
        act->setToolTip(tr("标尺编辑 (Ctrl+B)\n"
            "打开全局任意线路中的任意一个标尺的编辑面板。"));
        connect(act, SIGNAL(triggered()), this, SLOT(actNaviToRuler()));
        act->setShortcut(Qt::CTRL + Qt::Key_B);
        addAction(act);
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/forbid.png"), tr("天窗编辑"), this);
        act->setToolTip(tr("天窗编辑\n打开或导航到任意一基线的天窗编辑面板。"));
        connect(act, SIGNAL(triggered()), this, SLOT(actNaviToForbid()));
        forbidMenu = new SARibbonMenu(this);
        act->setMenu(forbidMenu);
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/new-file.png"), tr("新建线路"), this);
        connect(act, SIGNAL(triggered()), naviView, SLOT(actAddRailway()));
        act->setToolTip(tr("新建线路\n新建空白的铁路线路"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        panel = cat->addPannel(tr("调整"));
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/ruler_pen.png"), tr("标尺综合"), this);
        act->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_B);
        addAction(act);
        act->setToolTip(tr("标尺综合 (Ctrl+Shift+B)\n"
            "从一组选定的车次中，读取一套标尺数据，用于建立新标尺，或更新既有标尺。"));
        connect(act, SIGNAL(triggered()), this, SLOT(actReadRulerWizard()));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        panel->addSeparator();
        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView),
            tr("定位"), this);
        act->setToolTip(tr("运行图定位 (Ctrl+G)\n输入时刻，线路及其里程，"
            "快速定位到运行图面板上。"));
        act->setShortcut(Qt::CTRL + Qt::Key_G);
        connect(act, &QAction::triggered, this, &MainWindow::actLocateDiagram);
        addAction(act);
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        panel = cat->addPannel(tr("路网管理"));

        act = new QAction(QIcon(":/icons/database.png"), tr("数据库"), this);
        act->setToolTip(tr("线路数据库 (Ctrl+H)\n"
            "查看、编辑或者导入线路数据库中的基线数据。"));
        connect(act, &QAction::triggered, this, &MainWindow::actRailDBDock);
        act->setShortcut(Qt::CTRL + Qt::Key_H);
        addAction(act);
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        //menu = new SARibbonMenu(this);
        //auto actsub = menu->addAction(tr("线路数据库 (pyETRC风格)"));
        //addAction(actsub);
        ////actsub->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_H);
        //connect(actsub, &QAction::triggered, this, &MainWindow::actRailDB);
        //act->setMenu(menu);

        act = new QAction(QIcon(":/icons/diagram.png"), tr("快速径路"), this);
        act->setToolTip(tr("快速径路生成 (Ctrl+J)\n"
            "通过线路数据库中的数据，给出经由的关键点表，利用最短路算法生成新线路数据。"));
        act->setShortcut(Qt::CTRL + Qt::Key_J);
        addAction(act);
        panel->addMediumAction(act);
        actQuickPath = act;

        act = new QAction(QIcon(":/icons/polyline.png"), tr("经由选择"), this);
        act->setToolTip(tr("交互式经由选择 (Ctrl+K)\n"
            "通过数据库中的数据，手动指定通过邻站、邻线或者区间最短路方式，"
            "生成径路可精确控制的新线路数据。"));
        act->setShortcut(Qt::CTRL + Qt::Key_K);
        addAction(act);
        panel->addMediumAction(act);
        actSelector = act;
    }

    QAction* actRemoveInterp, * actAutoBusiness, * actImportTimetableCsv;
    //列车
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("列车(&3)"));
        auto* panel = cat->addPannel(tr("车次管理"));

        auto* act = actTrainList;
        auto* menu = new QMenu(tr("列车管理扩展"), this);

        auto* actsub = new QAction(tr("删除所有车次"), this);
        connect(actsub, SIGNAL(triggered()), this, SLOT(removeAllTrains()));
        menu->addAction(actsub);

        actsub = new QAction(tr("删除所有车次和交路"), this);
        connect(actsub, &QAction::triggered, this, &MainWindow::removeAllTrainsAndRoutings);
        menu->addAction(actsub);

        menu->addSeparator();

        actsub = new QAction(tr("自动始发终到站适配"), this);
        menu->addAction(actsub);
        connect(actsub, SIGNAL(triggered()), this, SLOT(actAutoStartingTerminal()));

        actsub = menu->addAction(tr("自动推断所有列车类型"));
        connect(actsub, &QAction::triggered, this, &MainWindow::actAutoTrainType);

        actAutoBusiness = menu->addAction(tr("自动设置所有营业站"));

        act->setMenu(menu);
        auto* btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/add_train.png"), tr("导入车次"), this);
        act->setToolTip(tr("导入车次 (Ctrl+D)\n从既有运行图或者车次数据库文件中导入部分或全部的车次。"));
        act->setShortcut(Qt::CTRL + Qt::Key_D);
        addAction(act);
        connect(act, SIGNAL(triggered()), naviView, SLOT(importTrains()));
        
        menu = new SARibbonMenu(this);
        actImportTimetableCsv = menu->addAction(tr("导入时刻表 (CSV)"));
        act->setMenu(menu);
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView),
            tr("搜索车次"), this);
        act->setToolTip(tr("搜索车次 (Ctrl+F)\n搜索车次，将车次设定为当前车次，并高亮运行线。"));
        connect(act, &QAction::triggered, this, &MainWindow::actSearchTrain);
        act->setShortcut(Qt::CTRL + Qt::Key_F);
        addAction(act);
        panel->addMediumAction(act);
        diaActions.search = act;

        act = new QAction(QIcon(":/icons/add.png"), tr("新建车次"), this);
        act->setToolTip(tr("新建车次 (Ctrl+Shift+C)\n新建空白车次"));
        act->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C);
        addAction(act);
        diaActions.addTrain = act;
        btn = panel->addMediumAction(act);
        connect(act, SIGNAL(triggered()), naviView, SLOT(actAddTrain()));

        panel = cat->addPannel(tr("工具"));

        act = timetableQuickDock->toggleViewAction();
        act->setIcon(QIcon(":/icons/clock.png"));
        act->setShortcut(Qt::CTRL + Qt::Key_I);
        addAction(act);
        act->setToolTip(tr("速览时刻 (Ctrl+I)\n"
            "显示或隐藏pyETRC风格的双行只读时刻表。"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(70);

        act = trainInfoDock->toggleViewAction();
        act->setIcon(QIcon(":/icons/info.png"));
        act->setShortcut(Qt::CTRL + Qt::Key_Q);
        addAction(act);
        act->setToolTip(tr("速览信息 (Ctrl+Q)\n"
            "显示或隐藏pyETRC风格的列车信息栏。"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(70);

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

            panel->addWidget(w, SARibbonPannelItem::Large);

            act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton),
                tr("应用"), this);
            connect(act, SIGNAL(triggered()), this, SLOT(actChangePassedStations()));
            btn = panel->addLargeAction(act);
            btn->setMinimumWidth(80);
        }

        panel = cat->addPannel(tr("选项"));
        act = new QAction(QIcon(":/icons/weaken.png"), tr("背景虚化"), this);
        act->setToolTip(tr("背景虚化\n若启用，则在选中列车运行线时，自动虚化其他列车运行线。"
            "点击空白处取消选择则取消虚化。\n"
            "若此功能出现问题，请刷新运行图。"));
        act->setCheckable(true);
        act->setChecked(SystemJson::instance.weaken_unselected);
        connect(act, &QAction::triggered, this, &MainWindow::actToggleWeakenUnselected);
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        panel = cat->addPannel(tr("交路"));
        act = routingDock->toggleViewAction();
        act->setIcon(QIcon(":/icons/polyline.png"));

        routingMenu = new SARibbonMenu(tr("打开的交路编辑"), this);
        act->setMenu(routingMenu);

        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/text.png"), tr("批量解析"), this);
        connect(act, &QAction::triggered, this, &MainWindow::actBatchParseRouting);
        act->setToolTip(tr("批量文本解析\n批量地从车次套用顺序文本中解析出交路数据"));
        panel->addMediumAction(act);

        act = new QAction(QIcon(":/icons/identify.png"), tr("批量识别"), this);
        connect(act, &QAction::triggered, this, &MainWindow::actBatchDetectRouting);
        act->setToolTip(tr("批量识别车次\n对所有交路，尝试识别车次，使得虚拟车次转变为实体车次。"));
        panel->addMediumAction(act);

        panel = cat->addPannel(tr("分析"));
        act = new QAction(QIcon(":/icons/compare.png"), tr("车次对照"), this);
        act->setToolTip(tr("两车次运行对照\n在指定线路上，对比两个选定车次的运行情况。"));
        connect(act, SIGNAL(triggered()), this, SLOT(actTrainDiff()));
        panel->addMediumAction(act);
        diaActions.trainRef = act;

        act = new QAction(QIcon(":/icons/identify.png"), tr("时刻诊断"), this);
        act->setToolTip(tr("车次时刻诊断\n诊断指定车次或所有车次的时刻表是否存在"
            "可能的问题，例如到开时刻填反。"));
        connect(act, &QAction::triggered, this, &MainWindow::actDiagnose);
        panel->addMediumAction(act);

        panel = cat->addPannel(tr("排图"));

        act = new QAction(QIcon(":/icons/ruler_pen.png"), tr("标尺排图"), this);
        addAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_R);
        act->setToolTip(tr("标尺排图向导 (Ctrl+R)\n使用指定线路的指定标尺，"
            "铺画新的列车运行线，或者重新铺画既有列车运行线的一部分。"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);
        connect(act, SIGNAL(triggered()), this, SLOT(actRulerPaint()));
        diaActions.rulerPaint = act;
        
        act = new QAction(QIcon(":/icons/copy.png"), tr("批量复制"), this);
        act->setToolTip(tr("批量复制运行线\n"
            "将指定车次的运行线按照其起始时刻复制若干个（车次不同的）副本。"));
        connect(act, SIGNAL(triggered()), this, SLOT(actBatchCopyTrain()));
        diaActions.batchCopy = act;
        panel->addMediumAction(act);

        act = new QAction(QIcon(":/icons/add.png"), tr("时刻插值"), this);
        act->setToolTip(tr("时刻插值（推定通过站时刻）\n"
            "通过标尺数据，推定列车时刻表中未给出的通过站的时刻。"));
        connect(act, &QAction::triggered, this, &MainWindow::actInterpolation);

        menu = new SARibbonMenu(this);
        actRemoveInterp = menu->addAction(tr("删除所有推定结果"));
        act->setMenu(menu);
        btn = panel->addMediumAction(act);
        btn->setPopupMode(QToolButton::DelayedPopup);
        
    }

    //显示
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("显示(&4)"));
        catView = new ViewCategory(this, cat, this);
        connect(trainListWidget->getModel(), &TrainListModel::trainShowChanged,
            catView, &ViewCategory::actChangeSingleTrainShow);
    }

    //context: page 5
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr(""));
        contextPage = new PageContext(_diagram, cat, this);
        connect(contextPage, &PageContext::pageRemoved,
            naviView, &NaviTree::removePage);
        connect(contextPage, &PageContext::pageNameChanged,
            naviModel, &DiagramNaviModel::onPageNameChanged);
    }

    //context: train 6 7
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr(""));
        contextTrain = new TrainContext(_diagram, cat, this);

        connect(contextTrain, &TrainContext::timetableChanged,
            trainListWidget->getModel(), &TrainListModel::onTrainChanged);
        connect(contextTrain, &TrainContext::actRemoveTrain,
            naviView, &NaviTree::removeSingleTrain);
        connect(naviView, &NaviTree::editTrain,
            contextTrain, &TrainContext::actShowBasicWidget);
        connect(trainListWidget, &TrainListWidget::editTrain,
            contextTrain, &TrainContext::actShowBasicWidget);

        connect(naviModel, &DiagramNaviModel::newTrainAdded,
            this, &MainWindow::onNewTrainAdded);
        connect(naviModel, &DiagramNaviModel::undoneAddTrain,
            this, &MainWindow::undoAddNewTrain);
        connect(contextTrain, &TrainContext::focusInRouting,
            this, &MainWindow::focusInRouting);
        connect(trainInfoWidget, &TrainInfoWidget::editTrain,
            contextTrain, &TrainContext::showBasicWidget);

        connect(timetableQuickWidget->getModel(),
            &TimetableQuickEditableModel::trainStationTimeUpdated,
            contextTrain, &TrainContext::onTrainStationTimeChanged);

        connect(actRemoveInterp, &QAction::triggered,
            contextTrain, &TrainContext::actRemoveInterpolation);
        connect(timetableQuickWidget, &TimetableQuickWidget::locateToBoundStation,
            contextTrain, &TrainContext::locateToBoundStation);
        connect(actAutoBusiness, &QAction::triggered,
            contextTrain, &TrainContext::actAutoBusiness);

        connect(actImportTimetableCsv, &QAction::triggered,
            contextTrain, &TrainContext::actImportTrainFromCsv);
    }

    //context: rail 8
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr(""));
        contextRail = new RailContext(_diagram, cat, this, this);
        connect(contextRail, &RailContext::railNameChanged,
            naviModel, &DiagramNaviModel::onRailNameChanged);
        connect(contextRail, &RailContext::stationTableChanged,
            this, &MainWindow::onStationTableChanged);
        connect(contextRail, &RailContext::selectRuler,
            this, &MainWindow::focusInRuler);
        connect(contextRail, &RailContext::rulerInsertedAt,
            naviModel, &DiagramNaviModel::insertRulerAt);
        connect(contextRail, &RailContext::rulerRemovedAt,
            naviModel, &DiagramNaviModel::removeRulerAt);
        connect(naviView, &NaviTree::editForbid,
            contextRail, &RailContext::openForbidWidgetTab);
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

    //ApplicationMenu的初始化放在最后，因为可能用到前面的..
    initAppMenu();
    connect(ribbon->applicationButton(), SIGNAL(clicked()), this,
        SLOT(actPopupAppButton()));

    ribbon->setRibbonStyle(
        static_cast<SARibbonBar::RibbonStyle>(SystemJson::instance.ribbon_style));
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

    undoStack->clear();
    undoStack->resetClean();

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
    catView->refreshTypeGroup();

    spPassedStations->setValue(_diagram.config().max_passed_stations);

    updateWindowTitle();
}

void MainWindow::resetDiagramPages()
{
    if (_diagram.pages().empty()&&!_diagram.isNull())
        _diagram.createDefaultPage();
    for (auto p : _diagram.pages()) {
        addPageWidget(p);
    }
}

bool MainWindow::openGraph(const QString& filename)
{
    clearDiagramUnchecked();

    Diagram dia;
    dia.readDefaultConfigs();   //暂定这里读取一次默认配置，防止move时丢失数据
    bool flag = dia.fromJson(filename);

    if (flag && !dia.isNull()) {
        beforeResetGraph();
        _diagram = std::move(dia);   //move assign
        endResetGraph();
        addRecentFile(filename);
        return true;
    }
    else {
        qDebug() << "MainWindow::openGraph: WARNING: open failed: " << filename;
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

void MainWindow::actAutoStartingTerminal()
{
    QString text = tr("遍历所有车次，当车次满足以下条件时，将该车次的始发站调整为在本线的第一个车站，终到站调整为" \
        "在本线的最后一个车站。\n" 
        "（1）该车次在时刻表中的第一个（最后一个）站铺画在运行图上；\n" 
        "（2）该车次时刻表中第一个（最后一个）站站名以始发（终到）站站名开头，" 
        "“场”字结尾；\n"
        "（3）始发（终到）站及时刻表中第一个（最后一个）站站名不含域解析符（::）。\n"
        "是否继续？");
    auto flag = QMessageBox::question(this, tr("自动始发终到站适配"), text);
    if (flag != QMessageBox::Yes)
        return;
    qecmd::StartingTerminalData data;
    for (auto train : _diagram.trains()) {
        if (train->empty())
            continue;
        auto* first = train->boundFirst();
        if (first) {
            const auto& firstname = first->trainStation->name;
            const auto& starting = train->starting();
            if (firstname.isSingleName() && starting.isSingleName() &&
                firstname.station().startsWith(starting.station()) &&
                firstname.station().endsWith(tr("场"))) {
                data.startings.emplace_back(std::make_pair(train, firstname));
            }
        }
        auto* last = train->boundLast();
        if (last) {
            const auto& lastname = last->trainStation->name;
            const auto& terminal = train->terminal();
            if(lastname.isSingleName()&&terminal.isSingleName() && 
                lastname.station().startsWith(terminal.station()) &&
                lastname.station().endsWith(tr("场"))) {
                data.terminals.emplace_back(std::make_pair(train, lastname));
            }
        }
    }
    if (data.startings.empty() && data.terminals.empty()) {
        QMessageBox::information(this, tr("提示"), tr("没有车次受到影响。"));
    }
    else {
        QMessageBox::information(this, tr("提示"), tr("设置成功。影响%1个车次的始发站和"
            "%2个车次的终到站。").arg(data.startings.size()).arg(data.terminals.size()));
        undoStack->push(new qecmd::AutoStartingTerminal(std::move(data), contextTrain));
    }
}

void MainWindow::actTrainDiff()
{
    auto* dialog = new TrainDiffDialog(_diagram, this);
    dialog->open();
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
    auto flag = QMessageBox::question(this, tr("提示"), tr("此操作根据当前的列车类型管理器，"
        "对所有列车，根据其全车次，使用正则表达式匹配到列车类型。\n如果以前有手动设置的列车"
        "类型，数据将被覆盖。\n是否确认？"));
    if (flag != QMessageBox::Yes) return;

    qecmd::AutoTrainType::data_t data;

    auto& coll = _diagram.trainCollection();
    foreach(auto train, coll.trains()) {
        auto type = coll.typeManager().fromRegex(train->trainName());
        if (type != train->type()) {
            data.emplace_back(train, type);
        }
    }
    if (data.empty()) {
        QMessageBox::information(this, tr("提示"), tr("自动类型推断应用完成，"
            "没有产生任何变化。"));
    }
    else {
        int sz = data.size();
        undoStack->push(new qecmd::AutoTrainType(std::move(data), contextTrain));
        QMessageBox::information(this, tr("提示"), tr("自动类型推断应用完成，"
            "%1个车次的类型发生变化。如有问题，可以撤销。").arg(sz));
    }
}

void MainWindow::actLocateDiagram()
{
    if (!locateDialog) {
        locateDialog = new LocateDialog(_diagram, this);
        connect(locateDialog, &LocateDialog::locateOnMile,
            this, &MainWindow::locateDiagramOnMile);
        connect(locateDialog, &LocateDialog::locateOnStation,
            this, &MainWindow::locateDiagramOnStation);
    }
    locateDialog->showDialog();
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
    _diagram.rebindAllTrains();
    updateAllDiagrams();
    //直接由Main管理的子页面
    naviModel->resetModel();
    trainListWidget->refreshData();
    for (auto p : railStationWidgets) {
        p->refreshData();
    }
    catView->refreshTypeGroup();
    //更新各个context，由context负责更新自己管理的widget的数据
    contextTrain->refreshAllData();
    contextRail->refreshAllData();
    contextRuler->refreshAllData();
    contextPage->refreshAllData();
    updateWindowTitle();
    auto end = std::chrono::system_clock::now();
    showStatus(tr("全部刷新完毕  用时 %1 毫秒").arg((end - start) / std::chrono::milliseconds(1)));
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

void MainWindow::actOpenGraph()
{
    using namespace std::chrono_literals;
    if (changed && !saveQuestion())
        return;
    QString res = QFileDialog::getOpenFileName(this, QObject::tr("打开"), QString(),
        QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    auto start = std::chrono::system_clock::now();
    bool flag = openGraph(res);
    if (!flag)
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
    else {
        markUnchanged();
        auto end = std::chrono::system_clock::now();
        showStatus(tr("打开运行图文件%1成功  用时%2毫秒").arg(res)
            .arg((end - start) / 1ms));
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
        showStatus(tr("保存成功  用时%1毫秒").arg((end-start)/1ms));
    }
}

void MainWindow::actSaveGraphAs()
{
    QString res = QFileDialog::getSaveFileName(this, QObject::tr("另存为"), "",
        tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    bool flag = _diagram.saveAs(res);
    if (flag) {
        using namespace std::chrono_literals;
        auto start = std::chrono::system_clock::now();
        markUnchanged();
        undoStack->setClean();
        addRecentFile(res);
        updateWindowTitle();
        auto end = std::chrono::system_clock::now();
        showStatus(tr("保存成功  用时%1毫秒").arg((end-start)/1ms));
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
        auto* act = new QAction(tr("%1. %2").arg(++i).arg(p), this);
        appMenu->addAction(act);
        actRecent.append(act);
        act->setData(p);
        connect(act, SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

}

void MainWindow::openRecentFile()
{
    using namespace std::chrono_literals;
    QAction* act = qobject_cast<QAction*>(sender());
    if (act) {
        const QString& filename = act->data().toString();
        if (!changed || saveQuestion()) {
            auto start = std::chrono::system_clock::now();
            bool flag = openGraph(filename);
            if (!flag)
                QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
            else {
                markUnchanged();
                resetRecentActions();
                updateWindowTitle();
                auto end = std::chrono::system_clock::now();
                showStatus(tr("打开运行图文件%1成功  用时%2毫秒").arg(filename)
                    .arg((end - start) / 1ms));
            }
        }
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
    auto* dock = new ads::CDockWidget(tr("运行图 - %1").arg(page->name()));
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
    ribbonBar()->showContextCategory(contextTrain->context());
    contextTrain->setTrain(train);
    if (!timetableQuickDock->isClosed() ) {
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
}

void MainWindow::focusOutRuler()
{
    ribbonBar()->hideContextCategory(contextRuler->context());
}

void MainWindow::focusInRouting(std::shared_ptr<Routing> routing)
{
    contextRouting->setRouting(routing);
    ribbonBar()->showContextCategory(contextRouting->context());
}

void MainWindow::focusOutRouting()
{
    ribbonBar()->hideContextCategory(contextRouting->context());
}

void MainWindow::repaintRoutingTrainLines(std::shared_ptr<Routing> routing)
{
    for (const auto& p : routing->order()) {
        if (!p.isVirtual()) {
            repaintTrainLines(p.train());
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
        contextTrain->highlightTrainLine(train);
    }
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


