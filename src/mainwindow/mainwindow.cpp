#include "mainwindow.h"

#include <QString>


#include <QTableView>
#include <QDockWidget>
#include <QtWidgets>
#include "model/train/trainlistmodel.h"
#include "editors/trainlistwidget.h"
#include "model/diagram/diagramnavimodel.h"
#include "data/diagram/diagram.h"
#include "navi/navitree.h"
#include "navi/addpagedialog.h"
#include "editors/configdialog.h"
#include "wizards/rulerpaint/rulerpaintwizard.h"

#include "version.h"

#include "SARibbonBar.h"
#include "SARibbonCategory.h"
#include "SARibbonMenu.h"
#include "SARibbonPannel.h"
#include "SARibbonQuickAccessBar.h"
#include <SARibbonToolButton.h>

#include "DockAreaWidget.h"

//以下引入的是临时测试的
#include "editors/railstationwidget.h"


MainWindow::MainWindow(QWidget* parent)
    : SARibbonMainWindow(parent),
    manager(new ads::CDockManager(this)),
    naviModel(new DiagramNaviModel(_diagram, this)),
    undoStack(new QUndoStack(this))
{
    _diagram.readDefaultConfigs();
    undoStack->setUndoLimit(100);
    connect(undoStack, SIGNAL(indexChanged(int)), this, SLOT(markChanged()));

    initUI();
    loadInitDiagram();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{

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
    undoStack->clear();  //不支持撤销
    markChanged();
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
    focusOutTrain();
}

void MainWindow::undoAddNewRailway(std::shared_ptr<Railway> rail)
{
    removeRailStationWidget(rail);
}

void MainWindow::onNewTrainAdded(std::shared_ptr<Train> train)
{
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
    text += tr("QQ群：865211882\nhttps://github.com/CDK6182CHR/qETRC");
    QMessageBox::about(this, tr("关于"), text);
}

void MainWindow::useWpsStyle()
{
    ribbonBar()->setRibbonStyle(SARibbonBar::WpsLiteStyle);
}

void MainWindow::useOfficeStyle()
{
    ribbonBar()->setRibbonStyle(SARibbonBar::OfficeStyle);
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

void MainWindow::initUI()
{
    initDockWidgets();
    initToolbar();
}

void MainWindow::initDockWidgets()
{
    ads::CDockWidget* dock;

    //总导航
    if constexpr (true) {
        auto* tree = new NaviTree(naviModel, undoStack);
        naviView = tree;
        dock = new ads::CDockWidget(QObject::tr("运行图资源管理器"));
        naviDock = dock;
        dock->setWidget(tree);
        manager->addDockWidget(ads::LeftDockWidgetArea, dock);

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
    }

    //列车管理
    if constexpr (true) {
        auto* tw = new TrainListWidget(_diagram.trainCollection(), undoStack);
        dock = new ads::CDockWidget(tr("列车管理"));
        trainListWidget = tw;
        trainListDock = dock;
        dock->setWidget(tw);
        manager->addDockWidget(ads::LeftDockWidgetArea, dock);
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

}

void MainWindow::initToolbar()
{
    SARibbonBar* ribbon = ribbonBar();
    ribbon->applicationButton()->setText(QStringLiteral("文件"));


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

    QAction* actTrainList;
    //开始
    if constexpr (true) {
        SARibbonCategory* cat = ribbon->addCategoryPage(QObject::tr("开始"));
        SARibbonPannel* panel = cat->addPannel(QObject::tr("文件"));

        QAction* act = new QAction(QIcon(":/icons/new-file.png"), QObject::tr("新建"), this);
        panel->addLargeAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_N);
        act->setToolTip(tr("新建 (Ctrl+N)\n关闭当前运行图文件，并新建空白运行图文件。"));
        addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actNewGraph()));
        sharedActions.newfile = act;

        act = new QAction(QIcon(":/icons/open.png"), QObject::tr("打开"), this);
        addAction(act);
        act->setToolTip(tr("打开 (Ctrl+O)\n关闭当前运行图文件，并打开新的既有运行图文件。"));
        act->setShortcut(Qt::CTRL + Qt::Key_O);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actOpenGraph()));
        sharedActions.open = act;

        act = new QAction(QIcon(":/icons/save1.png"), QObject::tr("保存"), this);
        act->setToolTip(tr("保存 (Ctrl+S)\n保存当前运行图。如果是新运行图，则需要先选择文件。"));
        act->setShortcut(Qt::CTRL + Qt::Key_S);
        addAction(act);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actSaveGraph()));
        sharedActions.save = act;

        panel = cat->addPannel(tr("窗口"));
        act = naviDock->toggleViewAction();
        act->setText(QObject::tr("导航"));
        act->setToolTip(tr("导航面板\n打开或关闭运行图总导航面板。"));
        act->setIcon(QIcon(":/icons/Graph-add.png"));
        auto* btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = trainListDock->toggleViewAction();
        actTrainList = act;
        act->setText(tr("列车管理"));
        act->setToolTip(tr("列车管理\n打开或关闭（pyETRC风格的）列车管理列表面板。"));
        act->setIcon(QIcon(":/icons/list.png"));
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
        QUndoView* view = new QUndoView(undoStack);
        auto* dock = new ads::CDockWidget(tr("历史记录"));
        dock->setWidget(view);
        manager->addDockWidgetTab(ads::CenterDockWidgetArea, dock);

        act = dock->toggleViewAction();
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

        panel = cat->addPannel(tr("系统"));
        act = new QAction(QIcon(":/icons/info.png"), tr("关于"), this);
        connect(act, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
        panel->addLargeAction(act);

        act = new QAction(QIcon(":/icons/close.png"), tr("退出"), this);
        connect(act, SIGNAL(triggered()), this, SLOT(close()));
        panel->addLargeAction(act);

    }

    //线路
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("线路"));
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

        act = new QAction(QIcon(":/icons/new-file.png"), tr("新建线路"), this);
        connect(act, SIGNAL(triggered()), naviView, SLOT(actAddRailway()));
        act->setToolTip(tr("新建线路\n新建空白的铁路线路"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);
    }

    //列车
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("列车"));
        auto* panel = cat->addPannel(tr("车次管理"));

        auto* act = actTrainList;
        auto* menu = new QMenu(tr("列车管理扩展"), this);

        auto* actsub = new QAction(tr("删除所有车次"), this);
        connect(actsub, SIGNAL(triggered()), this, SLOT(removeAllTrains()));
        menu->addAction(actsub);

        act->setMenu(menu);
        auto* btn = panel->addLargeAction(act);
        btn->setMinimumWidth(70);

        act = new QAction(QIcon(":/icons/add_train.png"), tr("导入车次"), this);
        act->setToolTip(tr("导入车次 (Ctrl+D)\n从既有运行图或者车次数据库文件中导入部分或全部的车次。"));
        act->setShortcut(Qt::CTRL + Qt::Key_D);
        addAction(act);
        connect(act, SIGNAL(triggered()), naviView, SLOT(importTrains()));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(70);

        act = new QAction(QIcon(":/icons/add.png"), tr("新建车次"), this);
        act->setToolTip(tr("新建车次\n新建空白车次"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(70);
        connect(act, SIGNAL(triggered()), naviView, SLOT(actAddTrain()));

        act = new QAction(QIcon(":/icons/ruler_pen.png"), tr("标尺排图"), this);
        addAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_R);
        act->setToolTip(tr("标尺排图向导 (Ctrl+R)\n使用指定线路的指定标尺，"
            "铺画新的列车运行线，或者重新铺画既有列车运行线的一部分。"));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);
        connect(act, SIGNAL(triggered()), this, SLOT(actRulerPaint()));

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

            act = new QAction(QIcon(":/icons/tick.png"), tr("应用"), this);
            connect(act, SIGNAL(triggered()), this, SLOT(actChangePassedStations()));
            btn = panel->addLargeAction(act);
            btn->setMinimumWidth(80);
        }
       


    }

    //显示
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("显示"));
        catView = new ViewCategory(this, cat, this);
    }

    //context: page
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr("运行图"));
        contextPage = new PageContext(_diagram, cat, this);
        connect(contextPage, &PageContext::pageRemoved,
            naviView, &NaviTree::removePage);
        connect(contextPage, &PageContext::pageNameChanged,
            naviModel, &DiagramNaviModel::onPageNameChanged);
    }

    //context: train
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr("当前列车"));
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
    }

    //context: rail
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr("当前线路"));
        contextRail = new RailContext(_diagram, cat, this, this);
        connect(contextRail, &RailContext::railNameChanged,
            naviModel, &DiagramNaviModel::onRailNameChanged);
        connect(contextRail, &RailContext::stationTableChanged,
            this, &MainWindow::onStationTableChanged);
        connect(contextRail, &RailContext::selectRuler,
            this, &MainWindow::focusInRuler);
    }

    //context: ruler
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr("当前标尺"));
        contextRuler = new RulerContext(_diagram, cat, this);
        connect(contextRuler, SIGNAL(ordinateRulerModified(Railway&)),
            this, SLOT(updateRailwayDiagrams(Railway&)));
        connect(contextRuler, &RulerContext::focusOutRuler,
            this, &MainWindow::focusOutRuler);
    }

    //ApplicationMenu的初始化放在最后，因为可能用到前面的..
    initAppMenu();
    connect(ribbon->applicationButton(), SIGNAL(clicked()), this,
        SLOT(actPopupAppButton()));

    ribbon->setRibbonStyle(SARibbonBar::OfficeStyle);
}

