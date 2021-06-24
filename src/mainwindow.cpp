#include "mainwindow.h"

#include <QString>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto s = QString(R"(D:\Python\train_graph\sample.pyetgr)");
    _diagram.fromJson(s);

    auto t = QString(R"(D:\Python\train_graph\source\符夹线.json)");
    Diagram d2(t);
    _diagram.addRailway(d2.firstRailway());

    DiagramWidget* w = new DiagramWidget(_diagram, this);
    setCentralWidget(w);
}

MainWindow::~MainWindow()
{
}

