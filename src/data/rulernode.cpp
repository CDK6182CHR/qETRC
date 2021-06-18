#include "rulernode.h"

#include "ruler.h"
#include "railinterval.h"
#include "railstation.h"


RulerNode::RulerNode(Ruler &ruler, RailInterval &railint,
                     int interval_, int start_, int stop_):
    _ruler(ruler),_railint(railint),interval(interval_),start(start_),stop(stop_)
{

}

std::shared_ptr<const RulerNode> RulerNode::nextNode() const
{
    auto t=_railint.nextInterval();
    if(t)
        return t->rulerNodeAt(_ruler.index());
    else
        return std::shared_ptr<const RulerNode>();
}

std::shared_ptr<RulerNode> RulerNode::nextNodeCirc()
{
    auto t=nextNode();
    if(!t&&_ruler.different()&& isDownInterval()){
        return _ruler.firstUpNode();
    }
    else
        return t;
}

std::shared_ptr<const RulerNode> RulerNode::nextNodeCirc()const
{
    auto t=nextNode();
    if(!t&&_ruler.different()&&isDownInterval()){
        return _ruler.firstUpNode();
    }
    else
        return t;
}

bool RulerNode::isDownInterval() const
{
    return _railint.isDown();
}

const StationName &RulerNode::fromStationName() const
{
    return railInterval().fromStation()->name;
}

const StationName &RulerNode::toStationName() const
{
    return railInterval().toStation()->name;
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

std::shared_ptr<RulerNode> RulerNode::nextNode()
{
    auto t=_railint.nextInterval();
    if(t)
        return t->rulerNodeAt(_ruler.index());
    else
        return std::shared_ptr<RulerNode>();
}
