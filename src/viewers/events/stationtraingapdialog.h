#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <memory>
#include <vector>

#include "data/diagram/traingap.h"

class QTableView;
class RailStation;
class Railway;
class Diagram;
class TrainFilterCore;

/**
 * 暂定这里不包括转换算法，只是个简单的展示。
 */
class StationTrainGapModel: public QStandardItemModel
{
    Diagram& diagram;
    const std::shared_ptr<Railway> railway;
    const std::shared_ptr<RailStation> station;
    const RailStationEventList& events;
    const TrainFilterCore& filter;
    TrainGapList data;
    TrainGapStatistics stat;
public:
    enum {
        ColGapType = 0,
        ColPeriod,
        ColPos,
        ColLeftTrain,
        ColLeftTime,
        ColRightTrain,
        ColRightTime,
        ColNumber,
        ColMAX
    };
    StationTrainGapModel(Diagram& diagram,
        std::shared_ptr<Railway> railway_,
        std::shared_ptr<RailStation> station_,
        const RailStationEventList& events,
        const TrainFilterCore& filter,
        QObject* parent = nullptr);
    void refreshData();
private:
    void setupModel();
    
    /**
     * 判断所给的间隔是否是同类中最小的。
     * 通通间隔，只要站前和站后中，有一个是，就算是
     */
    bool isMinimumGap(const TrainGap& gap)const;

    void setRowColor(int row, const QColor& color);
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
    StationTrainGapDialog(Diagram& diagram,
        std::shared_ptr<Railway> railway_,
        std::shared_ptr<RailStation> station_,
        const RailStationEventList& events,
        const TrainFilterCore& filter,
        QWidget* parent = nullptr);
    void refreshData();
private:
    void initUI();
private slots:
    void actToCsv();
};

