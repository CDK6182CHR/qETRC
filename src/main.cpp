#include "mainwindow.h"

#include <QApplication>

#ifndef LEAP_WITH_CRT

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
    auto s = QString(R"(D:\Python\train_graph\source\合九线.json)");
    //auto s = QString(R"(D:\Python\train_graph\source\京沪线上局段20191230.pyetgr)");
    {Diagram _diagram;
    _diagram.fromJson(s);
    qDebug()<<"Before end"<<Qt::endl;
    }
}

//内存泄漏检查...


#else

int main(int argc, char* argv[])
{
    _CrtDumpMemoryLeaks();
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(319);

	QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#endif
