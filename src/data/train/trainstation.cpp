#include "trainstation.h"
#include "util/utilfunc.h"

#include <QJsonObject>


TrainStation::TrainStation(const StationName& name_, 
    const TrainTime& arrive_, const TrainTime& depart_, bool business_,
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
    arrive = qeutil::parseTrainTime(obj.value("ddsj").toString());
    if(arrive.isNull()){
        arrive=TrainTime::fromString(obj.value("ddsj").toString());
    }
    //depart=QTime::fromString(obj.value("cfsj").toString(),"hh:mm:ss");
    depart = qeutil::parseTrainTime(obj.value("cfsj").toString());
    if(depart.isNull()){
        depart=TrainTime::fromString(obj.value("cfsj").toString());
    }
    business = obj.value("business").toBool(false);   // 2023.01.13: default false
    track = obj.value("track").toString();
    note = obj.value("note").toString({});   // 2023.01.13: default false
}

QJsonObject TrainStation::toJson() const
{
    QJsonObject res{
        {"zhanming",name.toSingleLiteral()},
        {"ddsj",arrive.toString(TrainTime::HMS)},
        {"cfsj",depart.toString(TrainTime::HMS)},
        // 2023.01.13: use default to reduce file size
        //{"business",business},
        //{"note",note}
    };
    if (!track.isEmpty()) {
        res.insert("track", track);
    }
    if (!note.isEmpty()) {
        res.insert("note", note);
    }
    if (business) {
        res.insert("business", business);
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

bool TrainStation::timeInStoppedRange(int secs, int period_hour) const
{
    int t1 = arrive.secondsSinceStart(), t2 = depart.secondsSinceStart();
    int periodSecs = period_hour * 3600;
    if (t2 < t1)t2 += periodSecs;
    secs = (secs + periodSecs) % periodSecs;
    //考虑最多一个边界条件
    if (t1 <= secs && secs <= t2) 
        return true;
    secs += periodSecs;
    if (t1 <= secs && secs <= t2)
        return true;
    return false;
}

bool TrainStation::stopRangeIntersected(const TrainStation& another, int period_hour) const
{
    const int periodSecs = period_hour * 3600;
    int xm1 = arrive.secondsSinceStart(), xm2 = depart.secondsSinceStart();
    int xh1 = another.arrive.secondsSinceStart(), xh2 = another.depart.secondsSinceStart();
    bool flag1 = (xm2 < xm1), flag2 = (xh2 < xh1);
    if (flag1)xm2 += periodSecs;
    if (flag2)xh2 += periodSecs;
    bool res1 = (std::max(xm1, xh1) <= std::min(xm2, xh2));   //不另加PBC下的比较
    if (res1 || flag1 == flag2) {
        // 如果都加了或者都没加PBC，这就是结果
        return res1;
    }
    //如果只有一边加了PBC，那么应考虑把另一边也加上PBC再试试
    if (flag1) {
        xh1 += periodSecs; xh2 += periodSecs;
    }
    else {
        xm1 += periodSecs; xm2 += periodSecs;
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
        return QString("%1/%2").arg(arrive.toString(TrainTime::HMS), depart.toString(TrainTime::HMS));
    }
    else {
        return QString("%1/...").arg(arrive.toString(TrainTime::HMS));
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

bool TrainStation::ltArrive(const TrainStation& lhs, const TrainStation& rhs)
{
    return lhs.arrive < rhs.arrive;
}

bool TrainStation::ltDepart(const TrainStation& lhs, const TrainStation& rhs)
{
    return lhs.depart < rhs.depart;
}

QDebug operator<<(QDebug debug, const TrainStation& ts)
{
    debug << ts.name << " " <<
             ts.arrive.toString(TrainTime::HMS) << " " << ts.depart.toString(TrainTime::HMS);
	return debug;
}
