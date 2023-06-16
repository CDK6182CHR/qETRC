#include "traingapana.h"

#include <data/diagram/diagram.h>
#include <data/train/trainfiltercore.h>
#include <data/rail/railstation.h>

namespace _gapdetail {

    /**
     * @brief 2022.09.10  getTrainGapsReal的helper
     * 记录指定站(站前站后)(上行下行) 共四种类型事件的上一个遇到的。
     * 对每一个新来的事件，判定上一个与之相关的是哪个事件。
     */
    struct EventStackTop {
        const std::optional<bool> preSingle, postSingle;
        std::shared_ptr<const RailStationEvent>
            downFormerLast, downLatterLast, upFormerLast, upLatterLast;

        EventStackTop(const RailStation& st):
            preSingle(st.isPreSingle()),postSingle(st.isPostSingle()){}

        void updateStack(const std::shared_ptr<const RailStationEvent>& ev);

        /**
         * @brief 站前事件中，与所给事件有关的上一个事件。
         */
        std::shared_ptr<const RailStationEvent> preRelatedEvent(const RailStationEvent& ev)const;
        std::shared_ptr<const RailStationEvent> postRelatedEvent(const RailStationEvent& ev)const;

        /**
         * 2023.06.14 Experimental: the COUNTER-place event related to the pre-station event, i.e. the post-station event.
         * The difference from preRelatedEvent is that, here the preSingle is not ensured to be valid.
         */
        std::shared_ptr<const RailStationEvent> preCounterEvent(const RailStationEvent& ev)const;
        std::shared_ptr<const RailStationEvent> postCounterEvent(const RailStationEvent& ev)const;

        /**
         * 判断两事件之间是否存在待避。对向判断的标准是：
         * （潜在的）被踩的车（stay事件所示）的【出发】方向存在敌对进路。
         * 当然，同向的肯定是算的。
         */
        bool hasAvoidEvent(const RailStationEvent* stay, const RailStationEvent* leave);

    };

    void EventStackTop::updateStack(const std::shared_ptr<const RailStationEvent>& ev)
    {
        if (ev->pos & RailStationEvent::Pre) {
            if (ev->dir == Direction::Down)
                downFormerLast = ev;
            else
                upLatterLast = ev;
        }
        if (ev->pos & RailStationEvent::Post) {
            if (ev->dir == Direction::Down)
                downLatterLast = ev;
            else
                upFormerLast = ev;
        }
    }

    std::shared_ptr<const RailStationEvent> EventStackTop::preRelatedEvent(const RailStationEvent& ev)const
    {
        // 如果不是单线，则只能是同向那个；如果单线，则返回两个中较新的那个。
        if (*preSingle) {
            int secs1 = downFormerLast->secsTo(ev),
                secs2 = upLatterLast->secsTo(ev);
            if (secs1 == secs2) [[unlikely]] {
                // 相同时优先返回同方向的
                return ev.dir == Direction::Down ? downFormerLast : upLatterLast;
            }
            else if (secs1 < secs2) {
                return downFormerLast;
            }
            else return upLatterLast;
        }
        else {
            return ev.line->dir() == Direction::Down ? downFormerLast : upLatterLast;
        }
    }

    std::shared_ptr<const RailStationEvent> EventStackTop::postRelatedEvent(const RailStationEvent& ev)const
    {
        if (*postSingle) {
            int secs1 = downLatterLast->secsTo(ev),
                secs2 = upFormerLast->secsTo(ev);
            if (secs1 == secs2) [[unlikely]] {
                return ev.dir == Direction::Down ? downLatterLast : upFormerLast;
            }
            else if (secs1 < secs2) {
                return downLatterLast;
            }
            else {
                return upFormerLast;
            }
        }
        else {
            return ev.line->dir() == Direction::Down ? downLatterLast : upFormerLast;
        }
    }

    std::shared_ptr<const RailStationEvent> EventStackTop::preCounterEvent(const RailStationEvent& ev) const
    {
        return postSingle.has_value() ? postRelatedEvent(ev) : nullptr;
    }

    std::shared_ptr<const RailStationEvent> EventStackTop::postCounterEvent(const RailStationEvent& ev) const
    {
        return preSingle.has_value() ? preRelatedEvent(ev) : nullptr;
    }

    bool EventStackTop::hasAvoidEvent(const RailStationEvent* stay, const RailStationEvent* leave)
    {
        if (stay->dir == leave->dir)
            return true;
        if ((stay->dir == Direction::Down && *postSingle) ||
            (stay->dir == Direction::Up && *preSingle))
            return true;
        return false;
    }

}

