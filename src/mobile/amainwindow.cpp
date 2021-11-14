#include "adiagrampage.h"
#include "amainwindow.h"
#include "astartpage.h"

#ifdef QETRC_MOBILE

AMainWindow::AMainWindow()
{
    initUI();
}

void AMainWindow::initUI()
{
    pgStart=new AStartPage();
    addTab(pgStart,tr("开始"));
    connect(pgStart,&AStartPage::diagramRefreshed,
            this,&AMainWindow::refreshDiagram);

    pgDiagram=new ADiagramPage(pgStart->getDiagram());
    addTab(pgDiagram,tr("运行图"));
}

void AMainWindow::refreshDiagram()
{
    pgStart->refreshData();
    pgDiagram->refreshData();
}


#endif
