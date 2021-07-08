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


MainWindow::MainWindow(QWidget *parent)
    : SARibbonMainWindow(parent),
    manager(new ads::CDockManager(this)),
    naviModel(new DiagramNaviModel(_diagram,this)),
    undoStack(new QUndoStack(this))
{
    _diagram.readDefaultConfigs();
    undoStack->setUndoLimit(100);
    connect(undoStack, SIGNAL(indexChanged(int)), this, SLOT(markChanged()));

    initUI();
    loadInitDiagram();

    return;



    //暴力构造测试用例


    //auto s = QString(R"(D:\Python\train_graph\sample.pyetgr)");
    //auto s = QString(R"(D:\Python\train_graph\source\京沪线上局段20191230.pyetgr)");
    auto s = QString(R"(D:\Python\train_graph\source\濉阜线.json)");
    //auto s = R"(D:\Python\train_graph\source\成贵客专线F20191230-分颜色.pyetgr)";

    clearDiagramUnchecked();
    _diagram.fromJson(s);
    //_diagram.config().avoid_cover = false;
    qDebug() << "trains count: " << _diagram.trainCollection().trains().size() << Qt::endl;

    
    auto t = QString(R"(D:\Python\train_graph\source\阜淮、淮南线.json)");
    //auto t = QString(R"(D:\Python\train_graph\source\京局410\京广线京石段.json)");
    Diagram d2;
    d2.fromJson(t);
    _diagram.addRailway(d2.firstRailway());
    _diagram.addTrains(d2.trainCollection());

    _diagram.createDefaultPage();

    auto page = std::make_shared<DiagramPage>(
        QList<std::shared_ptr<Railway>>{ _diagram.railways().at(0) }, tr("濉阜线"));
    _diagram.pages().append(page);
    
    endResetGraph();

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
    for (auto p : diagramWidgets) {
        for (auto t : trains) {
            p->removeTrain(*t);
            contextTrain->removeTrainWidget(t);
            if (t == contextTrain->getTrain()) {
                contextTrain->resetTrain();
                focusOutTrain();
            }
        }
    }
    //informTrainListChanged();
}

void MainWindow::onStationTableChanged(std::shared_ptr<Railway> rail, bool equiv)
{
    _diagram.updateRailway(rail);
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


void MainWindow::undoAddPage(std::shared_ptr<DiagramPage> page)
{
    auto dock = diagramDocks.takeLast();
    manager->removeDockWidget(dock);
    delete dock;
    diagramWidgets.removeLast();
    informPageListChanged();
}

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
        connect(tree, &NaviTree::pageAdded, this, &MainWindow::actAddPage);
        connect(tree, &NaviTree::focusInPage, this, &MainWindow::focusInPage);
        //connect(tree, &NaviTree::focusOutPage, this, &MainWindow::focusOutPage);
        connect(tree, &NaviTree::focusInTrain, this, &MainWindow::focusInTrain);
        //connect(tree, &NaviTree::focusOutTrain, this, &MainWindow::focusOutTrain);
        connect(tree, &NaviTree::focusInRailway, this, &MainWindow::focusInRailway);
        //connect(tree, &NaviTree::focusOutRailway, this, &MainWindow::focusOutRailway);
        connect(tree, &NaviTree::editRailway, this, &MainWindow::actOpenRailStationWidget);
        connect(tree, &NaviTree::trainsImported, this, &MainWindow::onTrainsImported);
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
    }
    

    //开始
    if constexpr (true) {
        SARibbonCategory* cat = ribbon->addCategoryPage(QObject::tr("开始"));
        SARibbonPannel* panel = cat->addPannel(QObject::tr("文件"));

        QAction* act = new QAction(QIcon(":/icons/new-file.png"), QObject::tr("新建"), this);
        panel->addLargeAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_N);
        addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actNewGraph()));
        sharedActions.newfile = act;

        act = new QAction(QIcon(":/icons/open.png"), QObject::tr("打开"), this);
        addAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_O);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actOpenGraph()));
        sharedActions.open = act;

        act = new QAction(QIcon(":/icons/save1.png"), QObject::tr("保存"), this);
        act->setShortcut(Qt::CTRL + Qt::Key_S);
        addAction(act);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actSaveGraph()));
        sharedActions.save = act;

        panel = cat->addPannel(tr("窗口"));
        act = naviDock->toggleViewAction();
        act->setText(QObject::tr("导航"));
        act->setIcon(QIcon(":/icons/Graph-add.png"));
        auto* btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = trainListDock->toggleViewAction();
        act->setText(tr("列车管理"));
        act->setIcon(QIcon(":/icons/list.png"));
        btn=panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        SARibbonMenu* menu = new SARibbonMenu(this);
        pageMenu = menu;
        menu->setTitle(QObject::tr("运行图窗口"));
        menu->setIcon(QIcon(":/icons/diagram.png"));
        btn=panel->addLargeMenu(menu);
        btn->setMinimumWidth(80);

        //undo  试一下把dock也放在这里初始化...
        QUndoView* view = new QUndoView(undoStack);
        auto* dock = new ads::CDockWidget(tr("历史记录"));
        dock->setWidget(view);
        auto* a=manager->addDockWidgetTab(ads::CenterDockWidgetArea,dock);
        
        act = dock->toggleViewAction();
        act->setText(tr("历史记录"));
        act->setIcon(QIcon(":/icons/clock.png"));
        btn=panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        act = new QAction(QIcon(":/icons/add.png"), tr("添加运行图"), this);
        connect(act, SIGNAL(triggered()), naviView, SLOT(addNewPage()));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(80);

    }

    //线路
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("线路"));
        auto* panel = cat->addPannel(tr("基础数据"));

        auto* act = new QAction(QIcon(":/icons/add.png"), tr("导入线路"), this);
        connect(act, SIGNAL(triggered()), naviView, SLOT(importRailways()));
        auto* btn=panel->addLargeAction(act);
        btn->setMinimumWidth(80);

        auto* menu = new SARibbonMenu(this);
        railMenu = menu;
        menu->setTitle(tr("线路编辑"));
        menu->setIcon(QIcon(":/icons/rail.png"));
        btn = panel->addLargeMenu(menu);
        btn->setMinimumWidth(80);
    }

    //列车
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("列车"));
        auto* panel = cat->addPannel(tr("车次管理"));
        
        auto* act = new QAction(QIcon(":/icons/list.png"), tr("列车管理"), this);
        auto* menu = new QMenu(tr("列车管理扩展"), this);
        
        auto* actsub = new QAction(tr("删除所有车次"), this);
        connect(actsub, SIGNAL(triggered()), this, SLOT(removeAllTrains()));
        menu->addAction(actsub);
        
        act->setMenu(menu);
        auto* btn=panel->addLargeAction(act);
        btn->setMinimumWidth(70);

        act = new QAction(QIcon(":/icons/add_train.png"), tr("导入车次"), this);
        act->setShortcut(Qt::CTRL + Qt::Key_D);
        addAction(act);
        connect(act, SIGNAL(triggered()), naviView, SLOT(importTrains()));
        btn = panel->addLargeAction(act);
        btn->setMinimumWidth(70);
    }

    //显示
    if constexpr (true) {
        auto* cat = ribbon->addCategoryPage(tr("显示"));
        catView = new ViewCategory(this, cat, this);
    }

    //context: page
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr("运行图"));
        contextPage = cat;
        auto* page = cat->addCategoryPage(tr("管理"));
        auto* panel = page->addPannel(tr(""));

        QAction* act = new QAction(QIcon(":/icons/close.png"), tr("删除"), this);
        //todo: connect
        panel->addLargeAction(act);


    }

    //context: train & rail
    if constexpr (true) {
        auto* cat = ribbon->addContextCategory(tr("当前列车"));
        contextTrain = new TrainContext(_diagram, cat, this);

        cat = ribbon->addContextCategory(tr("当前线路"));
        contextRail = new RailContext(_diagram, cat, this, this);
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
}

