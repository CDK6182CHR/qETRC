#include "graphinterval.h"
#include "data/rail/forbid.h"
#include "data/rail/rulernode.h"
#include "data/rail/ruler.h"


GraphForbidNode::GraphForbidNode(const ForbidNode &node):
    beginTime(node.beginTime),endTime(node.endTime)
{

}

GraphRulerNode::GraphRulerNode(const RulerNode &node):
    name(node.dataHead().name()),
    interval(node.interval),start(node.start),stop(node.stop)
{

}

GraphInterval::GraphInterval(const QString &name, const RailInterval &interval):
    railName(name),dir(interval.direction())
{
    foreach(const auto& fn, interval._forbidNodes){
        forbidNodes.push_back(*fn);
    }
    foreach(const auto& rn, interval._rulerNodes){
        rulerNodes.push_back(*rn);
    }
}
