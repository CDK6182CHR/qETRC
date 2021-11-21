#pragma once

#ifdef QETRC_MOBILE

#include <QTabWidget>
#include <memory>

class ARailPage;
class Train;
class ATrainPage;
class ADiagramPage;
class AStartPage;
class AMainWindow : public QTabWidget
{
    Q_OBJECT
    AStartPage* pgStart;
    ADiagramPage* pgDiagram;
    ARailPage* pgRail;
    ATrainPage* pgTrain;
    enum {
        PageStart=0,
        PageDiagram,
        PageRailway,
        PageTrain,
        PageMAX
    };
public:
    AMainWindow();
private:
    void initUI();
private slots:
    void refreshDiagram();
public slots:
    void switchToTrain(std::shared_ptr<Train> train);
};


#endif