void MainWindow::endResetGraph()
{
    resetDiagramPages();

    //导航窗口
    naviModel->resetModel();
    trainListWidget->refreshData();
    catView->refreshTypeGroup();
}

void MainWindow::resetDiagramPages()
{
    if (_diagram.pages().empty())
        _diagram.createDefaultPage();
    for (auto p : _diagram.pages()) {
        addPageWidget(p);
    }
}

bool MainWindow::openGraph(const QString& filename)
{
    clearDiagramUnchecked();

    Diagram dia;
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

void MainWindow::removeTrainLine(Train& train)
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
    auto* w = new RailStationWidget(undoStack);
    auto* dock = new ads::CDockWidget(tr("基线编辑-%1").arg(rail->name()));
    auto* act = dock->toggleViewAction();
    railMenu->addAction(act);
    dock->setWidget(w);
    w->setRailway(rail);
    connect(w, &RailStationWidget::stationTableChanged,
        this, &MainWindow::onStationTableChanged);
    railStationWidgets.append(w);
    railStationDocks.append(dock);

    manager->addDockWidgetFloating(dock);
}

void MainWindow::actOpenGraph()
{
    if (changed && !saveQuestion())
        return;
    QString res=QFileDialog::getOpenFileName(this, QObject::tr("打开"), QString(),
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
    bool flag=_diagram.saveAs(res);
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
            }
        }
    }
}

void MainWindow::addPageWidget(std::shared_ptr<DiagramPage> page)
{
    DiagramWidget* dw = new DiagramWidget(_diagram, page);
    auto* dock = new ads::CDockWidget(tr("运行图-%1").arg(page->name()));
    dock->setWidget(dw);
    manager->addDockWidget(ads::RightDockWidgetArea, dock);
    QAction* act = dock->toggleViewAction();
    pageMenu->addAction(act);
    diagramDocks.append(dock);
    diagramWidgets.append(dw);
    informPageListChanged();
    connect(dw, &DiagramWidget::trainSelected, this, &MainWindow::focusInTrain);
    connect(contextTrain, &TrainContext::highlightTrainLine, dw, &DiagramWidget::highlightTrain);
}

void MainWindow::actAddPage(std::shared_ptr<DiagramPage> page)
{
    undoStack->push(new qecmd::AddPage(_diagram, page, this));
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
    ribbonBar()->showContextCategory(contextPage);
}

void MainWindow::focusOutPage()
{
    ribbonBar()->hideContextCategory(contextPage);
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


