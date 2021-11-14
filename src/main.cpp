#include "mainwindow/mainwindow.h"
#include "mobile/amainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <chrono>


int main(int argc, char *argv[])
{
    {
    auto start = std::chrono::system_clock::now();
    QApplication a(argc, argv);
#ifdef QETRC_MOBILE
    AMainWindow w;
#else
    MainWindow w;
#endif
    w.showMaximized();
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return a.exec();
    }
}



