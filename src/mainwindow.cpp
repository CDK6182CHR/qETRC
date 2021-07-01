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


MainWindow::MainWindow(QWidget *parent)
    : SARibbonMainWindow(parent),manager(new ads::CDockManager(this)),
    naviModel(new DiagramNaviModel(_diagram,this))
{
    _diagram.readDefaultConfigs();

    initUI();

    return;








    //auto s = QString(R"(D:\Python\train_graph\sample.pyetgr)");
    //auto s = QString(R"(D:\Python\train_graph\source\京沪线上局段20191230.pyetgr)");
    auto s = QString(R"(D:\Python\train_graph\source\濉阜线.json)");
    //auto s = R"(D:\Python\train_graph\source\成贵客专线F20191230-分颜色.pyetgr)";

    _diagram.fromJson(s);
    //_diagram.config().avoid_cover = false;
    qDebug() << "trains count: " << _diagram.trainCollection().trains().size() << Qt::endl;

    
    auto t = QString(R"(D:\Python\train_graph\source\阜淮、淮南线.json)");
    //auto t = QString(R"(D:\Python\train_graph\source\京局410\京广线京石段.json)");
    Diagram d2;
    d2.fromJson(t);
    _diagram.addRailway(d2.firstRailway());
    _diagram.addTrains(d2.trainCollection());

    diagramWidget = new DiagramWidget(*(_diagram.createDefaultPage()), this);

    ads::CDockWidget* dock = new ads::CDockWidget(QObject::tr("运行图窗口"));
    dock->setWidget(diagramWidget);

    auto* dock1 = new ads::CDockWidget(QObject::tr("车次管理"));
    auto* w = new TrainListWidget(_diagram.trainCollection());
    dock1->setWidget(w);

    manager->addDockWidget(ads::CenterDockWidgetArea, dock);
    manager->addDockWidget(ads::LeftDockWidgetArea, dock1);

    //
    QTreeView* tree = new QTreeView;
    DiagramNaviModel* dm = new DiagramNaviModel(_diagram, this);
    tree->setModel(dm);
    auto* dock2 = new ads::CDockWidget(QObject::tr("运行图资源管理器"));
    dock2->setWidget(tree);

    manager->addDockWidget(ads::LeftDockWidgetArea, dock2);

}

MainWindow::~MainWindow()
{

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
}

void MainWindow::initToolbar()
{
    SARibbonBar* ribbon = ribbonBar();
    ribbon->applitionButton()->setText(QStringLiteral("文件"));

    //开始
    if constexpr (true) {
        SARibbonCategory* cat = ribbon->addCategoryPage(QObject::tr("开始"));
        SARibbonPannel* panel = cat->addPannel(QObject::tr("文件"));

        QAction* act = new QAction(QIcon(":/icons/new-file.png"), QObject::tr("新建"), this);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actNewGraph()));

        act = new QAction(QIcon(":/icons/open.png"), QObject::tr("打开"), this);
        panel->addLargeAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(actOpenGraph()));

        act = new QAction(QIcon(":/icons/save1.png"), QObject::tr("保存"), this);
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

void MainWindow::actOpenGraph()
{
    if (!clearDiagram())
        return;
    QString res=QFileDialog::getOpenFileName(this, QObject::tr("打开"), QString(),
        QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isNull())
        return;
    bool flag = openGraph(res);
    if (!flag)
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("文件错误，请检查!"));
}

void MainWindow::actSaveGraph()
{
    if (_diagram.filename().isEmpty())
        actSaveGraphAs();
    _diagram.save();
}

void MainWindow::actSaveGraphAs()
{
}

void MainWindow::addPageWidget(std::shared_ptr<DiagramPage> page)
{
    DiagramWidget* dw = new DiagramWidget(*page);
    auto* dock = new ads::CDockWidget(page->name());
    dock->setWidget(dw);
    manager->addDockWidget(ads::RightDockWidgetArea, dock);
    QAction* act = dock->toggleViewAction();
    pageMenu->addAction(act);
    diagramDocks.append(dock);
}

