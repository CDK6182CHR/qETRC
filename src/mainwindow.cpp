#include "mainwindow.h"

#include <QString>

//test...
#include <QTableView>
#include <QDockWidget>
#include <QtWidgets>
#include "model/train/trainlistmodel.h"
#include "editors/trainlistwidget.h"
#include "model/diagram/diagramnavimodel.h"
#include "data/diagram/diagram.h"

#include "SARibbonBar.h"
#include "SARibbonCategory.h"
#include "SARibbonMenu.h"
#include "SARibbonPannel.h"
#include "SARibbonQuickAccessBar.h"


MainWindow::MainWindow(QWidget *parent)
    : SARibbonMainWindow(parent),manager(new ads::CDockManager(this)),
    naviModel(new DiagramNaviModel(_diagram,this)),
    undoStack(new QUndoStack(this))
{
    _diagram.readDefaultConfigs();

    initUI();

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

    auto page = std::make_shared<DiagramPage>(_diagram,
        QList<std::shared_ptr<Railway>>{ _diagram.railways().at(0) }, tr("濉阜线"));
    _diagram.pages().append(page);
    
    endResetGraph();

}

MainWindow::~MainWindow()
{

}

void MainWindow::undoRemoveTrains(const QList<std::shared_ptr<Train>>& trains, 
    const QList<int>& indexes)
{
    //重新添加运行线
    for (auto p : trains) {
        addTrainLine(*p);
    }
    informTrainListChanged();
}

void MainWindow::initUI()
{
    initDockWidgets();
    initToolbar();

    loadInitDiagram();
}

void MainWindow::initDockWidgets()
{
    ads::CDockWidget* dock;

    //总导航
    auto* tree = new QTreeView;
    naviView = tree;
    dock = new ads::CDockWidget(QObject::tr("运行图资源管理器"));
    naviDock = dock;
    tree->setModel(naviModel);
    dock->setWidget(tree);
    manager->addDockWidget(ads::LeftDockWidgetArea, dock);

    auto* tw = new TrainListWidget(_diagram.trainCollection());
    dock = new ads::CDockWidget(tr("列车管理"));
    trainListWidget = tw;
    trainListDock = dock;
    dock->setWidget(tw);
    manager->addDockWidget(ads::LeftDockWidgetArea, dock);
    connect(tw, SIGNAL(trainsRemoved(const QList<std::shared_ptr<Train>>&, const QList<int>&)),
        this, SLOT(trainsRemoved(const QList<std::shared_ptr<Train>>&, const QList<int>&)));
}

void MainWindow::initToolbar()
{
    SARibbonBar* ribbon = ribbonBar();
    ribbon->setRibbonStyle(SARibbonBar::WpsLiteStyle);
    ribbon->applitionButton()->setText(QStringLiteral("文件"));

    //顶上的工具条
    if constexpr (true) {
        //撤销重做
        QAction* act = undoStack->createUndoAction(this, tr("撤销"));
        act->setIcon(QIcon(":/icons/undo.png"));
        act->setShortcut(Qt::CTRL + Qt::Key_Z);
        ribbon->quickAccessBar()->addButton(act);

        act = undoStack->createRedoAction(this, tr("重做"));
        act->setIcon(QIcon(":/icons/redo.png"));
        act->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
        ribbon->quickAccessBar()->addButton(act);

    }

    //开始
    if constexpr (true) {
        SARibbonCategory* cat = ribbon->addCategoryPage(QObject::tr("开始"));
        SARibbonPannel* panel = cat->addPannel(QObject::tr("文件"));

        QAction* act = new QAction(QIcon(":/icons/new-file.png"), QObject::tr("新建"), this);
        panel->addLargeAction(act);
        act->setShortcut(Qt::CTRL + Qt::Key_N);
        connect(act, SIGNAL(triggered()), this, SLOT(actNewGraph()));

        act = new QAction(QIcon(":/icons/open.png"), QObject::tr("打开"), this);
        act->setShortcut(Qt::CTRL + Qt::Key_O);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actOpenGraph()));

        act = new QAction(QIcon(":/icons/save1.png"), QObject::tr("保存"), this);
        act->setShortcut(Qt::CTRL + Qt::Key_S);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actSaveGraph()));

        panel = cat->addPannel("窗口");
        act = naviDock->toggleViewAction();
        act->setText(QObject::tr("导航"));
        act->setIcon(QIcon(":/icons/Graph-add.png"));
        panel->addLargeAction(act);

        act = trainListDock->toggleViewAction();
        act->setText(tr("列车管理"));
        act->setIcon(QIcon(":/icons/list.png"));
        panel->addLargeAction(act);

        SARibbonMenu* menu = new SARibbonMenu(this);
        pageMenu = menu;
        menu->setTitle(QObject::tr("运行图窗口"));
        menu->setIcon(QIcon(":/icons/diagram.png"));
        panel->addLargeMenu(menu);
    }
}

void MainWindow::loadInitDiagram()
{
    if (!openGraph(SystemJson::instance.last_file))
        openGraph(SystemJson::instance.default_file);
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

    //最后：清理数据
    _diagram.clear();
}

void MainWindow::beforeResetGraph()
{
}

void MainWindow::actNewGraph()
{

}

void MainWindow::endResetGraph()
{
    resetDiagramPages();

    //导航窗口
    naviModel->resetModel();
    trainListWidget->refreshData();
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
        SystemJson::instance.addHistoryFile(filename);
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

void MainWindow::informTrainListChanged()
{
    naviModel->resetModel();
    trainListWidget->refreshData();
    changed = true;   //既然通知变化了，就默认真的发生了变化！
}

void MainWindow::actOpenGraph()
{
    //todo: 询问保存
    QString res=QFileDialog::getOpenFileName(this, QObject::tr("打开"), QString(),
        QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    bool flag = openGraph(res);
    if (!flag)
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
    else
        changed = false;
}

void MainWindow::actSaveGraph()
{
    if (_diagram.filename().isEmpty())
        actSaveGraphAs();
    _diagram.save();
    changed = false;
}

void MainWindow::actSaveGraphAs()
{
    QString res = QFileDialog::getSaveFileName(this, QObject::tr("另存为"), "",
        tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    _diagram.saveAs(res);
    changed = false;
}

void MainWindow::addPageWidget(std::shared_ptr<DiagramPage> page)
{
    DiagramWidget* dw = new DiagramWidget(page);
    auto* dock = new ads::CDockWidget(page->name());
    dock->setWidget(dw);
    manager->addDockWidget(ads::RightDockWidgetArea, dock);
    QAction* act = dock->toggleViewAction();
    pageMenu->addAction(act);
    diagramDocks.append(dock);
    diagramWidgets.append(dw);
}

void MainWindow::trainsRemoved(const QList<std::shared_ptr<Train>>& trains, const QList<int>& indexes)
{
    for (auto p : diagramWidgets) {
        for (auto t : trains) {
            p->removeTrain(*t);
        }
    }
    undoStack->push(new qecmd::RemoveTrains(trains, indexes, _diagram.trainCollection(), this));

    informTrainListChanged();
}


