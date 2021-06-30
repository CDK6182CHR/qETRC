#include "mainwindow.h"

#include <QString>

//test...
#include <QTableView>
#include <QDockWidget>
#include <QHeaderView>
#include "model/train/trainlistmodel.h"
#include "editors/trainlistwidget.h"


MainWindow::MainWindow(QWidget *parent)
    : SARibbonMainWindow(parent)
{
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

    diagramWidget = new DiagramWidget(_diagram, this);

    manager = new ads::CDockManager(this);
    ads::CDockWidget* dock = new ads::CDockWidget(QObject::tr("运行图窗口"));
    dock->setWidget(diagramWidget);
    
    setDockOptions(dockOptions() | QMainWindow::VerticalTabs);

    auto* dock1 = new ads::CDockWidget(QObject::tr("车次管理"));
    auto* w = new TrainListWidget(_diagram.trainCollection());
    dock1->setWidget(w);

    manager->addDockWidget(ads::CenterDockWidgetArea, dock);
    manager->addDockWidget(ads::LeftDockWidgetArea, dock1);
}

MainWindow::~MainWindow()
{
    //test!!
    _diagram.saveAs("sample.json");
}

