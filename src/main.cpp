#include "mainwindow/mainwindow.h"

#include <QApplication>
#include <QStyleFactory>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}



