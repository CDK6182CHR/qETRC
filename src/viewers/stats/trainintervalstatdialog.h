#pragma once

#include <QDialog>
#include "data/analysis/runstat/trainintervalstat.h"

class Diagram;
class SelectTrainCombo;
class RailCategory;
class QTextBrowser;
class QLineEdit;
class QComboBox;
class Train;

/**
 * @brief The TrainIntervalStatDialog class
 * The IntervalWidget in pyETRC, but for all trains.
 * Generally, init with no train.
 */
class TrainIntervalStatDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Train> train=nullptr;
    TrainIntervalStat stat;
    SelectTrainCombo* cbTrain;
    QComboBox* cbStart,*cbEnd;
    QLineEdit* edStations,*edStops,*edTotalTime,*edRunTime,*edStopTime;
    QLineEdit* edMile,*edTravelSpeed,*edTechSpeed;
    QTextBrowser* edPath;

    int endComboStartIndex;
public:
    TrainIntervalStatDialog(Diagram& diagram,
                            QWidget* parent=nullptr);
private:
    void initUI();
    void refreshForTrain();
    void setupStartCombo();

    /**
     * @brief setupSecondCombo
     * Setup the second combo; the start combo should be set before.
     * @param startIdx the (assumed) index in the startCombo.
     */
    void setupEndCombo(int startIdx);

    /**
     * @brief actualEndIndex
     * The end station index in the timetable.
     * The QComboBox::index() is not the actual index.
     */
    int actualEndIndex()const;
private slots:
    void setTrain(std::shared_ptr<Train> train);
    void onStartIndexChanged(int i);
    void onEndIndexChanged(int i);
    void refreshData();
};

