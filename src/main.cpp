#include "mainwindow/mainwindow.h"
#include "mobile/amainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <chrono>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    {
    QApplication a(argc, argv);
    //    qDebug()<<QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
    //           <<Qt::endl;
#ifdef QETRC_MOBILE
    AMainWindow w;
#else
    MainWindow w;
#endif
    w.showMaximized();
    return a.exec();
    }
}



