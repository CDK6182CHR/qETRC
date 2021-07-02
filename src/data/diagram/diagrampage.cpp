#include "diagrampage.h"
#include "diagram.h"
#include "trainadapter.h"
#include "kernel/trainitem.h"

DiagramPage::DiagramPage(Diagram &diagram,
                         const QList<std::shared_ptr<Railway> > &railways,
    const QString& name):
    _diagram(diagram),_railways(railways),_name(name)
{

}

DiagramPage::DiagramPage(Diagram& diagram, const QJsonObject& obj):
    _diagram(diagram)
{
    fromJson(obj);
}

QString DiagramPage::railNameString() const
{
    if (_railways.empty())
        return "";
    auto p = _railways.begin();
    QString res = (*p)->name();
    for (++p; p != _railways.end(); ++p) {
        res += ", " + (*p)->name();
    }
    return res;
}

const Config& DiagramPage::config() const
{
    return _diagram.config();
}

const MarginConfig& DiagramPage::margins() const
{
    return _diagram.config().margins;
}

int DiagramPage::railwayIndex(const Railway& rail) const
{
    for (int i = 0; i < _railways.size(); i++) {
        if (&rail == _railways.at(i).get())
            return i;
    }
    return -1;
}

double DiagramPage::railwayStartY(const Railway& rail) const
{
    int idx = railwayIndex(rail);
    if (idx == -1)
        return 0;
    return _startYs[idx];
}

void DiagramPage::fromJson(const QJsonObject& obj)
{
    QJsonArray ar = obj.value("railways").toArray();
    for (auto p = ar.begin(); p != ar.end(); ++p) {
        auto r = _diagram.railwayByName(p->toString());
        if (!r) {
            qDebug() << "DiagramPage::fromJson: WARNING: "
                << "Railway not found: " << p->toString() << ", to be ignored" << Qt::endl;
        }
        else {
            _railways.append(r);
        }
    }
}

QJsonObject DiagramPage::toJson() const
{
    QJsonArray ar;
    for (auto p : _railways) {
        ar.append(p->name());
    }
    return QJsonObject{
        {"name",_name},
        {"railways",ar}
    };
}

TrainItem* DiagramPage::getTrainItem(TrainLine* line)
{
    if (_itemMap.contains(line))
        return _itemMap.value(line);
    return nullptr;
}

TrainItem* DiagramPage::takeTrainItem(TrainLine* line)
{
    if (_itemMap.contains(line))
        return _itemMap.take(line);
    return nullptr;
}

void DiagramPage::clearTrainItems(const Train& train)
{
    for (auto adp : train.adapters()) {
        for (auto p : adp->lines()) {
            _itemMap.remove(p.get());
        }
    }
}

void DiagramPage::clearAllItems()
{
    _itemMap.clear();
}

void DiagramPage::highlightTrainItems(const Train& train)
{
    for (auto adp : train.adapters()) {
        for (auto p : adp->lines()) {
            if (_itemMap.contains(p.get())) {
                _itemMap.value(p.get())->highlight();
            }
        }
    }
}

void DiagramPage::unhighlightTrainItems(const Train& train)
{
    for (auto adp : train.adapters()) {
        for (auto p : adp->lines()) {
            if (_itemMap.contains(p.get())) {
                _itemMap.value(p.get())->unhighlight();
            }
        }
    }
}

QList<QGraphicsRectItem*>& DiagramPage::dirForbidItem(const Forbid* forbid, Direction dir)
{
    return dir == Direction::Down ?
        _forbidDMap[forbid] : _forbidUMap[forbid];
}

void DiagramPage::addForbidItem(const Forbid* forbid, Direction dir, QGraphicsRectItem* item)
{
    dirForbidItem(forbid, dir).append(item);
}
