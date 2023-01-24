#include "graphinterval.h"
#include "data/rail/forbid.h"
#include "data/rail/rulernode.h"
#include "data/rail/ruler.h"


GraphForbidNode::GraphForbidNode(const ForbidNode &node):
    beginTime(node.beginTime),endTime(node.endTime)
{

}

void GraphForbidNode::exportToNode(ForbidNode &node) const
{
    node.beginTime=beginTime;
    node.endTime=endTime;
}

GraphRulerNode::GraphRulerNode(const RulerNode &node):
    name(node.dataHead().name()),
    interval(node.interval),start(node.start),stop(node.stop)
{

}

void GraphRulerNode::exportToNode(RulerNode &node) const
{
    node.interval=interval;
    node.start=start;
    node.stop=stop;
}

GraphInterval::GraphInterval(const QString &name, const RailInterval &interval):
    railName(name),dir(interval.direction()),mile(interval.mile())
{
    foreach(const auto& fn, interval._forbidNodes){
        forbidNodes.push_back(*fn);
    }
    foreach(const auto& rn, interval._rulerNodes){
        if (!rn->isNull())
            rulerNodes.push_back(*rn);
    }
}

GraphInterval::GraphInterval(const QString& name, Direction dir, double mile):
    railName(name),dir(dir),mile(mile)
{
}

double GraphInterval::getMile(const GraphInterval &inter)
{
    return inter.mile;
}
