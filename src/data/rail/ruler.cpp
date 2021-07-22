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
        qDebug() << p->fromStationName() << "->" << p->toStationName() << '\t'
            << p->interval << ", " << p->start << ", " << p->stop << '\t'
            << this << " @ " << &p->railInterval()
            << Qt::endl;
    }
}

int Ruler::totalInterval(std::shared_ptr<RailStation> from, 
    std::shared_ptr<RailStation> to, Direction _dir) const
{
    auto p = from->dirNextInterval(_dir);
    auto node = p->rulerNodeAt(_index);
    int res = 0;
    for (; p; p = p->nextInterval()) {
        res += p->rulerNodeAt(_index)->interval;
        if (p->toStation() == to)
            return res;
    }
    qDebug() << "Ruler::totalInterval: WARNING: invalid arguments, possibly wrong direction "
        << from->name << " -> " << to->name << Qt::endl;
    return -1;
}

bool Ruler::isOrdinateRuler() const
{
    return _railway.get().ordinate().get() == this;
}

std::shared_ptr<Railway> Ruler::clone() const
{
    auto p = _railway.get().cloneBase();
    p->addRulerFrom(*this);
    return p;
}

void Ruler::swap(Ruler& other)
{
    std::swap(_different, other._different);
    std::swap(_name, other._name);
    for (auto n1 = firstDownNode(), n2 = other.firstDownNode();
        n1 && n2; n1 = n1->nextNodeCirc(), n2 = n2->nextNodeCirc()) 
    {
        n1->swap(*n2);
    }
}
