#include "diagrampage.h"
#include "diagram.h"
#include "trainadapter.h"
#include "kernel/trainitem.h"


DiagramPage::DiagramPage(const Config& config, const QList<std::shared_ptr<Railway> > &railways,
    const QString& name,const QString& note): _config(config),
    _railways(railways),_name(name),_note(note)
{

}

DiagramPage::DiagramPage(const QJsonObject& obj, Diagram& _diagram)
{
    fromJson(obj, _diagram);
}

std::shared_ptr<DiagramPage> DiagramPage::takenRailCopy(std::shared_ptr<Railway> rail) const
{
    decltype(_railways) rails;
    rails.reserve(_railways.size() - 1);
    foreach(auto r, _railways) {
        if (r != rail)
            rails.push_back(r);
    }
    return std::make_shared<DiagramPage>(_config, rails, _name, _note);
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

void DiagramPage::fromJson(const QJsonObject& obj, Diagram& _diagram)
{
    _name = obj.value("name").toString();
    _note = obj.value("note").toString();
    
    bool succ_cfg = false;
    const auto& obj_cfg = obj.value("config").toObject();
    if (!obj_cfg.isEmpty()) {
        succ_cfg = _config.fromJson(obj_cfg);
    }
    if (!succ_cfg) {
        qDebug() << "DiagramPage::fromJson: WARNING: load config for page " << _name << " failed, "
            << "use config from diagram." << Qt::endl;
        _config = _diagram.config();
    }

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
        {"railways",ar},
        {"note",_note},
        {"config",_config.toJson()}
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

void DiagramPage::highlightTrainItemsWithLink(const Train& train)
{
    for (auto adp : train.adapters()) {
        for (auto p : adp->lines()) {
            if (_itemMap.contains(p.get())) {
                _itemMap.value(p.get())->highlightWithLink();
            }
        }
    }
}

void DiagramPage::unhighlightTrainItemsWithLink(const Train& train)
{
    for (auto adp : train.adapters()) {
        for (auto p : adp->lines()) {
            if (_itemMap.contains(p.get())) {
                _itemMap.value(p.get())->unhighlightWithLink();
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

void DiagramPage::clearGraphics()
{
    _itemMap.clear();
    _forbidDMap.clear();
    _forbidUMap.clear();
    _belowLabels.clear();
    _overLabels.clear();
}

bool DiagramPage::containsRailway(std::shared_ptr<const Railway> rail) const
{
    for (auto r : _railways) {
        if (rail == r)
            return true;
    }
    return false;
}

bool DiagramPage::containsRailway(const Railway& railway) const
{
    for (auto r : _railways) {
        if (&railway == r.get())
            return true;
    }
    return false;
}

void DiagramPage::swapBaseInfo(DiagramPage& other)
{
    std::swap(_name, other._name);
    std::swap(_note, other._note);
}

#define _SWAP(_key) std::swap(_key,other._key)

void DiagramPage::swap(DiagramPage& other)
{
    swapBaseInfo(other);
    std::swap(_railways, other._railways);

    // Items
    _SWAP(_startYs);
    _SWAP(_itemMap);
    _SWAP(_forbidDMap);
    _SWAP(_forbidUMap);
    _SWAP(_overLabels);
    _SWAP(_belowLabels);
}


bool DiagramPage::hasTrain(const Train& train) const
{
    for (auto r : _railways) {
        for (auto a : train.adapters()) {
            if (r == (a->railway()))
                return true;
        }
    }
    return false;
}

void DiagramPage::removeRailway(std::shared_ptr<Railway> rail)
{
    auto p = _railways.begin();
    while (p != _railways.end()) {
        if ((*p) == rail)
            p = _railways.erase(p);
        else
            ++p;
    }
}
