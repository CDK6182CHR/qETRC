#include "mainwindow/mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <chrono>


int main(int argc, char *argv[])
{
    {
    auto start = std::chrono::system_clock::now();
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    qDebug() << "System init consumes: " <<
        static_cast<double>(duration.count()) * std::chrono::microseconds::period::num /
        std::chrono::microseconds::period::den << Qt::endl;
    return a.exec();
    }
}