void MainWindow::initAppMenu()
{
    auto* menu = new SARibbonMenu(tr("qETRC"), this);

    menu->addAction(sharedActions.newfile);
    menu->addAction(sharedActions.open);
    menu->addAction(sharedActions.save);

    auto* act = new QAction(QIcon(":/icons/saveas.png"), tr("另存为"), this);
    menu->addAction(act);

    menu->addSeparator();

    appMenu = menu;

    resetRecentActions();
}

void MainWindow::loadInitDiagram()
{
    if (!openGraph(SystemJson::instance.last_file))
        openGraph(SystemJson::instance.default_file);
    markUnchanged();
}

bool MainWindow::clearDiagram()
{
    //todo: 询问是否保存
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
}

void MainWindow::endResetGraph()
{
    resetDiagramPages();

    //导航窗口
    naviModel->resetModel();
    trainListWidget->refreshData();
    catView->refreshTypeGroup();

    spPassedStations->setValue(_diagram.config().max_passed_stations);
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
    if (changed && !saveQuestion())
        e->ignore();
    else
        e->accept();
}

//void MainWindow::informTrainListChanged()
//{
//    //注意：现在这个函数总是由TrainList那边变化发起的，所以TrainList不用更新
//    naviModel->resetModel();
//    //trainListWidget->refreshData();
//    markChanged();   //既然通知变化了，就默认真的发生了变化！
//}

