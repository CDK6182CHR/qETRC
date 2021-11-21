#pragma once
#ifdef QETRC_MOBILE

#include <QWidget>

class SelectRailwayCombo;
class Diagram;
class ARailAnalysis : public QWidget
{
    Q_OBJECT
    Diagram& diagram;
    SelectRailwayCombo* const cbRails;
public:
    explicit ARailAnalysis(Diagram& diagram, SelectRailwayCombo* cbRails,
                           QWidget *parent = nullptr);
private:
    void initUI();

signals:

private slots:
    void sectionCount();
    void stationTrains();
    void stationEvents();
    void sectionEvents();
    void railSnap();
    void trackAnalysis();
    void intervalAnalysis();
    void intervalSumamry();

};

#endif // ARAILANALYSIS_H
