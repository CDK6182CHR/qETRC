#include "mainwindow.h"

#include <QString>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //auto s = QString(R"(D:\Python\train_graph\sample.pyetgr)");
    //auto s = QString(R"(D:\Python\train_graph\source\合九线.json)");
    auto s = QString(R"(D:\Python\train_graph\source\濉阜线.json)");

    _diagram.fromJson(s);
    //_diagram.config().avoid_cover = false;
    qDebug() << "trains count: " << _diagram.trainCollection().trains().size() << Qt::endl;

    
    //auto t = QString(R"(D:\Python\train_graph\source\阜淮、淮南线.json)");
    auto t = QString(R"(D:\Python\train_graph\source\京局410\京广线京石段.json)");
    Diagram d2;
    d2.fromJson(t);
    _diagram.addRailway(d2.firstRailway());
    _diagram.addTrains(d2.trainCollection());

    DiagramWidget* w = new DiagramWidget(_diagram, this);
    setCentralWidget(w);
}

MainWindow::~MainWindow()
{
}

