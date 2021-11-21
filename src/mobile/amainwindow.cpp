#include "adiagrampage.h"
#include "amainwindow.h"
#include "arailpage.h"
#include "astartpage.h"
#include "atrainpage.h"

#ifdef QETRC_MOBILE

#include <QTabBar>

AMainWindow::AMainWindow()
{
    initUI();
//    setDocumentMode(true);
    tabBar()->setDrawBase(false);
}

void AMainWindow::initUI()
{
    pgStart=new AStartPage();
    addTab(pgStart,tr("开始"));
    connect(pgStart,&AStartPage::diagramRefreshed,
            this,&AMainWindow::refreshDiagram);

    pgDiagram=new ADiagramPage(pgStart->getDiagram());
    addTab(pgDiagram,tr("运行图"));
    connect(pgDiagram,&ADiagramPage::switchToTrain,
            this,&AMainWindow::switchToTrain);

    pgRail=new ARailPage(pgStart->getDiagram());
    addTab(pgRail,tr("线路"));

    pgTrain=new ATrainPage(pgStart->getDiagram());
    addTab(pgTrain,tr("列车"));
}

void AMainWindow::refreshDiagram()
{
    pgStart->refreshData();
    pgDiagram->refreshData();
    pgRail->refreshData();
    pgTrain->refreshData();
}

void AMainWindow::switchToTrain(std::shared_ptr<Train> train)
{
    if(train){
        pgTrain->setTrain(train);
        setCurrentIndex(PageTrain);
    }
}


#endif
