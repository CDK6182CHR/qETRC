#include "railstation.h"
#include "data/common/stationname.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

RailStation::RailStation(const StationName& name_,
    double mile_,
    int level_,
    std::optional<double> counter_,
    PassedDirection direction_,
    bool show_,
    bool passenger_, bool freight_) :
    name(name_), mile(mile_), level(level_), counter(counter_),
    y_coeff(std::nullopt), direction(direction_),
    _show(show_), passenger(passenger_), freight(freight_),
    tracks()
{
}

RailStation::RailStation(const QJsonObject &obj)
{
    fromJson(obj);
}

RailStation::RailStation(const RailStation& rs):
    name(rs.name),mile(rs.mile),level(rs.level),counter(rs.counter),
    y_coeff(rs.y_coeff),direction(rs.direction),
    _show(rs._show),passenger(rs.passenger),freight(rs.freight),
    tracks(rs.tracks)
{
    //shared_ptr直接用默认构造
}

void RailStation::fromJson(const QJsonObject &obj)
{
    name=StationName::fromSingleLiteral(obj.value("zhanming").toString());
    mile=obj.value("licheng").toDouble();
    level=obj.value("dengji").toInt(4);
    const auto& c=obj.value("counter");
    if(c.type()==QJsonValue::Null||c.type()==QJsonValue::Undefined){
        counter=std::nullopt;
    }else{
        counter=c.toDouble();
    }
    _show=obj.value("show").toBool(true);
    passenger=obj.value("passenger").toBool(true);
    freight=obj.value("freight").toBool(true);
    const QJsonArray& ar = obj.value("tracks").toArray();
    tracks.clear();
    for (const auto& p : ar) {
        tracks.append(p.toString());
    }
    direction=static_cast<PassedDirection>(obj.value("direction").toInt(3));
}

QJsonObject RailStation::toJson() const
{
    QJsonObject obj;
    obj.insert("zhanming",name.toSingleLiteral());
    obj.insert("licheng",mile);
    obj.insert("dengji",level);
    obj.insert("direction", static_cast<int>(direction));
    if(counter.has_value()){
        obj.insert("counter",counter.value());
    }else{
        obj.insert("counter",QJsonValue(QJsonValue::Null));
    }
    obj.insert("show",_show);
    obj.insert("passenger",passenger);
    obj.insert("freight",freight);
    QJsonArray ar;
    for (const auto& p : tracks) {
        ar.append(QJsonValue(p));
    }
    obj.insert("tracks", ar);
    return obj;
}

QString RailStation::counterStr() const
{
    if(counter.has_value()){
        return QString::number(counter.value(),'f',3);
    }
    else return " ";
}

std::shared_ptr<RailStation> RailStation::downAdjacent()
{
    if(!downNext)
        return std::shared_ptr<RailStation>();
    return downNext->toStation();
}

std::shared_ptr<RailStation> RailStation::upAdjacent()
{
    if(!upNext)
        return std::shared_ptr<RailStation>();
    return upNext->toStation();
}

std::shared_ptr<RailStation> RailStation::downPrevAdjacent()
{
    if (downPrev)
        return downPrev->fromStation();
    else return {};
}

std::shared_ptr<RailStation> RailStation::upPrevAdjacent()
{
    if (upPrev)
        return upPrev->fromStation();
    else return {};
}

std::shared_ptr<RailInterval> RailStation::adjacentIntervalTo(std::shared_ptr<const RailStation> another)
{
    if (downAdjacent() == another)
        return downNext;
    else if (upAdjacent() == another)
        return upNext;
    return nullptr;
}

