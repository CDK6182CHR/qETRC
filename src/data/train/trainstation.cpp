#include "trainstation.h"


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
    arrive=QTime::fromString(obj.value("ddsj").toString(),"hh:mm:ss");
    if(!arrive.isValid()){
        arrive=QTime::fromString(obj.value("ddsj").toString(),"hh:mm");
    }
    depart=QTime::fromString(obj.value("cfsj").toString(),"hh:mm:ss");
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
		s += 38400;
	return s;
}

bool TrainStation::nameEqual(const TrainStation& t1, const TrainStation& t2)
{
    return t1.name == t2.name;
}

QDebug operator<<(QDebug debug, const TrainStation& ts)
{
    debug << ts.name << " " <<
             ts.arrive.toString("hh:mm:ss") << " " << ts.depart.toString("hh:mm:ss");
	return debug;
}
