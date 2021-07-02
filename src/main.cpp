#include "mainwindow.h"

#include <QApplication>

#ifndef LEAP_WITH_CRT

int main(int argc, char *argv[])
{
    {QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    return a.exec();
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
