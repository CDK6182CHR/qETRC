﻿
#include "forbid.h"
#include <QDebug>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include "util/utilfunc.h"

Forbid::Forbid(std::weak_ptr<Railway> railway, bool different, int index) :
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

bool Forbid::toCsv(const QString& filename) const
{
    QFile file(filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qWarning() << "Open file " << filename << " failed";
        return false;
    }
    QTextStream sout(&file);

    // 发站，到站，起始，结束
    for (auto it = firstDownNode(); it; it = it->nextNodeCirc()) {
        sout << it->fromStationName().toSingleLiteral() << ","
            << it->toStationName().toSingleLiteral() << ","
            << it->beginTime.toString("hh:mm") << ","
            << it->endTime.toString("hh:mm") << "\n";
    }
    file.close();
    return true;
}

int Forbid::fromCsv(const QString& filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qWarning() << "Open file " << filename << " failed";
        return 0;
    }
    QTextStream sin(&file);

    int cnt = 0;
    int n = 0;
    while (!sin.atEnd()) {
        QString line = sin.readLine();
        n++;
        if (line.isEmpty()) continue;

        auto sp = line.split(",");
        if (sp.size() != 4) {
            qWarning() << "Invalid row data " << sp;
            if (sp.size() < 4) continue;
        }

        const auto& from = sp[0], & to = sp[1];
        auto node = this->getNode(from, to);
        if (!node) {
            qWarning() << "Invalid interval " << from << " -> " << to;
        }
        else {
            QTime bt = qeutil::parseTime(sp[2]);
            QTime et = qeutil::parseTime(sp[3]);
            if (bt.isValid() && et.isValid()) {
                node->beginTime = bt;
                node->endTime = et;
                cnt++;
            }
            else {
                qWarning() << "Invalid time encountered at row " << n << ": " << line;
            }
        }
    }
    file.close();
    return cnt;
}

std::shared_ptr<Railway> Forbid::clone() const
{
    auto r = railway()->cloneBase();
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

void ForbidNode::mergeWith(const ForbidNode& other, bool cover)
{
    if (isNull() || (cover && !other.isNull()))
        *this = other;
}

bool ForbidNode::operator==(const ForbidNode &rhs) const
{
    return beginTime==rhs.beginTime && endTime==rhs.endTime;
}

bool ForbidNode::operator!=(const ForbidNode &rhs) const
{
    return beginTime!=rhs.beginTime || endTime!=rhs.endTime;
}


