#pragma once

#include <QTableView>
#include <memory>
#include "model/train/timetablestdmodel.h"

class QTableView;
struct DiagramOptions;

/**
 * @brief The TrainTimetablePlane class
 * 只读的列车时刻表
 * 2022.02.10启用本类，并改为const Train
 */
class TrainTimetablePlane : public QTableView
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    std::shared_ptr<const Train> train{};
    TimetableConstModel*const model;
public:
    explicit TrainTimetablePlane(const DiagramOptions& ops, QWidget *parent = nullptr);

    void setTrain(std::shared_ptr<const Train> train_);


};