TrainGapAna::TrainGapAna(Diagram &diagram, const TrainFilterCore *filter):
    diagram(diagram), filter(filter)
{

}

TrainGapAna::TrainGapAna(Diagram& diagram):
    diagram(diagram)
{
}

std::map<TrainGap::GapTypesV2, int> TrainGapAna::globalMinimal(
        std::shared_ptr<Railway> rail) const
{
    std::map<TrainGap::GapTypesV2,int> res{};
    auto events=diagram.stationEventsForRail(rail);
    for(auto _p=events.begin();_p!=events.end();++_p){
        const RailStationEventList& lst=_p->second;
        auto gaps=calTrainGaps(lst, *filter,_p->first);
        TrainGapStatistics stat=countTrainGaps(gaps, _cutSecs);

        for (auto q = stat.begin(); q != stat.end(); ++q) {
            const auto& tp = q->first;
            std::shared_ptr<TrainGap> gap = q->second.begin().operator*();

            // 统计全局最小
            if (auto curmin = res.find(tp); curmin != res.end()) {
                curmin->second = std::min(curmin->second, gap->secs());
            }
            else {
                res.emplace(tp, gap->secs());
            }
        }
    }
    return res;
}

TrainGapList TrainGapAna::calTrainGaps(const RailStationEventList &events, const TrainFilterCore &filter, std::shared_ptr<const RailStation> st) const
{
     // 此版本尝试使用内建的单双线数据。
     // 目前的思路是，考虑在单线基础上，筛选一下事件的相关性。
     TrainGapList res;

     _gapdetail::EventStackTop stk(*st);

     stk.downFormerLast = findLastEvent(events, filter, Direction::Down, RailStationEvent::Pre);
     stk.downLatterLast = findLastEvent(events, filter, Direction::Down, RailStationEvent::Post);
     stk.upFormerLast = findLastEvent(events, filter, Direction::Up, RailStationEvent::Post);
     stk.upLatterLast = findLastEvent(events, filter, Direction::Up, RailStationEvent::Pre);

     // 当前站内车的情况。所有【到达】事件直接压进来。int位记录被踩了多少次。
     // 类似队列的结构：从back插入，从front删除，但不绝对禁止中间删除。
     std::deque<std::pair<std::shared_ptr<const RailStationEvent>, int>> down_in;

     // 对向判定待避的条件：（潜在）“被踩的”那个车的出发方向存在敌对进路。

     // 先预扫描一遍：把0点前进站没出站的存下来 （跨日停站的）
     for (auto _p = events.cbegin(); _p != events.cend(); ++_p) {
         const auto& p = *_p;
         if (!filter.check(p->line->train()))continue;
         if (p->type == TrainEventType::Arrive) {
             down_in.emplace_back(std::make_pair(p, 0));
         }
         else if (p->type == TrainEventType::Depart) {
             std::optional<decltype(down_in)::iterator> pitr{};   //被删除的那个的迭代器
             for (auto itr = down_in.begin(); itr != down_in.end(); ++itr) {
                 if (itr->first->line == p->line) {
                     // 找到当前的进站记录
                     pitr = down_in.erase(itr);
                     break;
                 }
             }
             if (pitr.has_value()) {
                 for (auto itr = down_in.begin(); itr != pitr; ++itr) {
                     if (stk.hasAvoidEvent(itr->first.get(),p.get()))
                         itr->second++;
                 }
             }
         }
         else if (p->type == TrainEventType::CalculatedPass ||
             p->type == TrainEventType::SettledPass) {
             if (p->pos == RailStationEvent::Both) {
                 // 站内所有车被踩一次
                 for (auto& q : down_in) {
                     if (stk.hasAvoidEvent(q.first.get(), p.get()))
                         q.second++;
                 }
             }
         }

     }

     // 注意：pre/post是说绝对位置（里程小端和大端），former latter是说运行方向前后
     for (auto _p = events.cbegin(); _p != events.cend(); ++_p) {
         const auto& p = *_p;
         if (!filter.check(p->line->train())) continue;
         if (p->pos & RailStationEvent::Pre) {
             // 与站前有交集
             // 既然遇到了一个，那么last一定不是空
             // 2022.09.10: 如果出现Pre，那么pre应该是存在的，这么直接*应该不会遭起
             auto lastRelated = stk.preRelatedEvent(*p);
             res.emplace_back(std::make_shared<TrainGap>(lastRelated, p));
             // 2023.06.14: for counter ??
             // 暂时是不行的。因为目前的事件/间隔分析里强烈依赖了站前/站后的标记。
             auto lastCounter = stk.preCounterEvent(*p);
             if (lastCounter && lastCounter != lastRelated) {
                 res.emplace_back(std::make_shared<TrainGap>(lastCounter, p));
             }
         }
         if (p->pos & RailStationEvent::Post) {
             // 与站后有交集
             if (!(p->pos & RailStationEvent::Pre)) {
                 // 这个条件是针对通通的情况，防止重复
                 auto lastRelated = stk.postRelatedEvent(*p);
                 res.emplace_back(std::make_shared<TrainGap>(lastRelated, p));
                 // 2023.06.14: for counter ??
                 auto lastCounter = stk.postCounterEvent(*p);
                 if (lastCounter &&lastCounter != lastRelated) {
                     res.emplace_back(std::make_shared<TrainGap>(lastCounter, p));
                 }
             }
         }

         // 现在：判断进出站情况。只考虑到达出发，以及通过的。
         if (p->type == TrainEventType::Arrive) {
             // 车次到达：进队列
             down_in.emplace_back(std::make_pair(p, 0));
         }
         else if (p->type == TrainEventType::Depart) {
             // 出发：先是所有比它到的早但还没跑掉的，都被踩一次
             std::optional<decltype(down_in)::iterator> pitr{};   //被删除的那个的迭代器
             for (auto itr = down_in.begin(); itr != down_in.end(); ++itr) {
                 if (itr->first->line == p->line) {
                     // 找到当前的进站记录
                     if (itr->second) {
                         // 存在被踩情况
                         res.emplace_back(std::make_shared<TrainGap>(
                             itr->first, p, TrainGap::Avoid, itr->second));
                     }
                     pitr = down_in.erase(itr);
                     break;
                 }
             }
             if (pitr.has_value()) {
                 // 删除实际发生了，所有此前列车都被踩一次
                 // 这个条件是为了防止运行线端点没有到达事件只有出发事件的情况
                 for (auto itr = down_in.begin(); itr != pitr; ++itr) {
                     if (stk.hasAvoidEvent(itr->first.get(), p.get()))
                         itr->second++;
                 }
             }
         }
         else if (p->type == TrainEventType::CalculatedPass ||
             p->type == TrainEventType::SettledPass) {
             if (p->pos == RailStationEvent::Both) {
                 // 站内所有车被踩一次
                 for (auto& q : down_in) {
                     if (stk.hasAvoidEvent(q.first.get(), p.get()))
                         q.second++;
                 }
             }
         }

         stk.updateStack(p);
     }
     return res;
}


