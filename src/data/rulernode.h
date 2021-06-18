#ifndef RULERNODE_H
#define RULERNODE_H

#include <memory>
#include "stationname.h"

#include <QJsonObject>

class Ruler;
class RailInterval;

class RulerNode
{
    Ruler& _ruler;
    RailInterval& _railint;
public:
    int interval,start,stop;
    RulerNode(Ruler& ruler,RailInterval& railint,
              int interval=0,int start=0,int stop=0);

    //需要时实现
    RulerNode(const RulerNode&)=delete;
    RulerNode(RulerNode&&)=default;
    RulerNode& operator=(const RulerNode&)=delete;
    RulerNode& operator=(RulerNode&&)=delete;

    const RailInterval& railInterval()const{
        return _railint;
    }
    RailInterval& railInterval(){
        return _railint;
    }

    std::shared_ptr<RulerNode> nextNode();
    std::shared_ptr<const RulerNode> nextNode()const;

    /*
     * 同时允许上下行的迭代
     */
    std::shared_ptr<RulerNode> nextNodeCirc();
    std::shared_ptr<const RulerNode> nextNodeCirc()const;

    bool isDownInterval()const;

    const StationName& fromStationName()const;
    const StationName& toStationName()const;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
};

#endif // RULERNODE_H
