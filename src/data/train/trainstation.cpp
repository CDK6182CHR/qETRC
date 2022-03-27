#include "trainstation.h"
#include "util/utilfunc.h"

#include <QJsonObject>


TrainStation::TrainStation(const StationName& name_, 
    const QTime& arrive_, const QTime& depart_, bool business_,
                           const QString& track_, const QString& note_):
    name(name_),arrive(arrive_),depart(depart_),business(business_),track(track_),
    note(note_)
{
}

TrainStation::TrainStation(const QJsonObject &obj)
{
    fromJson(obj);
}

void TrainStation::fromJson(const QJsonObject &obj)
{
    name=StationName::fromSingleLiteral(obj.value("zhanming").toString());
    //arrive=QTime::fromString(obj.value("ddsj").toString(),"hh:mm:ss");
    arrive = qeutil::parseTime(obj.value("ddsj").toString());
    if(!arrive.isValid()){
        arrive=QTime::fromString(obj.value("ddsj").toString(),"hh:mm");
    }
    //depart=QTime::fromString(obj.value("cfsj").toString(),"hh:mm:ss");
    depart = qeutil::parseTime(obj.value("cfsj").toString());
    if(!depart.isValid()){
        depart=QTime::fromString(obj.value("cfsj").toString(),"hh:mm");
    }
    note=obj.value("note").toString();
}

QJsonObject TrainStation::toJson() const
{
    QJsonObject res{
        {"zhanming",name.toSingleLiteral()},
        {"ddsj",arrive.toString("hh:mm:ss")},
        {"cfsj",depart.toString("hh:mm:ss")},
        {"business",business},
        {"note",note}
    };
    if (!track.isEmpty()) {
        res.insert("track", track);
    }
    return res;
}

int TrainStation::stopSec() const
{
	int s = arrive.secsTo(depart);
    if (s < 0){
        s += 24 * 3600;
    }
	return s;
}

bool TrainStation::nameEqual(const TrainStation& t1, const TrainStation& t2)
{
    return t1.name == t2.name;
}

bool TrainStation::timeInStoppedRange(int msecs) const
{
    int t1 = arrive.msecsSinceStartOfDay(), t2 = depart.msecsSinceStartOfDay();
    if (t2 < t1)t2 += msecsOfADay;
    msecs = (msecs + msecsOfADay) % msecsOfADay;
    //考虑最多一个边界条件
    if (t1 <= msecs && msecs <= t2) 
        return true;
    msecs += msecsOfADay;
    if (t1 <= msecs && msecs <= t2)
        return true;
    return false;
}

bool TrainStation::stopRangeIntersected(const TrainStation& another) const
{
    int xm1 = arrive.msecsSinceStartOfDay(), xm2 = depart.msecsSinceStartOfDay();
    int xh1 = another.arrive.msecsSinceStartOfDay(), xh2 = another.depart.msecsSinceStartOfDay();
    bool flag1 = (xm2 < xm1), flag2 = (xh2 < xh1);
    if (flag1)xm2 += msecsOfADay;
    if (flag2)xh2 += msecsOfADay;
    bool res1 = (std::max(xm1, xh1) <= std::min(xm2, xh2));   //不另加PBC下的比较
    if (res1 || flag1 == flag2) {
        // 如果都加了或者都没加PBC，这就是结果
        return res1;
    }
    //如果只有一边加了PBC，那么应考虑把另一边也加上PBC再试试
    if (flag1) {
        xh1 += msecsOfADay; xh2 += msecsOfADay;
    }
    else {
        xm1 += msecsOfADay; xm2 += msecsOfADay;
    }
    return (std::max(xm1, xh1) <= std::min(xm2, xh2));
}

QString TrainStation::stopString() const
{
    return qeutil::secsToString(stopSec());
}

bool TrainStation::operator==(const TrainStation& other) const
{
    return name == other.name &&
        arrive == other.arrive &&
        depart == other.depart &&
        note == other.note &&
        track == other.track &&
        business == other.business;
}

QString TrainStation::timeStringCompressed() const
{
    if (isStopped()) {
        return QString("%1/%2").arg(arrive.toString("hh:mm:ss"), depart.toString("hh:mm:ss"));
    }
    else {
        return QString("%1/...").arg(arrive.toString("hh:mm:ss"));
    }
}

void TrainStation::updateStopFlag()
{
    if (isStopped()) {
        flag |= Stopped;
    }
    else {
        flag &= ~Stopped;
    }
}

QDebug operator<<(QDebug debug, const TrainStation& ts)
{
    debug << ts.name << " " <<
             ts.arrive.toString("hh:mm:ss") << " " << ts.depart.toString("hh:mm:ss");
	return debug;
}
