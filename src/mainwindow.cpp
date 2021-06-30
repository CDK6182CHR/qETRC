#include "mainwindow.h"

#include <QString>

//test...
#include <QTableView>
#include <QDockWidget>
#include <QHeaderView>
#include "model/train/trainlistmodel.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //auto s = QString(R"(D:\Python\train_graph\sample.pyetgr)");
    //auto s = QString(R"(D:\Python\train_graph\source\京沪线上局段20191230.pyetgr)");
    //auto s = QString(R"(D:\Python\train_graph\source\濉阜线.json)");
    auto s = R"(D:\Python\train_graph\source\成贵客专线F20191230-分颜色.pyetgr)";

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
    setCentralWidget(diagramWidget);

    //QTableView* view = new QTableView;
    //view->verticalHeader()->setDefaultSectionSize(20);
    //QDockWidget* dock = new QDockWidget;
    //TrainListModel* model = new TrainListModel(_diagram.trainCollection(), this);
    //view->setModel(model);
    //for (int i = 0; i < 7; i++) {
    //    //view->setColumnWidth(i, widths[i]);
    //}
    //view->resizeColumnsToContents();
    //dock->setWidget(view);
    //addDockWidget(Qt::LeftDockWidgetArea, dock);
}

MainWindow::~MainWindow()
{
    //test!!
    _diagram.saveAs("sample.json");
}

