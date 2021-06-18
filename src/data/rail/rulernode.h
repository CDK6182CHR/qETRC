#ifndef RULERNODE_H
#define RULERNODE_H

#include <memory>
#include "railintervaldata.hpp"

#include <QJsonObject>

class Ruler;
class RailInterval;

class RulerNode:
        public RailIntervalNode<RulerNode,Ruler>
{
public:
    int interval,start,stop;
    RulerNode(Ruler& ruler,RailInterval& railint,
              int interval=0,int start=0,int stop=0);

    //需要时实现
    RulerNode(const RulerNode&)=delete;
    RulerNode(RulerNode&&)=default;
    RulerNode& operator=(const RulerNode&)=delete;
    RulerNode& operator=(RulerNode&&)=delete;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
};

#endif // RULERNODE_H
