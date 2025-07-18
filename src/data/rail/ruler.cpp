#include "ruler.h"

#include "railway.h"
#include "rulernode.h"
#include "railintervaldata.hpp"
#include "data/diagram/trainadapter.h"
#include "util/utilfunc.h"
#include "data/train/trainstation.h"
#include <QDebug>
#include <QJsonArray>
#include <QFile>

Ruler::Ruler(std::weak_ptr<Railway> railway, const QString &name, bool different, int index):
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

bool Ruler::toCsv(const QString& filename) const
{
    QFile file(filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qWarning() << "Open file " << filename << " failed";
        return false;
    }
    QTextStream sout(&file);

    // 发站，到站，通通，起，停
    for (auto it = firstDownNode(); it; it = it->nextNodeCirc()) {
        sout << it->fromStationName().toSingleLiteral() << ","
            << it->toStationName().toSingleLiteral() << ","
            << it->interval << ","
            << it->start << ","
            << it->stop << "\n";
    }
    file.close();

    return true;
}

int Ruler::fromCsv(const QString& filename)
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
        if (sp.size() != 5) {
            qWarning() << "Invalid row data " << sp;
            if (sp.size() < 5) continue;
        }

        const auto& from = sp[0], & to = sp[1];
        auto node = this->getNode(from, to);
        if (!node) {
            qWarning() << "Invalid interval " << from << " -> " << to;
        }
        else {
            int interval = sp[2].toInt();
            int start = sp[3].toInt();
            int stop = sp[4].toInt();
            if (!interval) {
                qWarning() << "Zero interval time at row " << n << ": " << line;
            }
            node->interval = interval;
            node->start = start;
            node->stop = stop;
            cnt++;
        }
    }
    file.close();
    return cnt;
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
    int res = 0;
    for (; p; p = p->nextInterval()) {
        auto node = p->rulerNodeAt(_index);
        if (node->isNull())return -1;
        res += node->interval;
        if (p->toStation() == to)
            return res;
    }
    qDebug() << "Ruler::totalInterval: WARNING: invalid arguments, possibly wrong direction "
        << from->name << " -> " << to->name << Qt::endl;
    return -1;
}

bool Ruler::isOrdinateRuler() const
{
    return railway()-> ordinate().get() == this;
}

std::shared_ptr<Railway> Ruler::clone() const
{
    auto p = railway()->cloneBase();
    p->addRulerFrom(*this);
    return p;
}

void Ruler::swap(Ruler& other)
{
    RailIntervalData::swap(other);
    std::swap(_name, other._name);
}

int Ruler::fromSingleTrain(std::shared_ptr<const TrainAdapter> adp, int start, int stop, int period_hours)
{
    int cnt = 0;
    for (auto line : adp->lines()) {
        if (line->isNull())
            continue;
        auto pr = line->stations().begin();
        for (auto p = std::next(pr); p != line->stations().end();
            pr = p, ++p)
        {
            auto it = pr->railStation.lock()->adjacentIntervalTo(p->railStation.lock());
            if (it) {
                //近邻区间存在  更新Node
                auto node = it->getRulerNode(*this);
                int real = qeutil::secsTo(pr->trainStation->depart, 
                    p->trainStation->arrive, period_hours);
                if (pr->trainStation->isStopped() ||
                    line->isStartingStation(pr)) {
                    real -= start;
                }
                if (p->trainStation->isStopped() || line->isTerminalStation(p)) {
                    real -= stop;
                }
                node->interval = std::max(real, 0);
                node->start = start;
                node->stop = stop;
                ++cnt;
            }
        }
    }
    return cnt;
}

void Ruler::fromSpeed(double speed, int start, int stop, bool asmax, int prec, 
    std::vector<bool>& selrow)
{
    int i = 0;
    for (auto n = firstDownNode(); n; n = n->nextNodeCirc()) {
        if (!selrow.at(i++))continue;
        double mile = n->railInterval().mile();
        double secs_f = mile / speed * 3600;
        int secs_i;
        if (asmax) {
            secs_i = qeutil::iceil(secs_f, prec);
        }
        else {
            secs_i = qeutil::iround(secs_f, prec);
        }
        n->interval = secs_i;
        n->start = start;
        n->stop = stop;
    }
}

int Ruler::validNodeCount() const
{
    int cnt = 0;
    for (auto n = firstDownNode(); n; n = n->nextNodeCirc()) {
        if (!n->isNull())cnt++;
    }
    return cnt;
}
