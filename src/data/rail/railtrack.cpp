#include "railtrack.h"
#include "util/utilfunc.h"
#include "data/diagram/trainline.h"

TrackItem::TrackItem(const QString &title, const StationName &stationName,
          const TrainTime &beginTime, const TrainTime &endTime,
          StayType type, std::shared_ptr<TrainLine> line, QString specTrack,
                     const train_st_t& st1, const train_st_t& st2):
    title(title),stationName(stationName),beginTime(beginTime),endTime(endTime),
    type(type),line(line),dir(line->dir()),specTrack(specTrack),
    trainStation1(st1),trainStation2(st2)
{

}

QString TrackItem::typeString() const
{
    switch (type)
    {
    case TrackItem::Stop: return QObject::tr("停车");
        break;
    case TrackItem::Pass: return QObject::tr("通过");
        break;
    case TrackItem::Link: return QObject::tr("交路接续");
        break;
    case TrackItem::Departure:return QObject::tr("始发");
        break;
    case TrackItem::Destination:return QObject::tr("终到");
        break;
    default:return {};
           break;
    }
}

QString TrackItem::toString() const
{
    QString text;
    if (specTrack.isEmpty())
        text = QObject::tr("[推定股道]");
    else
        text = QObject::tr("[图定股道]");
    QString timeString;
    if (beginTime == endTime) {
        timeString = beginTime.toString(TrainTime::HMS);
    }
    else {
        timeString = QString("%1 - %2").arg(beginTime.toString(TrainTime::HMS),
            endTime.toString(TrainTime::HMS));
    }
    text.append(QString(" %1 %2 %3 %4 %5").arg(title, DirFunc::dirToString(dir), typeString(),
        timeString, stationName.toSingleLiteral()));
    return text;
}

bool TrackItem::isStopped() const
{
    return beginTime.secsTo(endTime) != 0;
}

std::pair<TrainTime, TrainTime> TrackItem::occpiedRange(int period_hours) const
{
    if (isStopped()) {
        return std::make_pair(beginTime, endTime);
    }
    else {
        return std::make_pair(beginTime.addSecs(-30, period_hours), endTime.addSecs(30, period_hours));
    }
}

const TrainTime TrackOccupy::LEFT = TrainTime(0, 0, 0);
const TrainTime TrackOccupy::RIGHT = TrainTime(23, 59, 59);

const TrainTime& TrackOccupy::fromTime() const
{
    return fromLeft ? LEFT : item->beginTime;
}

const TrainTime& TrackOccupy::toTime() const
{
    return toRight ? RIGHT : item->endTime;
}

std::pair<TrainTime, TrainTime> TrackOccupy::occupiedRange(int period_hours) const
{
    auto p=item->occpiedRange(period_hours);
    if (fromLeft)
        p.first=LEFT;
    if (toRight)
        p.second=RIGHT;
    return p;
}

Track::const_iterator Track::occupiedInRange(const TrainTime& tm1, const TrainTime& tm2, int period_hours) const
{
    if (tm1 > tm2) {
        // 跨日的，直接拆开算，安全一些
        if (auto itr = occupiedInRange(tm1, TrackOccupy::RIGHT, period_hours); itr != end()) {
            return itr;
        }
        else return occupiedInRange(TrackOccupy::LEFT, tm2, period_hours);
    }

    // 现在 保证不发生跨日情况
    auto itr = lower_bound(tm1);
    if (itr == end()) return itr;
    // 本来应该遍历到开始时刻晚于所给的tm2；但因为开始时刻并没有保证，直接遍历到结尾好了。
    for (; itr != end(); ++itr) {
        // 其实都可以不用这个，因为
        //if (qeutil::timeRangeIntersected(itr->fromTime(), itr->toTime(), tm1, tm2))
        //    break;
        // 这里无需考虑PBC。直接比较范围都行

        // 2024.03.22: we should use occupiedRange()
        auto [p_start, p_end] = itr->occupiedRange(period_hours);
        if (std::max(p_start, tm1) < std::min(p_end, tm2))
            break;
    }
    return itr;
}

bool Track::isOccupied(const TrainTime& tm1, const TrainTime& tm2, int period_hours) const
{
    return occupiedInRange(tm1, tm2, period_hours) != end();
}

Track::const_iterator Track::conflictItem(const std::shared_ptr<TrackItem>& item,
    int sameSplitSecs, int oppsiteSplitSecs, int period_hours)const
{
    auto [start, end] = item->occpiedRange(period_hours);
    TrainTime startSame = start.addSecs(-sameSplitSecs, period_hours), endSame = end.addSecs(sameSplitSecs, period_hours);
    TrainTime startOpps = start.addSecs(-oppsiteSplitSecs, period_hours), endOpps = end.addSecs(oppsiteSplitSecs, period_hours);

    for (auto p = cbegin(); p != cend(); ++p) {
        // 2024.03.22: for proper treatment of passed train, here we should use occupiedRange(), not fromTime/endTime
        auto [p_start, p_end] = p->occupiedRange(period_hours);
        if (p->item->dir == item->dir) {
            // 同向
            if (qeutil::timeRangeIntersectedExcl(p_start, p_end,
                startSame, endSame, period_hours)) {
                return p;
            }
        }
        else {
            // 反向
            if (qeutil::timeRangeIntersectedExcl(p_start, p_end,
                startOpps, endOpps, period_hours)) {
                return p;
            }
        }
    }
    return cend();
}

bool Track::isIdleFor(const std::shared_ptr<TrackItem>& item, int sameSplitSecs, int oppsiteSplitSecs, int period_hours) const
{
    return conflictItem(item, sameSplitSecs, oppsiteSplitSecs, period_hours) == end();
}

bool Track::isIdleForDouble(const std::shared_ptr<TrackItem>& item, int sameSplitSecs, int period_hours) const
{
    auto [start, end] = item->occpiedRange(period_hours);
    return !isOccupied(start.addSecs(-sameSplitSecs, period_hours), end.addSecs(sameSplitSecs, period_hours), period_hours);
}

void Track::addItem(const std::shared_ptr<TrackItem>& item, int period_hours)
{
    auto [start, end] = item->occpiedRange(period_hours);
    if (start <= end) {
        // 正常添加
        emplace(item, false, false);
    }
    else {
        emplace(item, true, false);
        emplace(item, false, true);
    }
}


