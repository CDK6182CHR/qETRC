#include "mainwindow/mainwindow.h"
#include "mobile/amainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <chrono>
#include <QStandardPaths>

#include "mainwindow/startuppage.h"

int main(int argc, char *argv[])
{
    {
    QApplication a(argc, argv);
    //    qDebug()<<QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
    //           <<Qt::endl;
#ifdef QETRC_MOBILE
    AMainWindow w;
    w.showMaximized();
#else
    StartupPage::onStartup();
    a.processEvents();

    MainWindow w;
    w.showMaximized();
#endif
    return a.exec();
    }
}



