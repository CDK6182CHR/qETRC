#pragma once
#include <map>
#include "data/diagram/trainevents.h"

class GapConstraints;

/**
 * @brief The StationEventAxis class
 * 以时间轴形式，记载线路上车站的事件表。
 * 各个事件按照时间排序，支持二分查找。
 * 主要用于排图时，检测所给的时刻（或者：新插入的事件）是否符合约束条件。
 * 注意顺序应当由调用者来保证。本类直接开放vector的接口，并不做保证。
 */
class StationEventAxis:
        public QVector<std::shared_ptr<RailStationEvent>>
{
public:
    using QVector<std::shared_ptr<RailStationEvent>>::QVector;

    /**
     * @brief sortEvents
     * 按事件的时间进行排序。
     */
    void sortEvents();

    /**
     * @brief insertEvent
     * 二分查找方法插入新的事件，使得时间升序仍然维持。
     */
    void insertEvent(std::shared_ptr<RailStationEvent> ev);

    /**
     * @brief conflictEvent 找出与指定事件冲突的事件。
     * 如果没有冲突事件，即当前排图是许可的，则返回空。
     * @param ev  待插入的事件；或者说排图所导致的新增事件
     * @param constraint  排图约束条件
     * @return  按下列规则，**顺序**确定：
     * (1) 如果有左冲突事件（即时刻在ev之前的事件），优先返回左冲突事件。
     * (2) 暂定优先返回时刻离ev较近的事件。
     * 即从ev时刻开始，先左后右向两边遍历。
     */
    std::shared_ptr<RailStationEvent>
        conflictEvent(std::shared_ptr<RailStationEvent> ev,
                      const GapConstraints& constraint) const;

private:

    bool isConflict(std::shared_ptr<RailStationEvent> left,
                    std::shared_ptr<RailStationEvent> right,
                    const GapConstraints& constraint) const;
};

using RailStationEventList = StationEventAxis;


/**
 * 指定线路所有车站的事件顺序表。
 * 暂定直接using
 */
using RailwayStationEventAxis=std::map<std::shared_ptr<RailStation>,StationEventAxis>;



