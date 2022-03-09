#pragma once
#include <memory>
#include "stationeventaxis.h"
#include "intervalconflictreport.h"
#include "intervalconflictreport.h"

/**
 * 指定线路所有车站的事件顺序表。
 * 目前主要是StationEventAxis的集合，但需要支持站间查找的功能。
 */
class RailwayStationEventAxis :
    public std::map<std::shared_ptr<RailStation>, StationEventAxis>
{
    using Base = std::map<std::shared_ptr<RailStation>, StationEventAxis>;
public:
    using Base::map;

    /**
     * 搜索区间冲突情况时，在出发时刻左右多大范围内查找可能冲突的运行线
     * 单位为区间运行标尺的倍数
     */
    constexpr static const int INTERVAL_SEARCH_SCALE = 5;

    /**
     * 进行区间冲突检测。
     * @param from 区间发站
     * @param to 区间到站 发站和到站定义了当前要检测的区间，并且区间方向应当和
     * ev_start所示排图方向一致，但这里不做检查
     * @param ev_start 待校验事件在from车站的预设事件，同时包含了排图上下行信息。排图的上下行信息
     * 将同时决定取用from站的pre事件表还是post事件表，对to亦然。
     * @param secs 区间运行标尺
     * @param singleLine 是否单线，即是否检测对向敌对进路
     * @return 发现冲突的运行线的相关事件；该事件的时刻为下一尝试时刻。
     */
    IntervalConflictReport
        intervalConflicted(
            std::shared_ptr<RailStation> from, std::shared_ptr<RailStation> to,
            const RailStationEventBase& ev_start, int secs, bool singleLine)const;

private:

    /**
     * 针对所给事件ev_ex_start，判定该事件所指示的列车运行线与待铺画列车运行线是否存在区间冲突。
     * 区间冲突即区间交叉，检测原理是发站和到站的事件的先后顺序反过来了。
     * 注意由于仅在中心时刻附近一定范围搜索，这里认为搜索范围不会碰到12小时界限，因此直接采用PBC判定准则是安全的。
     * @param ev_start 待铺画列车事件
     * @param tm_to  预告下站时刻
     * @param ev_ex_start  已存在的出发站事件。事件并不保证和待铺画事件有关；如果无关（站前站后事件互不干扰）
     * 直接返回空。
     * @param axis_to  指定到站的事件轴
     * @param singleLine  单线
     * @return  如果存在冲突，返回冲突运行线在to站的事件；否则返回空。第二个元素：如果为true，
     * 表明搜索已经超过必要范围，可以停止
     */
    std::pair<std::shared_ptr<RailStationEvent>, bool>
        isConflictedWith(const RailStationEventBase& ev_start, const QTime& tm_to,
            std::shared_ptr<RailStationEvent> ev_ex_start, const StationEventAxis& axis_to, bool singleLine)const;
};

