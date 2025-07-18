#include "railwaystationeventaxis.h"
#include <util/utilfunc.h>
#include <QDebug>

IntervalConflictReport RailwayStationEventAxis::intervalConflicted(std::shared_ptr<const RailStation> from, 
    std::shared_ptr<const RailStation> to, Direction dir, const TrainTime& tm_start, 
    int secs, bool singleLine, bool backward, int period_hours) const
{
    auto& ax_from = this->at(from);
    auto& ax_to = this->at(to);
    TrainTime tm_to = tm_start.addSecs(secs, period_hours);

    // 搜索范围界限
    TrainTime leftBound = tm_start.addSecs(-secs * INTERVAL_SEARCH_SCALE, period_hours);
    TrainTime rightBound = tm_start.addSecs(secs * INTERVAL_SEARCH_SCALE, period_hours);

    // 这里使用UpperBound，相等的放到左边处理
    // 相等的要特殊处理一下，主要是不能直接break。除了区间共线外，不必相等的在这里不构成冲突。
    auto citr = std::lower_bound(ax_from.begin(), ax_from.end(), tm_start,
        RailStationEventBase::PtrTimeComparator());
    auto rcitr = std::make_reverse_iterator(citr);

    // 首先处理共线的情况
    if (citr!=ax_from.end() && (*citr)->time == tm_start) {
        auto [res, _] = isConflictedWith(tm_start, tm_to, dir, *citr, ax_to, singleLine, period_hours);
        if (res) {
            // 注意发站时刻相同的情况下还被定冲突了，只能是共线
            return { IntervalConflictReport::CoverConflict,nullptr };
        }
        else {
            ++citr;  // 跳过这个中间的
        }
    }

    // 首先向左搜索，解决区间越行的情况
    if (leftBound < tm_start) {
        // 没发生左越界的情况
        for (auto ritr = rcitr; ritr != ax_from.rend(); ++ritr) {
            if ((*ritr)->time < leftBound)break;
            auto [res, exeed] = isConflictedWith(tm_start, tm_to, dir,  *ritr, ax_to, singleLine, period_hours);
            if (res)return { IntervalConflictReport::LeftConflict, backward ? *ritr : res };
            else if (exeed) break;
        }
    }
    else {
        // 存在左越界的，跨日界线
        bool flag = false;
        for (auto ritr = rcitr; ritr != ax_from.rend(); ++ritr) {
            auto [res, exeed] = isConflictedWith(tm_start, tm_to, dir, *ritr, ax_to, singleLine, period_hours);
            if (res)
                return { IntervalConflictReport::LeftConflict,backward ? *ritr : res };
            else if (exeed) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            for (auto ritr = ax_from.rbegin(); ritr != ax_from.rend(); ++ritr) {
                if ((*ritr)->time < leftBound)
                    break;
                auto [res, exeed] = isConflictedWith(tm_start, tm_to, dir, *ritr, ax_to, singleLine, period_hours);
                if (res)return { IntervalConflictReport::LeftConflict, backward ? *ritr : res };
                else if (exeed) break;
            }
        }
    }

    // 向右搜索  注意右冲突返回的是from轴的事件！
    if (rightBound > tm_start) {
        // 不存在右跨日
        for (auto itr = citr; itr != ax_from.end(); ++itr) {
            if ((*itr)->time > rightBound)break;
            auto [res, exeed] = isConflictedWith(tm_start, tm_to, dir, *itr, ax_to, singleLine, period_hours);
            if (res) {
                return { IntervalConflictReport::RightConflict, backward? res: *itr };
            }
            else if (exeed)break;
        }
    }
    else {
        // 存在右跨日
        bool flag = false;
        for (auto itr = citr; itr != ax_from.end(); ++itr) {
            auto [res, exeed] = isConflictedWith(tm_start, tm_to, dir, *itr, ax_to, singleLine, period_hours);
            if (res) {
                return { IntervalConflictReport::RightConflict, backward ? res : *itr };
            }
            else if (exeed) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            for (auto itr = ax_from.begin(); itr != citr; ++itr) {
                auto [res, exeed] = isConflictedWith(tm_start, tm_to, dir, *itr, ax_to, singleLine, period_hours);
                if (res) {
                    return { IntervalConflictReport::RightConflict, backward ? res : *itr };
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
    const TrainTime& tm_start, const TrainTime& tm_to, Direction dir,
    std::shared_ptr<RailStationEvent> ev_ex_start, const StationEventAxis& axis_to,
    bool singleLine, int period_hours) const
{
    // 首先排除不相关事件
    if (!(ev_ex_start->pos & qeutil::dirLatterPos(dir))) {
        return {};
    }

    const auto& corr_ev_map = axis_to.positionEvents(qeutil::dirFormerPos(dir));
    if (auto itr = corr_ev_map.find(ev_ex_start->line); itr != corr_ev_map.end()) {
        // 成功找到同一运行线对面的事件
        //qDebug() << "Another event: " << itr->second->time << Qt::endl;
        if (ev_ex_start->dir == dir) {
            // 同向运行
            if (qeutil::timeCrossed(tm_start, ev_ex_start->time,
                tm_to, itr->second->time, period_hours)) {
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
            if (qeutil::timeCrossed(tm_start, ev_ex_start->time, tm_to, itr->second->time, period_hours)) {
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
