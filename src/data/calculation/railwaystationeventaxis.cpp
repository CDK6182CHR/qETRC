#include "railwaystationeventaxis.h"
#include <util/utilfunc.h>

IntervalConflictReport RailwayStationEventAxis::intervalConflicted(std::shared_ptr<RailStation> from, 
    std::shared_ptr<RailStation> to, const RailStationEventBase& ev_start, 
    int secs, bool singleLine) const
{
    auto& ax_from = this->at(from);
    auto& ax_to = this->at(to);
    QTime tm_to = ev_start.time.addSecs(secs);

    // 搜索范围界限
    QTime leftBound = ev_start.time.addSecs(-secs * INTERVAL_SEARCH_SCALE);
    QTime rightBound = ev_start.time.addSecs(secs * INTERVAL_SEARCH_SCALE);

    // 这里使用UpperBound，相等的放到左边处理
    // 相等的要特殊处理一下，主要是不能直接break。除了区间共线外，不必相等的在这里不构成冲突。
    auto citr = std::upper_bound(ax_from.begin(), ax_from.end(), ev_start.time,
        RailStationEventBase::PtrTimeComparator());
    auto rcitr = std::make_reverse_iterator(citr);

    // 首先处理共线的情况
    if ((*citr)->time == ev_start.time) {
        auto [res, _] = isConflictedWith(ev_start, tm_to, *citr, ax_to, singleLine);
        if (res) {
            // 注意发站时刻相同的情况下还被定冲突了，只能是共线
            return { IntervalConflictReport::CoverConflict,nullptr };
        }
        else {
            ++rcitr;  // 跳过这个中间的
        }
    }

    // 首先向左搜索，解决区间越行的情况
    if (leftBound < ev_start.time) {
        // 没发生左越界的情况
        for (auto ritr = rcitr; ritr != ax_from.rend(); ++ritr) {
            if ((*ritr)->time < leftBound)break;
            auto [res, exeed] = isConflictedWith(ev_start, tm_to, *ritr, ax_to, singleLine);
            if (res)return { IntervalConflictReport::LeftConflict,res };
            else if (exeed) break;
        }
    }
    else {
        // 存在左越界的，跨日界线
        bool flag = false;
        for (auto ritr = rcitr; ritr != ax_from.rend(); ++ritr) {
            auto [res, exeed] = isConflictedWith(ev_start, tm_to, *ritr, ax_to, singleLine);
            if (res)
                return { IntervalConflictReport::LeftConflict,res };
            else if (exeed) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            for (auto ritr = ax_from.rbegin(); ritr != ax_from.rend(); ++ritr) {
                if ((*ritr)->time < leftBound)
                    break;
                auto [res, exeed] = isConflictedWith(ev_start, tm_to, *ritr, ax_to, singleLine);
                if (res)return { IntervalConflictReport::LeftConflict,res };
                else if (exeed) break;
            }
        }
    }

    // 向右搜索  注意右冲突返回的是from轴的事件！
    if (rightBound > ev_start.time) {
        // 不存在右跨日
        for (auto itr = citr; itr != ax_from.end(); ++itr) {
            if ((*itr)->time > rightBound)break;
            auto [res, exeed] = isConflictedWith(ev_start, tm_to, *itr, ax_to, singleLine);
            if (res) {
                return { IntervalConflictReport::RightConflict,*itr };
            }
            else if (exeed)break;
        }
    }
    else {
        // 存在右跨日
        bool flag = false;
        for (auto itr = citr; itr != ax_from.end(); ++itr) {
            auto [res, exeed] = isConflictedWith(ev_start, tm_to, *itr, ax_to, singleLine);
            if (res) {
                return { IntervalConflictReport::RightConflict,*itr };
            }
            else if (exeed) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            for (auto itr = ax_from.begin(); itr != citr; ++itr) {
                auto [res, exeed] = isConflictedWith(ev_start, tm_to, *itr, ax_to, singleLine);
                if (res) {
                    return { IntervalConflictReport::RightConflict,*itr };
                }
                else if (exeed)break;
            }
        }
    }

    // 检测完毕，无冲突
    return { IntervalConflictReport::NoConflict,nullptr };
}

std::pair<std::shared_ptr<RailStationEvent>, bool>
RailwayStationEventAxis::isConflictedWith(
    const RailStationEventBase& ev_start, const QTime& tm_to, 
    std::shared_ptr<RailStationEvent> ev_ex_start, const StationEventAxis& axis_to,
    bool singleLine) const
{
    // 首先排除不相关事件
    if (!(ev_ex_start->pos & qeutil::dirLatterPos(ev_start.dir))) {
        return {};
    }

    const auto& corr_ev_map = axis_to.positionEvents(qeutil::dirFormerPos(ev_start.dir));
    if (auto itr = corr_ev_map.find(ev_ex_start->line); itr != corr_ev_map.end()) {
        // 成功找到同一运行线对面的事件
        if (ev_ex_start->dir == ev_start.dir) {
            // 同向运行
            if (qeutil::timeCrossed(ev_start.time, tm_to, 
                ev_ex_start->time, itr->second->time)) {
                // 发生同向越行
                return std::make_pair(itr->second, false);
            }
            else {
                // 出发、到达都在同侧，实际上可以直接break了
                return std::make_pair(nullptr, true);
            }
        }
        else if(singleLine) {
            // 只有单线需要考虑反向的 和同向的区别是，对方运行线的两站时刻反着来
            if (qeutil::timeCrossed(ev_start.time, tm_to, itr->second->time, ev_ex_start->time)) {
                return std::make_pair(itr->second, false);
            }
            else {
                return std::make_pair(nullptr, true);
            }
        }
        // 只要检测到一条运行线完全早于或晚于待铺画运行线，就可以终止搜索，这是由搜索顺序决定的。
        // 如果此后还出现了与待排运行线冲突的，那么该运行线必然已经与现在检测的那一条运行线冲突了，
        // 则区间的约束已经被破坏，保证待排运行线与该运行线不相交没有意义。
        // 换句话说，假定已有的同侧运行线将“保护”待排运行线不被同侧更远的运行线干扰到。
        // 反向运行线，可以证明上面说的break条件对存在反向的情况仍然适用。
    }
    else {
        // 不存在对面运行线
    }
    return {};
}
