#include "rulernode.h"

#include "ruler.h"
#include "railinterval.h"
#include "railstation.h"
#include "railintervaldata.hpp"


RulerNode::RulerNode(Ruler &ruler, RailInterval &railint,
                     int interval_, int start_, int stop_):
    RailIntervalNode<RulerNode,Ruler>(ruler,railint),
    interval(interval_),start(start_),stop(stop_)
{

}

void RulerNode::fromJson(const QJsonObject &obj)
{
    interval=obj.value("interval").toInt();
    start=obj.value("start").toInt();
    stop=obj.value("stop").toInt();
}

QJsonObject RulerNode::toJson() const
{
    return QJsonObject ({
        {"fazhan",fromStationName().toSingleLiteral()},
        {"daozhan",toStationName().toSingleLiteral()},
        {"interval",interval},
        {"start",start},
        {"stop",stop}
    });
}

QString RulerNode::intervalString() const
{
    if (isNull()) 
        return "NA";
    else 
        return QString::asprintf("%d:%02d", interval / 60, interval % 60);
}


