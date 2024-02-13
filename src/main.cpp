#include "mainwindow/mainwindow.h"
#include "mobile/amainwindow.h"
#include "data/common/qesystem.h"

#include <QApplication>
#include <QStyleFactory>
#include <chrono>
#include <QStandardPaths>
#include <QTranslator>

#include "mainwindow/startuppage.h"

int main(int argc, char *argv[])
{
    {
    QApplication a(argc, argv);
    //    qDebug()<<QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
    //           <<Qt::endl;

    // translation
    QTranslator trans;
    bool res = trans.load(QLocale{ SystemJson::instance.language }, "./tr/", "qETRC_");
    if (!res) {
        qWarning() << "load translation file failed!";
    }
    else {
        a.installTranslator(&trans);
    }
    
    // translation of Qt
    QTranslator trans2;
    bool res2 = trans2.load(QLocale{ SystemJson::instance.language }, "./translations/", "qt_");
    if (!res2) {
        qWarning() << "load Qt translation file failed!";
    }
    else {
        a.installTranslator(&trans2);
    }

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



