#include "trainstation.h"
#include "util/utilfunc.h"


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
    return QJsonObject{
        {"zhanming",name.toSingleLiteral()},
        {"ddsj",arrive.toString("hh:mm:ss")},
        {"cfsj",depart.toString("hh:mm:ss")},
        {"note",note}
    };
}

int TrainStation::stopSec() const
{
	int s = arrive.secsTo(depart);
    if (s < 0)
        s += 24 * 3600;
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
    if (xm2 < xm1)xm2 += msecsOfADay;
    if (xh2 < xh1)xh2 += msecsOfADay;
    return std::max(xm1, xh1) <= std::min(xm2, xh2);
}

QDebug operator<<(QDebug debug, const TrainStation& ts)
{
    debug << ts.name << " " <<
             ts.arrive.toString("hh:mm:ss") << " " << ts.depart.toString("hh:mm:ss");
	return debug;
}
