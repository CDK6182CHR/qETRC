#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <memory>
#include <vector>

#include "data/diagram/traingap.h"

class QTableView;
class RailStation;
class Railway;

/**
 * 暂定这里不包括转换算法，只是个简单的展示。
 */
class StationTrainGapModel: public QStandardItemModel
{
    std::shared_ptr<Railway> railway;
    std::shared_ptr<RailStation> station;
    std::vector<TrainGap> data;
public:
    enum {
        ColGapType = 0,
        ColPeriod,
        ColDir,
        ColPos,
        ColLeftTrain,
        ColLeftTime,
        ColRightTrain,
        ColRightTime,
        ColNumber,
        ColMAX
    };
    StationTrainGapModel(std::shared_ptr<Railway> railway_,
                         std::shared_ptr<RailStation> station_,
                         std::vector<TrainGap>&& data_,
                         QObject* parent=nullptr);
    void refreshData();
private:
    void setupModel();
};


/**
 * @brief The StationTrainGapDialog class
 * 将车站事件表转换为列车间隔的表
 */
class StationTrainGapDialog : public QDialog
{
    Q_OBJECT
    std::shared_ptr<Railway> railway;
    std::shared_ptr<RailStation> station;
    StationTrainGapModel* const model;

    QTableView * table;
public:
    StationTrainGapDialog(std::shared_ptr<Railway> railway_,
                          std::shared_ptr<RailStation> station_,
                          std::vector<TrainGap>&& data_,
                          QWidget* parent=nullptr);
    void refreshData();
private:
    void initUI();
private slots:
    void actToCsv();
};

