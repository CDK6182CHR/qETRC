#include "ruler.h"

#include "railway.h"
#include <QDebug>

Ruler::Ruler(Railway &railway, const QString &name, bool different, int index):
    _railway(railway),_name(name),_different(different),_index(index)
{

}

std::shared_ptr<RulerNode> Ruler::firstDownNode()
{
    auto t = _railway.firstDownInterval();
    if(t)
        return t->rulerNodeAt(_index);
    else
        return std::shared_ptr<RulerNode>();
}

std::shared_ptr<RulerNode> Ruler::firstUpNode()
{
    auto t = _railway.firstUpInterval();
    if(t)
        return t->rulerNodeAt(_index);
    else
        return std::shared_ptr<RulerNode>();
}

std::shared_ptr<const RulerNode> Ruler::firstDownNode() const
{
    auto t = _railway.firstDownInterval();
    if(t)
        return t->rulerNodeAt(_index);
    else
        return std::shared_ptr<RulerNode>();
}

std::shared_ptr<const RulerNode> Ruler::firstUpNode() const
{
    auto t = _railway.firstUpInterval();
    if(t)
        return t->rulerNodeAt(_index);
    else
        return std::shared_ptr<RulerNode>();
}

QJsonObject Ruler::toJson() const
{
    QJsonObject obj({
        {"name",_name},
        {"different",_different}
        });
    QJsonArray nodes;
    auto p = firstDownNode();
    for (; p; p = p->nextNodeCirc()) {
        nodes.append(p->toJson());
    }
    obj.insert("nodes", nodes);
    return obj;
}

std::shared_ptr<RulerNode> Ruler::getNode(const StationName &from, const StationName &to)
{
    auto t = _railway.findInterval(from, to);
    if (!t && !_different)
        t = _railway.findInterval(to, from);
    if (t)
        return t->rulerNodeAt(_index);
    else
        return std::shared_ptr<RulerNode>();
}

std::shared_ptr<const RulerNode> Ruler::getNode(const StationName &from, const StationName &to) const
{
    auto t=_railway.findInterval(from,to);
    if(!t&&!_different)
        t=_railway.findInterval(to,from);
    if(t)
        return t->rulerNodeAt(_index);
    else
        return std::shared_ptr<RulerNode>();
}

void Ruler::show() const
{
    qDebug()<<"Ruler "<<_name<<", diff: "<<_different<<  Qt::endl;
    auto p=firstDownNode();
    for(;p;p=p->nextNodeCirc()){
        qDebug()<<p->fromStationName()<<"->"<<p->toStationName()<<'\t'
               <<p->interval<<", "<<p->start<<", "<<p->stop<<Qt::endl;
    }
}
