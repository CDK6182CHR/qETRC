#include "ruler.h"

#include "railway.h"
#include "rulernode.h"
#include "railintervaldata.hpp"
#include <QDebug>

Ruler::Ruler(Railway &railway, const QString &name, bool different, int index):
    RailIntervalData<RulerNode,Ruler>(railway,different,index),
    _name(name)
{

}


QJsonObject Ruler::toJson() const
{
    QJsonObject obj({
        {"name",_name},
        {"different",different()}
        });
    QJsonArray nodes;
    auto p = firstDownNode();
    for (; p; p = p->nextNodeCirc()) {
        nodes.append(p->toJson());
    }
    obj.insert("nodes", nodes);
    return obj;
}

void Ruler::show() const
{
    qDebug()<<"Ruler "<<_name<<", diff: "<<different()<<  Qt::endl;
    auto p=firstDownNode();
    for(;p;p=p->nextNodeCirc()){
        qDebug()<<p->fromStationName()<<"->"<<p->toStationName()<<'\t'
               <<p->interval<<", "<<p->start<<", "<<p->stop<<Qt::endl;
    }
}
