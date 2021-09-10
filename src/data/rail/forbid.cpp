
#include "forbid.h"
#include <QDebug>
#include <QJsonArray>

Forbid::Forbid(Railway& railway, bool different, int index) :
    RailIntervalData<ForbidNode, Forbid>(railway, different, index),
    downShow(false),upShow(false)
{

}


void Forbid::toggleDirShow(Direction dir)
{
    switch (dir)
    {
    case Direction::Down: downShow = !downShow;
        break;
    case Direction::Up:upShow = !upShow;
        break;
    default:
        break;
    }
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

std::shared_ptr<Railway> Forbid::clone() const
{
    auto r = railway().cloneBase();
    r->addForbidFrom(*this);
    return r;
}

QString Forbid::name() const
{
    switch(index()){
    case 0:return QObject::tr("综合维修");
    case 1:return QObject::tr("综合施工");
    default:return QObject::tr("天窗%1").arg(index());
    }
}

void Forbid::swap(Forbid& other)
{
    RailIntervalData::swap(other);
    std::swap(downShow, other.downShow);
    std::swap(upShow, other.upShow);
}

ForbidNode::ForbidNode(Forbid& forbid, RailInterval& railint, const QTime& beginTime_ ,
    const QTime& endTime_) :
    RailIntervalNode<ForbidNode, Forbid>(forbid, railint),beginTime(beginTime_),
    endTime(endTime_)
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

ForbidNode& ForbidNode::operator=(const ForbidNode& other)
{
    beginTime = other.beginTime;
    endTime = other.endTime;
    return *this;
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

void ForbidNode::swap(ForbidNode& other)
{
    std::swap(beginTime, other.beginTime);
    std::swap(endTime, other.endTime);
}


