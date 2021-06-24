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

int Ruler::totalInterval(std::shared_ptr<RailStation> from, 
    std::shared_ptr<RailStation> to, Direction dir) const
{
    auto p = from->dirNextInterval(dir);
    auto node = p->rulerNodeAt(_index);
    int res = node->interval;
    for (; p; p = p->nextInterval()) {
        res += p->rulerNodeAt(_index)->interval;
        if (p->toStation() == to)
            return res;
    }
    qDebug() << "Ruler::totalInterval: WARNING: invalid arguments, possibly wrong direction "
        << from->name << " -> " << to->name << Qt::endl;
    return -1;
}
