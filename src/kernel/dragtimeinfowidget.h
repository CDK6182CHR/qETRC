#pragma once
#include <QFrame>

class QLabel;
class TrainTime;

/**
 * @brief The DragTimeInfoWidget class
 * 2023.05.31  Show the information on dragging.
 */
class DragTimeInfoWidget : public QFrame
{
    Q_OBJECT
//    QLabel *labTrain,*labStation,*labPoint;
//    QLabel *labOldTime, *labNewTime;

    QLabel* labMain;

public:
    DragTimeInfoWidget(QWidget* parent=nullptr);
    void showInfo(bool shift, const QString& trainName, const QString& stationName,
                  const QString& pointName, const TrainTime& oldTime, const TrainTime& newTime);
private:
    void initUI();
};

