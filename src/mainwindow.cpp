#include "mainwindow.h"

#include <QString>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //auto s = QString(R"(D:\Python\train_graph\sample.pyetgr)");
//    auto s = QString(R"(D:\Python\train_graph\source\合九线.json)");
    auto s = QString(R"(D:\Python\train_graph\source\京沪线上局段20191230.pyetgr)");
    _diagram.fromJson(s);
    //_diagram.config().avoid_cover = false;
    qDebug() << "trains count: " << _diagram.trainCollection().trains().size() << Qt::endl;

    auto t = QString(R"(D:\Python\train_graph\source\符夹线.json)");
    Diagram d2(t);
    _diagram.addRailway(d2.firstRailway());

    DiagramWidget* w = new DiagramWidget(_diagram, this);
    setCentralWidget(w);
}

MainWindow::~MainWindow()
{
}

