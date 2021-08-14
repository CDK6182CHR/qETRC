#pragma once

#include <QTableView>
#include <memory>
#include "model/train/timetablestdmodel.h"

class QTableView;

/**
 * @brief The TrainTimetablePlane class
 * 只读的列车时刻表
 */
class TrainTimetablePlane : public QTableView
{
    Q_OBJECT;
    std::shared_ptr<Train> train{};
    TimetableStdModel*const model;
public:
    explicit TrainTimetablePlane(QWidget *parent = nullptr);

    void setTrain(std::shared_ptr<Train> train_);


};

