#include "diagrampage.h"
#include "diagram.h"
#include "trainadapter.h"
#include "kernel/trainitem.h"
#include "data/rail/railway.h"
#include "data/common/qesystem.h"


DiagramPage::DiagramPage(const Config& config, const QList<std::shared_ptr<Railway> > &railways,
    const QString& name,const QString& note): _config(config),
    _railways(railways),_name(name),_note(note)
{
    _config.transparent_config = SystemJson::instance.transparent_config;
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
        succ_cfg = _config.fromJson(obj_cfg, false);
    }
    if (!succ_cfg) {
        //qDebug() << "DiagramPage::fromJson: WARNING: load config for page " << _name << " failed, "
        //    << "use config from diagram." << Qt::endl;
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
    _belowLinks.clear();
    _overLinks.clear();
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

#define SWAP(_key) std::swap(_key,other._key)

void DiagramPage::swap(DiagramPage& other)
{
    swapBaseInfo(other);
    std::swap(_railways, other._railways);

    // Items
    SWAP(_startYs);
    SWAP(_itemMap);
    SWAP(_forbidDMap);
    SWAP(_forbidUMap);
    SWAP(_overLabels);
    SWAP(_belowLabels);
    SWAP(_overLinks);
    SWAP(_belowLinks);
}

#undef SWAP


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

std::shared_ptr<DiagramPage> DiagramPage::clone() const
{
    auto page = std::make_shared<DiagramPage>(config(), _railways, _name, _note);
    return page;
}

double DiagramPage::topLabelMaxHeight() const
{
    auto* first_station = _railways[0]->empty() ? nullptr: _railways[0]->stations().front().get();
    //qDebug() << "topLabelMaxHeight first station " << first_station;
    const auto& labels = first_station && _overLabels.contains(first_station) ? _overLabels[first_station] : label_map_t{};
    auto link_itr = _overLinks.find(first_station);
    const auto& links = first_station && link_itr != _overLinks.end() ? link_itr->second : RouteLinkLayerManager{};
    return labelMaxHeight(labels, links);
}

double DiagramPage::bottomLabelMaxHeight() const
{
    auto* last_station = _railways.back()->empty() ? nullptr : _railways.back()->stations().back().get();
    const auto& labels = last_station && _belowLabels.contains(last_station) ? _belowLabels[last_station] : label_map_t{};
    auto link_itr = _belowLinks.find(last_station);
    const auto& links = last_station && link_itr != _belowLinks.end() ? link_itr->second : RouteLinkLayerManager{};
    return labelMaxHeight(labels, links);
}

double DiagramPage::labelMaxHeight(const label_map_t& labels, const RouteLinkLayerManager& links) const
{
    //qDebug() << "labelMaxHeight: labels and links size: " << labels.size() << ", " << links.num_layers();
    double label_height = 0, link_height = 0;
    if (_config.floating_link_line || _config.train_name_mark_style == Config::TrainNameMarkStyle::Link) {
        // In this case, the link layer height is considered
        // An extra 1 is imported by multiplying num_layers (instead of num_layers - 1), 
        // which is just what we desired
        link_height = _config.base_link_height + _config.step_link_height * links.num_layers();
    }
    if (_config.train_name_mark_style != Config::TrainNameMarkStyle::Link) {
        // In this case, the (classic) label height managing is used
        if (_config.avoid_cover) {
            double mx_height = 0.;
            for (auto& p : labels) {
                mx_height = std::max(p.second.height, mx_height);
            }
            label_height = mx_height + _config.step_label_height;
        }
        else {
            label_height = std::max(_config.start_label_height, _config.end_label_height) * 2.;
        }
    }
    //qDebug() << "label_height, link_height " << label_height << " " << link_height;
    return std::max(label_height, link_height);
}
