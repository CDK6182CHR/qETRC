#pragma once

#include <data/diagram/traingap.h>
#include <data/calculation/stationeventaxis.h>
class Railway;


class TrainFilterCore;
class Diagram;

/**
 * @brief The TrainGapAna class
 * 2022.04.30 新增
 * 将TrainGap使用类似IntTrains的模式实现分析，
 * 相关算法主要封装到这个类里面。
 * 原则上所有算法都应该从Diagram下面弄出来，但现在暂时不做这一步重构
 * 主要目的是封装全局最小间隔计算
 * 本类构造压力不大，允许在stack上构造。
 * @see traingapstatdialog.cpp
 */
class TrainGapAna
{
    Diagram& diagram;
    const TrainFilterCore* filter=nullptr;
//    bool _singleLine=false;
    int _cutSecs = 24 * 3600;   // 2025.02.06: default value: seconds of a day (max possible value)
public:
    TrainGapAna(Diagram& diagram);
    TrainGapAna(Diagram& diagram, const TrainFilterCore* filter);

    void setFilter(const TrainFilterCore* f) { this->filter = f; }

//    void setSingleLine(bool on){_singleLine=on;}
    void setCutSecs(int secs){_cutSecs=secs;}

    std::map<TrainGap::GapTypesV2,int>
        globalMinimal(std::shared_ptr<Railway> rail)const;

    /**
     * @brief calTrainGaps  由所给的列车事件表计算列车间隔。
     * 2022.09.11添加，此版本使用内建的单双线数据。
     * 原Diagram::getTrainGaps()接口标记为废弃。
     * 上一个commit中，是Diagram::getTrainGapsReal()函数。
     * @param events  列车事件表。保证列车事件按时间顺序排列。
     * @param filter  列车筛选器核心。
     * @param st  要计算事件的车站。
     * @return  列车间隔列表。
     */
    TrainGapList calTrainGaps(const RailStationEventList& events,
        std::shared_ptr<const RailStation> st)const;

    /**
     * 2021.09.08
     * 统计各种不同类型的列车间隔。
     * Positions只取Pre, Post或者空白三种情况 （空白仅针对Avoid类型的）；
     * 通通的，同时算入两边。set按照TrainGap的间隔自动排序。
     */
    TrainGapStatistics countTrainGaps(const TrainGapList& gaps, int curSecs)const;

private:
    /**
     * 2022.09.11  从Diagram同名函数迁移过来
     * 倒着查，从车站事件表中查找最后一个符合指定方向、位置的事件。
     * 如果找不到（极端情况），返回空。
     */
    std::shared_ptr<const RailStationEvent>
        findLastEvent(const RailStationEventList& lst,
            const Direction& dir,
            RailStationEvent::Positions pos)const;
};