TrainGapStatistics TrainGapAna::countTrainGaps(const TrainGapList &gaps, int cutSecs) const
{
    TrainGapStatistics res;
    for (const auto& p: gaps) {
        if (p->secs() < cutSecs)
            continue;
        // 2023.06.15: for API v2
        // 此处对于站前站后都存在的间隔，应当手动分开。注意如果存在这种情况，只考虑同侧间隔。
        auto posLeft = p->gapTypeToPosLeft(p->type);
        auto posRight = p->gapTypeToPosRight(p->type);
        auto posCross = (posLeft & posRight);
        if (posCross == RailStationEvent::Both) {
            // 拆解事件为一个站前一个站后
            auto p_post = std::make_shared<TrainGap>(*p);   // copy construct
            p_post->type &= ~TrainGap::PreMask;
            auto p_pre = std::make_shared<TrainGap>(*p);
            p_pre->type &= ~TrainGap::PostMask;
            res.operator[](p_pre->type).emplace(p_pre);
            res.operator[](p_post->type).emplace(p_post);
            //qDebug() << "Splitted type: " << p_pre->type << ", " << p_post->type << Qt::endl;
        }
        else if (posLeft==RailStationEvent::Both || posRight==RailStationEvent::Both) {
            // 对于一个到一个通这种情况也需要特别对待，否则他们type的hash是不同的。
            auto p_copy = std::make_shared<TrainGap>(*p);
            p_copy->type = TrainGap::setLeftPos(p_copy->type, posCross);
            p_copy->type = TrainGap::setRightPos(p_copy->type, posCross);
        }
        else {
            res.operator[](p->type).emplace(p);
        }
    }
    return res;
}


std::shared_ptr<const RailStationEvent> TrainGapAna::findLastEvent(const RailStationEventList &lst, const TrainFilterCore &filter, const Direction &dir, RailStationEventBase::Positions pos) const
{
    for (auto p = lst.crbegin(); p != lst.crend(); ++p) {
        if (!filter.check((*p)->line->train())) continue;
        if ((*p)->line->dir() == dir && (*p)->pos & pos) {
            return *p;
        }
    }
    return nullptr;
}

