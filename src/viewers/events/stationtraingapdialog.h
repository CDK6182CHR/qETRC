#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <memory>
#include <vector>
#include <optional>

#include "data/diagram/traingap.h"
#include "data/calculation/stationeventaxis.h"
#include "data/common/qeglobal.h"

class QCheckBox;
class QTableView;
class RailStation;
class Railway;
class Diagram;
class TrainFilterCore;
class TrainFilterSelector;

/**
 * 暂定这里不包括转换算法，只是个简单的展示。
 */
class StationTrainGapModel: public QStandardItemModel
{
    Q_OBJECT
    Diagram& diagram;
    const std::shared_ptr<Railway> railway;
    const std::shared_ptr<RailStation> station;
    //std::optional<bool> singleLine;
    int _cutSecs = 0;

    /**
     * 2021.09.09 取消引用，改为值类型：为了保证从工具栏直接调起的操作是安全的
     */
    const RailStationEventList events;
    const TrainFilterCore* filter=nullptr;
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
        QObject* parent = nullptr,
        int cutSecs_ = 0);
    void refreshData(const TrainFilterCore* core);
    const auto& getStat()const { return stat; }
    //void setSingleLine(std::optional<bool> on){singleLine=on;}
    //auto isSingleLine()const { return singleLine; }
    void setCutSecs(int v) { _cutSecs = v; }
    int cutSecs()const { return _cutSecs; }
    std::shared_ptr<TrainGap> gapForRow(int row);
    auto& getDiagram() { return diagram; }
    void setFilter(const TrainFilterCore* core) { filter = core; }
private:
    void setupModel();
    
    /**
     * 判断所给的间隔是否是同类中最小的。
     * 通通间隔，只要站前和站后中，有一个是，就算是
     */
    bool isMinimumGap(const TrainGap& gap)const;

    void setRowColor(int row, const QColor& color);
};

class QSpinBox;

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
    TrainFilterSelector* const filter;
    StationTrainGapModel* const model;

    QTableView * table;
    //QCheckBox* ckSingle;
    QSpinBox* spCut;
public:

    /**
     * 此版本构造函数不创建新的filter，而是采用事件表页面传过来的filter对象。
     * 2023.01.23：新版列车筛选器下，不允许传递列车筛选器对象，而只是给一个预设core用来初始化筛选器。
     */
    StationTrainGapDialog(Diagram& diagram,
        std::shared_ptr<Railway> railway_,
        std::shared_ptr<RailStation> station_,
        const RailStationEventList& events,
        const TrainFilterCore& filterCore,
        QWidget* parent = nullptr,
        int cutSecs=0);

    /**
     * 此版本用于从工具栏直接调起，现场创建自己的filter对象
     */
    StationTrainGapDialog(Diagram& diagram,
        std::shared_ptr<Railway> railway_,
        std::shared_ptr<RailStation> station_,
        QWidget* parent = nullptr);
private:
    void initUI();
    void locateToTime(const TrainTime& tm);
signals:
    void locateToEvent(int pageIndex, std::shared_ptr<const Railway>, std::shared_ptr<const RailStation>,
        const TrainTime& time);
private slots:
    void actToCsv();
    void actStatistics();
    void locateLeft();
    void locateRight();
public slots:
    void refreshData();
};

