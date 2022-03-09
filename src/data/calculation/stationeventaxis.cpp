#include "stationeventaxis.h"
#include "gapconstraints.h"
#include <util/utilfunc.h>
#include <qDebug>

void StationEventAxis::sortEvents()
{
    std::sort(begin(),end(),RailStationEvent::PtrTimeComparator());
}

void StationEventAxis::constructLineMap()
{
    _preEvents.clear();
    _postEvents.clear();
    for (auto ev : *this) {
        if (ev->pos & RailStationEventBase::Pre) {
            _preEvents.emplace(ev->line, ev);
        }
        if (ev->pos & RailStationEventBase::Post) {
            _postEvents.emplace(ev->line, ev);
        }
    }
}

const typename StationEventAxis::line_map_t&
    StationEventAxis::positionEvents(RailStationEventBase::Position pos) const
{
    switch (pos)
    {
    case RailStationEventBase::Pre: return preEvents();
        break;
    case RailStationEventBase::Post:return postEvents();
        break;
    default:
        qDebug() << "StationEventAxis::positionEvents: INVALID pos " << pos << Qt::endl;
        return preEvents();
        break;
    }
}

void StationEventAxis::buildAxis()
{
    sortEvents();
    constructLineMap();
}

void StationEventAxis::insertEvent(std::shared_ptr<RailStationEvent> ev)
{
    //如果有时刻一样的，新的在后面
    auto itr=std::upper_bound(begin(),end(),ev,RailStationEvent::PtrTimeComparator());
    insert(itr,ev);
    if (ev->pos & RailStationEventBase::Pre) {
        _preEvents.emplace(ev->line, ev);
    }
    if (ev->pos & RailStationEventBase::Post) {
        _postEvents.emplace(ev->line, ev);
    }
}

std::shared_ptr<RailStationEvent> StationEventAxis::conflictEvent(
        std::shared_ptr<RailStationEventBase> ev,
        const GapConstraints &constraint) const
{
    int bound=constraint.correlationRange();
    QTime leftBound=ev->time.addSecs(-bound), rightBound=ev->time.addSecs(bound);
    // upper_bound 正好与reverse_iterator配合使用
    auto citr=std::upper_bound(begin(),end(),ev->time,
                              RailStationEvent::PtrTimeComparator());

    // 左侧
    if (leftBound > ev->time){
        // 发生左跨日情况
        for(auto ritr=std::make_reverse_iterator(citr);ritr!=rend();++ritr){
            if (isConflict(*ritr,ev,constraint))
                return *ritr;
        }
        for(auto ritr=rbegin();ritr!=rend();++ritr){
            if ((*ritr)->time < leftBound)
                break;
            if (isConflict(*ritr,ev,constraint))
                return *ritr;
        }
    }else{
        // 不发生左跨日
        for(auto ritr=std::make_reverse_iterator(citr);
            ritr!=rend() && (*ritr)->time >= leftBound; ++ritr){
            if (isConflict(*ritr,ev,constraint))
                return *ritr;
        }
    }

    // 右侧
    if (rightBound < ev->time){
        // 右跨日
        for(auto itr=citr;itr!=end();++itr){
            if(isConflict(ev,*itr,constraint))
                return *itr;
        }
        for(auto itr=begin();itr!=end()&&(*itr)->time<=rightBound;++itr){
            if(isConflict(ev,*itr,constraint))
                return *itr;
        }
    }else{
        for(auto itr=citr;itr!=end()&&(*itr)->time<=rightBound;++itr){
            if(isConflict(ev,*itr,constraint)){
                return *itr;
            }
        }
    }
    return nullptr;
}

bool StationEventAxis::isConflict(std::shared_ptr<RailStationEventBase> left,
                                  std::shared_ptr<RailStationEventBase> right,
                                  const GapConstraints &constraint) const
{
    auto gap_type = TrainGap::gapTypeBetween(left,right, constraint.isSingleLine());
    if (gap_type.has_value()){
        // 构成相干事件，检查间隔是否符合要求。
        // 注意约定正好相等的情况是符合要求（不冲突）的
        int secs=qeutil::secsTo(left->time, right->time);
        if (secs<constraint.at(*gap_type)){
            return true;
        }
    }
    return false;
}
