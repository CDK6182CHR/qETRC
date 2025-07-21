
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
            p->beginTime.toString(TrainTime::HM) << " -- " <<
            p->endTime.toString(TrainTime::HM) <<
            //'\t' << p->durationMin() <<  // <- 2025.07.21: remove this because we do not want to pass `period_hours` ...
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
            << it->beginTime.toString(TrainTime::HM) << ","
            << it->endTime.toString(TrainTime::HM) << "\n";
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
            TrainTime bt = qeutil::parseTrainTime(sp[2]);
            TrainTime et = qeutil::parseTrainTime(sp[3]);
            if (!bt.isNull() && !et.isNull()) {
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

ForbidNode::ForbidNode(Forbid& forbid, RailInterval& railint, const TrainTime& beginTime_ ,
    const TrainTime& endTime_) :
    RailIntervalNode<ForbidNode, Forbid>(forbid, railint),beginTime(beginTime_),
    endTime(endTime_)
{

}

void ForbidNode::fromJson(const QJsonObject& obj)
{
    beginTime = TrainTime::fromString(obj.value("begin").toString());
    endTime = TrainTime::fromString(obj.value("end").toString());
}

QJsonObject ForbidNode::toJson() const
{
    return QJsonObject({
        {"fazhan",railInterval().fromStationNameLit()},
        {"daozhan",railInterval().toStationNameLit()},
        {"begin",beginTime.toString(TrainTime::HM)},
        {"end",endTime.toString(TrainTime::HM)}
        });
}

ForbidNode& ForbidNode::operator=(const ForbidNode& other)
{
    beginTime = other.beginTime;
    endTime = other.endTime;
    return *this;
}

int ForbidNode::durationSec(int period_hours) const
{
    int x = beginTime.secsTo(endTime);
    if (x < 0) x += period_hours * 3600;
    return x;
}

int ForbidNode::durationMin(int period_hours) const
{
    return durationSec(period_hours) / 60;
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


