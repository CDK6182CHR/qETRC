#pragma once

#ifdef QETRC_MOBILE

#include <QTabWidget>

class ADiagramPage;
class AStartPage;
class AMainWindow : public QTabWidget
{
    Q_OBJECT
    AStartPage* pgStart;
    ADiagramPage* pgDiagram;
public:
    AMainWindow();
private:
    void initUI();
private slots:
    void refreshDiagram();
};


#endif
