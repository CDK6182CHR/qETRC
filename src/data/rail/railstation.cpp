﻿#include "railstation.h"


RailStation::RailStation(const StationName &name_,
                         double mile_,
                         int level_,
                         std::optional<double> counter_,
                         PassedDirection direction_):
    name(name_),mile(mile_),level(level_),counter(counter_),
    y_value(-1),direction(direction_),
    show(true),passenger(true),freight(true),
    tracks()
{
}

RailStation::RailStation(const QJsonObject &obj)
{
    fromJson(obj);
}

RailStation::RailStation(const RailStation& rs):
    name(rs.name),mile(rs.mile),level(rs.level),counter(rs.counter),
    y_value(rs.y_value),direction(rs.direction),
    show(rs.show),passenger(rs.passenger),freight(rs.freight),
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
    y_value=obj.value("y_value").toDouble(-1);
    show=obj.value("show").toBool(true);
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
    if(counter.has_value()){
        obj.insert("counter",counter.value());
    }else{
        obj.insert("connter",QJsonValue(QJsonValue::Null));
    }
    obj.insert("y_value",y_value);
    obj.insert("show",show);
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