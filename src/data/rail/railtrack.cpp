#include "railtrack.h"
#include "util/utilfunc.h"
#include "data/diagram/trainline.h"

TrackItem::TrackItem(const QString &title, const StationName &stationName,
          const QTime &beginTime, const QTime &endTime,
          StayType type, std::shared_ptr<TrainLine> line, QString specTrack):
    title(title),stationName(stationName),beginTime(beginTime),endTime(endTime),
    type(type),line(line),dir(line->dir()),specTrack(specTrack)
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
        timeString = beginTime.toString("hh:mm:ss");
    }
    else {
        timeString = QString("%1 - %2").arg(beginTime.toString("hh:mm:ss"),
            endTime.toString("hh:mm:ss"));
    }
    text.append(QString(" %1 %2 %3 %4 %5").arg(title, DirFunc::dirToString(dir), typeString(),
        timeString, stationName.toSingleLiteral()));
    return text;
}

bool TrackItem::isStopped() const
{
    return beginTime.secsTo(endTime) != 0;
}

std::pair<QTime, QTime> TrackItem::occpiedRange() const
{
    if (isStopped()) {
        return std::make_pair(beginTime, endTime);
    }
    else {
        return std::make_pair(beginTime.addSecs(-30), endTime.addSecs(30));
    }
}

const QTime TrackOccupy::LEFT = QTime(0, 0, 0, 0);
const QTime TrackOccupy::RIGHT = QTime(23, 59, 59, 999);

const QTime& TrackOccupy::fromTime() const
{
    return fromLeft ? LEFT : item->beginTime;
}

const QTime& TrackOccupy::toTime() const
{
    return toRight ? RIGHT : item->endTime;
}

std::pair<QTime, QTime> TrackOccupy::occupiedRange() const
{
    auto p=item->occpiedRange();
    if (fromLeft)
        p.first=LEFT;
    if (toRight)
        p.second=RIGHT;
    return p;
}

Track::const_iterator Track::occupiedInRange(const QTime& tm1, const QTime& tm2) const
{
    if (tm1 > tm2) {
        // 跨日的，直接拆开算，安全一些
        if (auto itr = occupiedInRange(tm1, TrackOccupy::RIGHT); itr != end()) {
            return itr;
        }
        else return occupiedInRange(TrackOccupy::LEFT, tm2);
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
        if (std::max(itr->fromTime(), tm1) < std::min(itr->toTime(), tm2))
            break;
    }
    return itr;
}

bool Track::isOccupied(const QTime& tm1, const QTime& tm2) const
{
    return occupiedInRange(tm1, tm2) != end();
}

Track::const_iterator Track::conflictItem(const std::shared_ptr<TrackItem>& item,
    int sameSplitSecs, int oppsiteSplitSecs)const
{
    auto [start, end] = item->occpiedRange();
    QTime startSame = start.addSecs(-sameSplitSecs), endSame = end.addSecs(sameSplitSecs);
    QTime startOpps = start.addSecs(-oppsiteSplitSecs), endOpps = end.addSecs(oppsiteSplitSecs);

    for (auto p = cbegin(); p != cend(); ++p) {
        if (p->item->dir == item->dir) {
            // 同向
            if (qeutil::timeRangeIntersectedExcl(p->fromTime(), p->toTime(),
                startSame, endSame)) {
                return p;
            }
        }
        else {
            // 反向
            if (qeutil::timeRangeIntersectedExcl(p->fromTime(), p->toTime(),
                startOpps, endOpps)) {
                return p;
            }
        }
    }
    return cend();
}

bool Track::isIdleFor(const std::shared_ptr<TrackItem>& item, int sameSplitSecs, int oppsiteSplitSecs) const
{
    return conflictItem(item, sameSplitSecs, oppsiteSplitSecs) == end();
}

bool Track::isIdleForDouble(const std::shared_ptr<TrackItem>& item, int sameSplitSecs) const
{
    auto [start, end] = item->occpiedRange();
    return !isOccupied(start.addSecs(-sameSplitSecs), end.addSecs(sameSplitSecs));
}

void Track::addItem(const std::shared_ptr<TrackItem>& item)
{
    auto [start, end] = item->occpiedRange();
    if (start <= end) {
        // 正常添加
        emplace(item, false, false);
    }
    else {
        emplace(item, true, false);
        emplace(item, false, true);
    }
}


