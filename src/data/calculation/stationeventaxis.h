#pragma once
#include <map>
#include <unordered_map>
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
    using line_map_t = std::unordered_map<std::shared_ptr<const TrainLine>, std::shared_ptr<RailStationEvent>>;
    /**
     * 2022.03.09
     * 构建运行线->事件的查找表。
     * 显然，同一运行线在站前或站后分别最多只出现一次。通过事件同时算站前和站后。
     */
     line_map_t _preEvents, _postEvents;
public:
    using QVector<std::shared_ptr<RailStationEvent>>::QVector;

    const auto& preEvents()const { return _preEvents; }
    const auto& postEvents()const { return _postEvents; }

    /**
     * 根据pre还是Post选择一个映射表。
     * 注意这里必须是pre或者post，不能是Both；否则出错 （暂定直接抛异常）
     */
    const line_map_t & positionEvents(RailStationEventBase::Position pos)const;

    /**
     * 实行排序并建立映射表
     */
    void buildAxis();

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
     * @param singleLine  2022.09.11增加。这是区间的local单双线性质，由railint算出来。
     * constraint不再包含单双线性质。
     * @return  按下列规则，**顺序**确定：
     * (1) 如果有左冲突事件（即时刻在ev之前的事件），优先返回左冲突事件。
     * (2) 暂定优先返回时刻离ev较近的事件。
     * 即从ev时刻开始，先左后右向两边遍历。
     */
    std::shared_ptr<RailStationEvent>
        conflictEvent(const RailStationEventBase& ev,
                      const GapConstraints& constraint,
                      bool singleLine, int period_hours) const;

private:

    /**
     * @brief sortEvents
     * 按事件的时间进行排序。
     */
    void sortEvents();

    /**
     * 生成运行线到事件的映射表
     */
    void constructLineMap();

    bool isConflict(const RailStationEventBase& left,
                    const RailStationEventBase& right,
                    const GapConstraints& constraint,
                    bool singleLine) const;


};

using RailStationEventList = StationEventAxis;



