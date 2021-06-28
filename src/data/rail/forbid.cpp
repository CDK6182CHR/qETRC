
#include "forbid.h"
#include <QDebug>

Forbid::Forbid(Railway& railway, bool different, int index) :
    RailIntervalData<ForbidNode, Forbid>(railway, different, index),
    downShow(false),upShow(false)
{

}

QJsonObject Forbid::toJson() const
{
    QJsonObject obj{
        {"different",different()},
        {"downShow",downShow},
        {"upShow",upShow}
    };
    QJsonArray ar;
    for (auto p = firstDownNode(); p; p = p->nextNodeCirc()) {
        if (!p->isNull())
            ar.append(p->toJson());
    }
    obj.insert("nodes", ar);
    return obj;
}

void Forbid::_show() const
{
    qDebug() << "Forbid index " << index() << Qt::endl;
    for (auto p = firstDownNode(); p; p = p->nextNodeCirc()) {
        qDebug() << p->railInterval() << '\t' <<
            p->beginTime.toString("hh:mm") << " -- " <<
            p->endTime.toString("hh:mm") <<
            '\t' << p->durationMin() <<
            Qt::endl;
    }
}

ForbidNode::ForbidNode(Forbid& forbid, RailInterval& railint) :
    RailIntervalNode<ForbidNode, Forbid>(forbid, railint)
{

}

void ForbidNode::fromJson(const QJsonObject& obj)
{
    beginTime = QTime::fromString(obj.value("begin").toString(), "hh:mm");
    endTime = QTime::fromString(obj.value("end").toString(), "hh:mm");
}

QJsonObject ForbidNode::toJson() const
{
    return QJsonObject({
        {"fazhan",railInterval().fromStationNameLit()},
        {"daozhan",railInterval().toStationNameLit()},
        {"begin",beginTime.toString("hh:mm")},
        {"end",endTime.toString("hh:mm")}
        });
}

int ForbidNode::durationSec() const
{
    int x = beginTime.secsTo(endTime);
    if (x < 0) x += 24 * 3600;
    return x;
}

int ForbidNode::durationMin() const
{
    return durationSec() / 60;
}