void MainWindow::informPageListChanged()
{
    naviModel->resetModel();
    markChanged();
}

//void MainWindow::trainsReordered()
//{
//    //informTrainListChanged();
//}
//
//void MainWindow::trainSorted(const QList<std::shared_ptr<Train>>& oldList, 
//    TrainListModel* model)
//{
//    undoStack->push(new qecmd::SortTrains(oldList, model));
//    trainsReordered();
//}

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

void MainWindow::updateTrainLines(std::shared_ptr<Train> train, QList<std::shared_ptr<TrainAdapter>>&& adps)
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
    if (changed && !saveQuestion())
        return;
    QString res = QFileDialog::getOpenFileName(this, QObject::tr("打开"), QString(),
        QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    bool flag = openGraph(res);
    if (!flag)
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
    else
        markUnchanged();
}

void MainWindow::actSaveGraph()
{
    if (_diagram.filename().isEmpty())
        actSaveGraphAs();
    _diagram.save();
    markUnchanged();
    undoStack->setClean();
}

void MainWindow::actSaveGraphAs()
{
    QString res = QFileDialog::getSaveFileName(this, QObject::tr("另存为"), "",
        tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    bool flag = _diagram.saveAs(res);
    if (flag) {
        markUnchanged();
        undoStack->setClean();
        addRecentFile(res);
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
    QAction* act = qobject_cast<QAction*>(sender());
    if (act) {
        const QString& filename = act->data().toString();
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
}

void MainWindow::addPageWidget(std::shared_ptr<DiagramPage> page)
{

    int idx = diagramDocks.size();
    insertPageWidget(page, idx);
}

void MainWindow::insertPageWidget(std::shared_ptr<DiagramPage> page, int index)
{
    DiagramWidget* dw = new DiagramWidget(_diagram, page);
    auto* dock = new ads::CDockWidget(tr("运行图 - %1").arg(page->name()));
    dock->setWidget(dw);
    manager->addDockWidget(ads::RightDockWidgetArea, dock);
    QAction* act = dock->toggleViewAction();
    pageMenu->addAction(act);
    diagramDocks.insert(index, dock);
    diagramWidgets.insert(index, dw);
    //informPageListChanged();
    connect(dw, &DiagramWidget::trainSelected, this, &MainWindow::focusInTrain);
    connect(contextTrain, &TrainContext::highlightTrainLine, dw, &DiagramWidget::highlightTrain);
    connect(dw, &DiagramWidget::pageFocussedIn, this, &MainWindow::focusInPage);
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


