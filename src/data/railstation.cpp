#include "railstation.h"


RailStation::RailStation(const StationName &name_,
                         double mile_,
                         int level_,
                         std::optional<double> counter_,
                         PassedDirection direction_):
    name(name_),mile(mile_),level(level_),counter(counter_),
    y_value(-1),direction(direction_),
    show(true),passenger(true),freight(true),
    tracks(QObject::tr(""))
{
}

RailStation::RailStation(const QJsonObject &obj)
{
    fromJson(obj);
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
    tracks=obj.value("tracks").toString("");
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
    obj.insert("tracks",tracks);
    return obj;
}

QString RailStation::counterStr() const
{
    if(counter.has_value()){
        return QString::number(counter.value(),'f',3);
    }
    else return " ";
}
