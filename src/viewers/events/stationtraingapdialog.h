#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <memory>
#include <vector>

#include "data/diagram/traingap.h"

class QCheckBox;
class QTableView;
class RailStation;
class Railway;
class Diagram;
class TrainFilterCore;
class TrainFilter;

/**
 * 暂定这里不包括转换算法，只是个简单的展示。
 */
class StationTrainGapModel: public QStandardItemModel
{
    Diagram& diagram;
    const std::shared_ptr<Railway> railway;
    const std::shared_ptr<RailStation> station;
    bool singleLine=false;

    /**
     * 2021.09.09 取消引用，改为值类型：为了保证从工具栏直接调起的操作是安全的
     */
    const RailStationEventList events;
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
        QObject* parent = nullptr,
        bool useSingleLine = false);
    void refreshData();
    const auto& getStat()const { return stat; }
    void setSingleLine(bool on){singleLine=on;}
    bool isSingleLine()const { return singleLine; }
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
     
    // filter必须在model之前初始化！！
    TrainFilter* const filter;
    StationTrainGapModel* const model;

    QTableView * table;
    QCheckBox* ckSingle;
public:

    /**
     * 此版本构造函数不创建新的filter，而是采用事件表页面传过来的filter对象。
     */
    StationTrainGapDialog(Diagram& diagram,
        std::shared_ptr<Railway> railway_,
        std::shared_ptr<RailStation> station_,
        const RailStationEventList& events,
        TrainFilter* filter,
        QWidget* parent = nullptr,
        bool useSingleLine = false);

    /**
     * 此版本用于从工具栏直接调起，现场创建自己的filter对象
     */
    StationTrainGapDialog(Diagram& diagram,
        std::shared_ptr<Railway> railway_,
        std::shared_ptr<RailStation> station_,
        QWidget* parent = nullptr);
private:
    void initUI();
private slots:
    void actToCsv();
    void actStatistics();
public slots:
    void refreshData();
};

