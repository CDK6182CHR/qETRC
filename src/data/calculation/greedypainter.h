#pragma once
#include <memory>
#include "gapconstraints.h"
#include "stationeventaxis.h"

class Diagram;
class TrainName;
class Railway;
/**
 * @brief The GreedyPainter class
 * 贪心算法全自动铺画运行线。
 * 本类包含铺画约束条件（间隔约束条件，必停站，锚点站，起讫点）和铺图核心算法。
 * 本类只管生成铺画区间的运行线，不去管整合的问题。
 */
class GreedyPainter
{
    Diagram& diagram;
    std::shared_ptr<Railway> _railway;
    std::shared_ptr<RailStation> _anchor,_start,_end;
    QTime _anchorTime;

    /**
     * @brief _train
     * 这是铺画数据的目标；这里新建。
     */
    std::shared_ptr<Train> _train;
    GapConstraints _constraints;
    RailwayStationEventAxis _railAxis;

public:
    GreedyPainter(Diagram& diagram);
    auto railway(){return _railway;}
    auto anchor(){return _anchor;}
    auto start(){return _start;}
    auto end(){return _end;}
    void setRailway(std::shared_ptr<Railway> railway){_railway=railway;}
    void setAnchor(std::shared_ptr<RailStation> anchor){_anchor=anchor;}
    void setStart(std::shared_ptr<RailStation> value){_start=value;}
    void setEnd(std::shared_ptr<RailStation> value){_end=value;}
    auto& constraints(){return _constraints;}
    const auto& constraints()const{return _constraints;}
    auto train(){return _train;}

    /**
     * @brief paint  核心接口函数，铺画运行线。
     * @param trainName  新铺列车的车次，根据这个车次创建新对象。这个车次其实也没多大用
     * @return 铺画结果。如果铺画失败，返回是否成功。
     * see also: train()
     * 暂定在这里算事件表，以防止中途变化了
     */
    bool paint(const TrainName& trainName);
};

