#include "diagram.h"
#include "data/rail/railway.h"
#include "trainadapter.h"
#include "util/utilfunc.h"
#include "data/train/routing.h"
#include "data/train/trainfiltercore.h"
#include "data/diagram/diagrampage.h"
#include "data/rail/forbid.h"
#include "mainwindow/version.h"

#include <QFile>
#include <QJsonObject>
#include <numeric>
#include <QJsonDocument>
#include <cmath>


void Diagram::addRailway(std::shared_ptr<Railway> rail)
{
    railways().append(rail);
    foreach (auto p , trains()) {
        p->bindToRailway(rail, _config);
    }
}

void Diagram::insertRailwayAt(int i, std::shared_ptr<Railway> rail)
{
    railways().insert(i, rail);
    foreach(auto p, trains()) {
        p->bindToRailway(rail, _config);
    }
}

void Diagram::addTrains(const TrainCollection& coll)
{
    //复制语义
    for (auto p : coll.trains()) {
        if (!_trainCollection.trainNameExisted(p->trainName())) {
            auto t = std::make_shared<Train>(*p);
            _trainCollection.trains().append(t);
            updateTrain(t);
        }
    }
}

std::shared_ptr<Railway> Diagram::railwayByName(const QString &name)
{
    foreach(const auto& p,railways()){
        if(p->name() == name)
            return p;
    }
    return nullptr;
}

void Diagram::updateRailway(std::shared_ptr<Railway> r)
{
    foreach (const auto& p, _trainCollection.trains()){
        p->updateBoundRailway(r, _config);
    }
}

void Diagram::updateTrain(std::shared_ptr<Train> t)
{
    foreach(const auto& r, railways()){
        t->updateBoundRailway(r, _config);
    }
}

TrainEventList Diagram::listTrainEvents(const Train& train) const
{
    TrainEventList res;
    foreach (auto p , train.adapters()) {
        res.push_back(qMakePair(p, p->listAdapterEvents(_trainCollection)));
    }
    return res;
}

DiagnosisList Diagram::diagnoseTrain(const Train& train, bool withIntMeet,
    std::shared_ptr<Railway> railway, std::shared_ptr<RailStation> start,
    std::shared_ptr<RailStation> end) const
{
    DiagnosisList res;
    bool filtByRange = railway && start && end;
    foreach(auto adp, train.adapters()) {
        if (!railway || adp->railway() == railway) {
            foreach(auto line, adp->lines()) {
                auto sub = line->diagnoseLine(_trainCollection, withIntMeet);
                if (filtByRange) {
                    foreach(auto ev, sub) {
                        if (ev.inRange(start,end)) {
                            res.push_back(ev);
                        }
                    }
                }
                else {
                    res.append(sub);
                }
            }
        }
    }
    return res;
}

DiagnosisList Diagram::diagnoseAllTrains(std::shared_ptr<Railway> railway, std::shared_ptr<RailStation> start,
    std::shared_ptr<RailStation> end) const
{
    DiagnosisList res;
    foreach(auto t, _trainCollection.trains()) {
        res.append(diagnoseTrain(*t, true, railway, start, end));
    }
    return res;
}

std::shared_ptr<DiagramPage> Diagram::createDefaultPage()
{
    auto t = std::make_shared<DiagramPage>(_config, railways(),
        validPageName(QObject::tr("默认运行图")));
    _pages.append(t);
    return t;
}

bool Diagram::pageNameExisted(const QString& name) const
{
    foreach (auto p , _pages) {
        if (p->name() == name)
            return true;
    }
    return false;
}

bool Diagram::pageNameExisted(const QString& name, std::shared_ptr<DiagramPage> ignore) const
{
    foreach(auto p, _pages) {
        if (p->name() == name && p != ignore)
            return true;
    }
    return false;
}

bool Diagram::pageNameIsValid(const QString& name, std::shared_ptr<DiagramPage> page)
{
    if (name.isEmpty())return false;
    foreach (auto p , _pages) {
        if (p->name() == name && p != page)
            return false;
    }
    return true;
}

bool Diagram::railwayNameExisted(const QString& name) const
{
    for (auto p : railways()) {
        if (p->name() == name)
            return true;
    }
    return false;
}

QString Diagram::validRailwayName(const QString& prefix) const
{
    for (int i = 0;; i++) {
        QString res = prefix;
        if (i)res += QString::number(i);
        if (!railwayNameExisted(res))
            return res;
    }
}

void Diagram::applyBindOn(TrainCollection& coll)
{
    foreach (auto p , railways()) {
        for (auto t : coll.trains()) {
            t->bindToRailway(p, _config);
        }
    }
}

bool Diagram::isValidRailName(const QString& name, std::shared_ptr<Railway> rail)
{
    if (name.isEmpty())
        return false;
    foreach (auto r , railways()) {
        if (r != rail && r->name() == name)
            return false;
    }
    return true;
}

int Diagram::getPageIndex(std::shared_ptr<DiagramPage> page) const
{
    for (int i = 0; i < _pages.size(); i++)
        if (_pages.at(i) == page)
            return i;
    return -1;
}

[[deprecated]]
void Diagram::removeRailwayAt(int i)
{
    auto rail = railways().takeAt(i);
    //清理Page中的
    auto it = _pages.begin();
    while (it != _pages.end()) {
        (*it)->removeRailway(rail);
        if ((*it)->isNull())
            it = _pages.erase(it);
        else
            ++it;
    }
    //与列车解除绑定
    foreach (auto p , _trainCollection.trains()) {
        p->unbindToRailway(rail);
    }
}

void Diagram::removeRailwayAtU(int i)
{
    auto rail = railways().takeAt(i);
    //与列车解除绑定
    foreach(auto p, _trainCollection.trains()) {
        p->unbindToRailway(rail);
    }
}

void Diagram::rebindAllTrains()
{
    foreach (auto t , _trainCollection.trains()) {
        t->clearBoundRailways();
        foreach (auto p , railways()) {
            t->bindToRailway(p, _config);
        }
    }
}

void Diagram::undoImportRailway()
{
    auto t = railways().takeLast();
    foreach(auto p, _trainCollection.trains()) {
        p->unbindToRailway(t);
    }
}

std::map<std::shared_ptr<RailInterval>, int> Diagram::sectionTrainCount(std::shared_ptr<Railway> railway) const
{
    std::map<std::shared_ptr<RailInterval>, int> res;
    for (auto train : _trainCollection.trains()) {
        foreach (auto adp , train->adapters()) {
            if((adp->railway())==railway){
                foreach(auto line,adp->lines()){
                    sectionTrainCount(res, line);
                }
            }
        }
    }
    return res;
}

std::pair<Diagram::sec_cnt_t, Diagram::sec_cnt_t>
    Diagram::sectionPassenFreighCount(std::shared_ptr<Railway> railway)const
{
    sec_cnt_t passen, freigh;
    foreach (auto train , _trainCollection.trains()) {
        bool ispas = train->getIsPassenger();
        foreach(auto adp, train->adapters()) {
            if ((adp->railway()) == railway) {
                foreach(auto line, adp->lines()) {
                    if (ispas) {
                        sectionTrainCount(passen, line);
                    }
                    else {
                        sectionTrainCount(freigh, line);
                    }
                }
            }
        }
    }
    return std::make_pair(passen, freigh);
}

std::vector<std::pair<std::shared_ptr<TrainLine>, const AdapterStation*>> 
Diagram::stationTrainsSettled(std::shared_ptr<Railway> railway,
    std::shared_ptr<RailStation> st) const
{
    std::vector<std::pair<std::shared_ptr<TrainLine>, const AdapterStation*>> res;
    foreach(auto train , _trainCollection.trains()) {
        foreach (auto adp , train->adapters()) {
            if (adp->isInSameRailway(railway)) {
                for (auto line : adp->lines()) {
                    auto* p = line->stationFromRail(st);
                    if (p) {
                        //res.append(qMakePair(line, p));
                        res.emplace_back(line, p);
                    }
                }
            }
        }
    }
    using PR = std::pair<std::shared_ptr<TrainLine>, const AdapterStation*>;
    //默认按照到达时刻排序  注意这里按照绝对时刻排序就好
    std::sort(res.begin(), res.end(), [](const PR& p1, const PR& p2) {
        return p1.second->trainStation->arrive <
            p2.second->trainStation->arrive;
        });
    return res;
}

RailStationEventList
    Diagram::stationEvents(std::shared_ptr<Railway> railway, 
        std::shared_ptr<const RailStation> st,
        const ITrainFilter* filter) const
{
    RailStationEventList res;
    foreach (auto train , _trainCollection.trains()) {
        if (filter && !filter->check(train)) continue;
        foreach (auto adp , train->adapters()) {
            if (adp->isInSameRailway(railway)) {
                foreach (auto line , adp->lines()) {
                    const auto& lst = line->stationEventFromRail(st);
                    for (auto p = lst.begin(); p != lst.end(); ++p) {
                        res.push_back(*p);
                    }
                }
            }
        }
    }
    using PR = RailStationEventList::value_type;
    std::sort(res.begin(), res.end(), [](const PR& p1, const PR& p2) {
        return p1->time < p2->time;
        });
    return res;
}

std::map<std::shared_ptr<RailStation>, RailStationEventList>
    Diagram::stationEventsForRail(std::shared_ptr<Railway> railway)const
{
    std::map<std::shared_ptr<RailStation>, RailStationEventList> res;
    foreach(auto p, qAsConst(railway->stations())) {
        if (p->direction != PassedDirection::NoVia) {
            res.emplace(p, stationEvents(railway, p));
        }
    }
    return res;
}

RailwayStationEventAxis Diagram::stationEventAxisForRail(std::shared_ptr<Railway> railway, 
    const ITrainFilter& filter) const
{
    RailwayStationEventAxis res;
    foreach(auto p, qAsConst(railway->stations())) {
        if (p->direction != PassedDirection::NoVia) {
            StationEventAxis staxis = stationEvents(railway, p, &filter);
            staxis.buildAxis();
            res.emplace(p, std::move(staxis));
        }
    }
    return res;
}

std::vector<std::pair<std::shared_ptr<TrainLine>, QTime>> Diagram::sectionEvents(std::shared_ptr<Railway> railway, double y) const
{
    SectionEventList res;
    for (auto train : _trainCollection.trains()) {
        for (auto adp : train->adapters()) {
            if (adp->isInSameRailway(railway)) {
                for (auto line : adp->lines()) {
                    auto t = line->sectionTime(y);
                    if (t.has_value()) {
                        res.emplace_back(line, t.value());
                    }
                }
            }
        }
    }
    using PR = std::pair<std::shared_ptr<TrainLine>, QTime>;
    std::sort(res.begin(), res.end(), [](const PR& p1, const PR& p2) {
        return p1.second< p2.second;
        });
    return res;
}

#if 0
TrainGapList Diagram::getTrainGaps(const RailStationEventList& events,
    const TrainFilterCore& filter, std::shared_ptr<const RailStation> st, 
    std::optional<bool> useSingle)const
{
    if (useSingle.has_value())
        return *useSingle ?
        getTrainGapsSingle(events, filter) :
        getTrainGapsDouble(events, filter);
    else return getTrainGapsSingle(events, filter);
}


TrainGapList Diagram::getTrainGapsDouble(const RailStationEventList& events,
    const TrainFilterCore& filter)const
{
    TrainGapList res;
    std::shared_ptr<const RailStationEvent>
        downFormerLast = findLastEvent(events,filter, Direction::Down, RailStationEvent::Pre),
        downLatterLast = findLastEvent(events, filter, Direction::Down, RailStationEvent::Post),
        upFormerLast = findLastEvent(events, filter, Direction::Up, RailStationEvent::Post),
        upLatterLast = findLastEvent(events, filter, Direction::Up, RailStationEvent::Pre);

    // 当前站内车的情况。所有【到达】事件直接压进来。int位记录被踩了多少次。
    // 类似队列的结构：从back插入，从front删除，但不绝对禁止中间删除。
    std::deque<std::pair<std::shared_ptr<const RailStationEvent>, int>> down_in, up_in;

    // 先预扫描一遍：把0点前进站没出站的存下来 （跨日停站的）
    for (auto _p = events.cbegin(); _p != events.cend(); ++_p) {
        const auto& p = *_p;
        if (!filter.check(p->line->train()))continue;
        if (p->line->dir() == Direction::Down) {
            if (p->type == TrainEventType::Arrive) {
                down_in.emplace_back(std::make_pair(p, 0));
            }
            else if (p->type == TrainEventType::Depart) {
                std::optional<decltype(down_in)::iterator> pitr{};   //被删除的那个的迭代器 
                for (auto itr = down_in.begin(); itr != down_in.end(); ++itr) {
                    if (itr->first->line == p->line) {
                        // 找到当前的进站记录
                        pitr = down_in.erase(itr);
                        break;
                    }
                }
                if (pitr.has_value()) {
                    for (auto itr = down_in.begin(); itr != pitr; ++itr) {
                        itr->second++;
                    }
                }
            }
            else if (p->type == TrainEventType::CalculatedPass ||
                p->type == TrainEventType::SettledPass) {
                if (p->pos == RailStationEvent::Both) {
                    // 站内所有车被踩一次
                    for (auto& q : down_in) {
                        q.second++;
                    }
                }
            }
        }
        else {  // Up
            if (p->type == TrainEventType::Arrive) {
                up_in.emplace_back(std::make_pair(p, 0));
            }
            else if (p->type == TrainEventType::Depart) {
                std::optional<decltype(up_in)::iterator> pitr{};   //被删除的那个的迭代器 
                for (auto itr = up_in.begin(); itr != up_in.end(); ++itr) {
                    if (itr->first->line == p->line) {
                        // 找到当前的进站记录
                        pitr = up_in.erase(itr);
                        break;
                    }
                }
                if (pitr.has_value()) {
                    for (auto itr = up_in.begin(); itr != pitr; ++itr) {
                        itr->second++;
                    }
                }
            }
            else if (p->type == TrainEventType::CalculatedPass ||
                p->type == TrainEventType::SettledPass) {
                if (p->pos == RailStationEvent::Both) {
                    // 站内所有车被踩一次
                    for (auto& q : up_in) {
                        q.second++;
                    }
                }
            }
        }
    }


    // 注意：pre/post是说绝对位置（里程小端和大端），former latter是说运行方向前后
    for (auto _p = events.cbegin(); _p != events.cend(); ++_p) {
        const auto& p = *_p;
        if (!filter.check(p->line->train())) continue;
        if (p->line->dir() == Direction::Down) {
            if (p->pos & RailStationEvent::Pre) {
                // 与站前有交集
                // 既然遇到了一个，那么last一定不是空
                res.emplace_back(std::make_shared<TrainGap>(downFormerLast, p));
                downFormerLast = p;
            }
            if (p->pos & RailStationEvent::Post) {
                // 与站后有交集
                if (!(p->pos & RailStationEvent::Pre)) {
                    // 这个条件是针对通通的情况，防止重复
                    res.emplace_back(std::make_shared<TrainGap>(downLatterLast, p));
                }
                downLatterLast = p;
            }

            // 现在：判断进出站情况。只考虑到达出发，以及通过的。
            if (p->type == TrainEventType::Arrive) {
                // 车次到达：进队列
                down_in.emplace_back(std::make_pair(p, 0));
            }
            else if (p->type == TrainEventType::Depart) {
                // 出发：先是所有比它到的早但还没跑掉的，都被踩一次
                std::optional<decltype(down_in)::iterator> pitr{};   //被删除的那个的迭代器 
                for (auto itr = down_in.begin(); itr != down_in.end(); ++itr) {
                    if (itr->first->line == p->line) {
                        // 找到当前的进站记录
                        if (itr->second) {
                            // 存在被踩情况
                            res.emplace_back(std::make_shared<TrainGap>(
                                itr->first, p, TrainGap::Avoid, itr->second));
                        }
                        pitr = down_in.erase(itr);
                        break;
                    }
                }
                if (pitr.has_value()) {
                    // 删除实际发生了，所有此前列车都被踩一次
                    // 这个条件是为了防止运行线端点没有到达事件只有出发事件的情况
                    for (auto itr = down_in.begin(); itr != pitr; ++itr) {
                        itr->second++;
                    }
                }
            }
            else if (p->type == TrainEventType::CalculatedPass ||
                p->type == TrainEventType::SettledPass) {
                if (p->pos == RailStationEvent::Both) {
                    // 站内所有车被踩一次
                    for (auto& q : down_in) {
                        q.second++;
                    }
                }
            }

        }
        else {  // Up
            if (p->pos & RailStationEvent::Pre) {
                // latter间隔
                res.emplace_back(std::make_shared<TrainGap>(upLatterLast, p));
                upLatterLast = p;
            }
            if(p->pos & RailStationEvent::Post) {
                // former间隔
                if (!(p->pos & RailStationEvent::Pre)) {
                    res.emplace_back(std::make_shared<TrainGap>(upFormerLast, p));
                }
                upFormerLast = p;
            }

            if (p->type == TrainEventType::Arrive) {
                up_in.emplace_back(p, 0);
            }
            else if (p->type == TrainEventType::Depart) {
                std::optional<decltype(up_in)::iterator> pitr{};   //被删除的那个的迭代器 
                for (auto itr = up_in.begin(); itr != up_in.end(); ++itr) {
                    if (itr->first->line == p->line) {
                        // 找到当前的进站记录
                        if (itr->second) {
                            // 存在被踩情况
                            res.emplace_back(std::make_shared<TrainGap>(
                                itr->first, p, TrainGap::Avoid, itr->second));
                        }
                        pitr = up_in.erase(itr);
                        break;
                    }
                }
                if (pitr.has_value()) {
                    // 删除实际发生了，所有此前列车都被踩一次
                    // 这个条件是为了防止运行线端点没有到达事件只有出发事件的情况
                    for (auto itr = up_in.begin(); itr != pitr; ++itr) {
                        itr->second++;
                    }
                }
            }
            else if (p->type == TrainEventType::CalculatedPass ||
                p->type == TrainEventType::SettledPass) {
                if (p->pos == RailStationEvent::Both) {
                    for (auto& itr : up_in) {
                        itr.second++;
                    }
                }
            }
        }
    }
    return res;
}

TrainGapList Diagram::getTrainGapsSingle(const RailStationEventList& events,
    const TrainFilterCore& filter)const
{
    // 单线版本：直接复制双线的代码，
    // 原来down的保留直接作为单线的，up的删除
    TrainGapList res;
    std::shared_ptr<const RailStationEvent>
        downFormerLast = findLastEvent(events, filter, Direction::Down, RailStationEvent::Pre),
        downLatterLast = findLastEvent(events, filter, Direction::Down, RailStationEvent::Post);

    // 当前站内车的情况。所有【到达】事件直接压进来。int位记录被踩了多少次。
    // 类似队列的结构：从back插入，从front删除，但不绝对禁止中间删除。
    std::deque<std::pair<std::shared_ptr<const RailStationEvent>, int>> down_in;

    // 先预扫描一遍：把0点前进站没出站的存下来 （跨日停站的）
    for (auto _p = events.cbegin(); _p != events.cend(); ++_p) {
        const auto& p = *_p;
        if (!filter.check(p->line->train()))continue;
        if (p->type == TrainEventType::Arrive) {
            down_in.emplace_back(std::make_pair(p, 0));
        }
        else if (p->type == TrainEventType::Depart) {
            std::optional<decltype(down_in)::iterator> pitr{};   //被删除的那个的迭代器 
            for (auto itr = down_in.begin(); itr != down_in.end(); ++itr) {
                if (itr->first->line == p->line) {
                    // 找到当前的进站记录
                    pitr = down_in.erase(itr);
                    break;
                }
            }
            if (pitr.has_value()) {
                for (auto itr = down_in.begin(); itr != pitr; ++itr) {
                    itr->second++;
                }
            }
        }
        else if (p->type == TrainEventType::CalculatedPass ||
            p->type == TrainEventType::SettledPass) {
            if (p->pos == RailStationEvent::Both) {
                // 站内所有车被踩一次
                for (auto& q : down_in) {
                    q.second++;
                }
            }
        }

    }

    // 注意：pre/post是说绝对位置（里程小端和大端），former latter是说运行方向前后
    for (auto _p = events.cbegin(); _p != events.cend(); ++_p) {
        const auto& p = *_p;
        if (!filter.check(p->line->train())) continue;
        if (p->pos & RailStationEvent::Pre) {
            // 与站前有交集
            // 既然遇到了一个，那么last一定不是空
            res.emplace_back(std::make_shared<TrainGap>(downFormerLast, p));
            downFormerLast = p;
        }
        if (p->pos & RailStationEvent::Post) {
            // 与站后有交集
            if (!(p->pos & RailStationEvent::Pre)) {
                // 这个条件是针对通通的情况，防止重复
                res.emplace_back(std::make_shared<TrainGap>(downLatterLast, p));
            }
            downLatterLast = p;
        }

        // 现在：判断进出站情况。只考虑到达出发，以及通过的。
        if (p->type == TrainEventType::Arrive) {
            // 车次到达：进队列
            down_in.emplace_back(std::make_pair(p, 0));
        }
        else if (p->type == TrainEventType::Depart) {
            // 出发：先是所有比它到的早但还没跑掉的，都被踩一次
            std::optional<decltype(down_in)::iterator> pitr{};   //被删除的那个的迭代器 
            for (auto itr = down_in.begin(); itr != down_in.end(); ++itr) {
                if (itr->first->line == p->line) {
                    // 找到当前的进站记录
                    if (itr->second) {
                        // 存在被踩情况
                        res.emplace_back(std::make_shared<TrainGap>(
                            itr->first, p, TrainGap::Avoid, itr->second));
                    }
                    pitr = down_in.erase(itr);
                    break;
                }
            }
            if (pitr.has_value()) {
                // 删除实际发生了，所有此前列车都被踩一次
                // 这个条件是为了防止运行线端点没有到达事件只有出发事件的情况
                for (auto itr = down_in.begin(); itr != pitr; ++itr) {
                    itr->second++;
                }
            }
        }
        else if (p->type == TrainEventType::CalculatedPass ||
            p->type == TrainEventType::SettledPass) {
            if (p->pos == RailStationEvent::Both) {
                // 站内所有车被踩一次
                for (auto& q : down_in) {
                    q.second++;
                }
            }
        }
    }
    return res;
}

#endif 


SnapEventList Diagram::getSnapEvents(std::shared_ptr<Railway> railway, const QTime& time) const
{
    SnapEventList res;
    foreach (auto train , _trainCollection.trains()) {
        foreach (auto adp , train->adapters()) {
            if (adp->isInSameRailway(railway)) {
                foreach (auto line , adp->lines()) {
                    res.append(line->getSnapEvents(time));
                }
            }
        }
    }
    std::sort(res.begin(), res.end());   //按里程排序
    return res;
}



ReadRulerReport Diagram::rulerFromMultiTrains(std::shared_ptr<Railway> railway, 
    const QVector<std::shared_ptr<RailInterval>> intervals,
    const QList<std::shared_ptr<Train>> trains,
    bool useAverage, int defaultStart, int defaultStop, 
    int cutStd, int cutSec, int prec, int cutCount)
{
    ReadRulerReport res;
    //先直接把要计算的区间都加进去，免得多搞个set
    foreach(auto p, intervals) {
        res.insert({ p,{} });
    }
    decltype(res.begin()) itr;

    foreach(auto train, trains) {
        auto adp = train->adapterFor(*railway);
        if (!adp)continue;
        foreach(auto line, adp->lines()) {
            if (line->count() < 2)continue;
            auto pr = line->stations().begin();
            for (auto p = std::next(pr); p != line->stations().end(); pr = p, ++p) {
                if (pr->railStation.lock()->dirAdjacent(line->dir()) ==
                    p->railStation.lock()) {
                    // pr->p是合法的区间
                    auto it = pr->railStation.lock()->dirNextInterval(line->dir());
                    if ((itr = res.find(it)) != res.end()) {
                        //此区间是要计算的区间
                        int secs = qeutil::secsTo(pr->trainStation->depart,
                            p->trainStation->arrive);
                        auto att = line->getIntervalAttachType(pr, p);
                        itr->second.raw.emplace(train, std::make_pair(secs, att));
                    }
                }
            }
        }
    }

    for (auto p = res.begin(); p != res.end(); ++p) {
        __intervalFt(p->second);
        if (useAverage) {
            __intervalRulerMean(p->second, defaultStart, defaultStop, 
                prec, cutStd, cutSec, cutCount);
        }
        else {
            __intervalRulerMode(p->second, defaultStart, defaultStop, prec, cutCount);
        }
    }
    return res;
}

QVector<int> Diagram::pageIndexWithRail(std::shared_ptr<const Railway> railway)const
{
    QVector<int> res;
    for (int i = 0; i < _pages.size(); i++) {
        if (_pages.at(i)->containsRailway(*railway)) {
            res.push_back(i);
        }
    }
    return res;
}

void Diagram::bindAllTrains()
{
    foreach (auto p , railways()) {
        foreach (auto t , _trainCollection.trains()) {
            t->bindToRailway(p, _config);
        }
    }
}

QString Diagram::validPageName(const QString& prefix) const
{
    for (int i = 0;; i++) {
        QString name = prefix;
        if (i) name.append(QString::number(i));
        if (!pageNameExisted(name))
            return name;
    }
}

void Diagram::sectionTrainCount(std::map<std::shared_ptr<RailInterval>, int>& res, 
    std::shared_ptr<TrainLine> line) const
{
    auto startst = line->firstRailStation();
    auto endst = line->lastRailStation();
    auto inter = startst->dirNextInterval(line->dir());
    for (; inter && inter->toStation() != endst; inter = inter->nextInterval()) {
        ++res[inter];
    }
    if (inter) {
        //这是最后一个区间
        ++res[inter];
    }
    else {
        qDebug() << "Diagram::sectionTrainCount: WARNING: unexpected non-ended interval: "
            << line->train()->trainName().full() << ", " << endst->name;
    }
        
}

#define TRC_WARNING qDebug()<<"Diagram::fromTrc: WARNING: "

namespace etrc_consts {
    static const QString
        SPLIT_CIRCUIT = "***Circuit***",
        SPLIT_TRAIN = "===Train===",
        SPLIT_COLOR = "---Color---",
        SPLIT_LINETYPE = "---LineType---",
        SPLIT_SETUP = "...Setup...";
    static const QString
        TRUE = "true", FALSE = "false", NA = "NA";

    enum class Status {
        Invalid,
        Circuit,
        Train,
        Color,
        LineType,
        Setup
    };
}

bool Diagram::fromTrc(QTextStream& fin)
{
    QString line;
    //fin.setCodec("utf-8");

    clear();
    trainCollection().typeManager().operator=(_defaultManager);

    using namespace etrc_consts;
    Status status = Status::Invalid;

    std::shared_ptr<Railway> railway;
    std::shared_ptr<Train> train;
    QMap<QString, QList<QPair<int, std::shared_ptr<Train>>>> rout_map;   //交路信息

    while (!fin.atEnd()) {
        fin.readLineInto(&line);
        line = line.trimmed();
        //状态判断，同时读取头部几行信息
        if (line == SPLIT_CIRCUIT) {
            status = Status::Circuit;
            fin.readLineInto(&line);    //线名
            railway = std::make_shared<Railway>(line);
            fin.readLineInto(nullptr);   //总里程
            continue;
        }
        else if (line == SPLIT_TRAIN) {
            status = Status::Train;
            fin.readLineInto(&line);    //车次头
            auto s = line.split(",");
            if (s.size() < 4) {
                TRC_WARNING << "Invalid train header: " << line << Qt::endl;
                status = Status::Invalid;
            }
            else {
                const QString& star = fin.readLine(), term = fin.readLine();
                train = std::make_shared<Train>(TrainName(s.at(1), s.at(2), s.at(3)),
                    star, term);
                trainCollection().appendTrain(train);
                train->setType(_trainCollection.typeManager().fromRegex(train->trainName()));
                if (s.size() >= 5 && !s.at(4).isEmpty() && s.at(4) != NA) {
                    //交路
                    auto rt = s.at(4).split("_");
                    if (rt.size() != 2) {
                        TRC_WARNING << "Invalid train routing info: " << s.at(4) << ", " <<
                            "for train " << train->trainName().full() << Qt::endl;
                    }
                    else {
                        rout_map[rt.at(0)].append(qMakePair(rt.at(1).toInt(), train));
                    }
                }
            }
            continue;
        }
        else if (line == SPLIT_COLOR) {
            status = Status::Color;
            continue;
        }
        else if (line == SPLIT_LINETYPE) {
            status = Status::LineType;
            continue;
        }
        else if (line == SPLIT_SETUP) {
            status = Status::Setup;
            continue;
        }

        //下面：读取内容部分
        if (status == Status::Circuit) {
            auto t = line.split(",");
            // 集宁南,0,2,false,,false,4,0,0,
            // 站名, 里程, 等级, 隐藏, 
            if (t.size() < 3) {
                qDebug() << "Diagram::fromTrc: WARNING: Invalid circuit line: "
                    << line << Qt::endl;
                continue;
            }
            bool show = true;
            if (t.size() >= 4) {
                show = (t.at(3) == FALSE);   //ETRC中是隐藏
            }
            // 2022.10.14：增加复线读取
            bool single = false;
            if (t.size() >= 6) {
                single = (t.at(5) == FALSE);   // ETRC中是复线
            }
            railway->appendStation(t.at(0), t.at(1).toDouble(), t.at(2).toInt(),
                std::nullopt, PassedDirection::BothVia, show, false, false, single);
            if (t.size() >= 10 && !t.at(9).isEmpty()
                && railway->stationCount() >= 2) {
                //at(9)是天窗信息
                auto s = t.at(9).split("-");
                if (s.size() >= 2) {
                    QTime start = qeutil::parseTime(s.at(0)),
                        end = qeutil::parseTime(s.at(1));
                    auto forbid = railway->firstForbid();
                    auto node = railway->firstUpInterval()->getForbidNode(forbid);
                    node->beginTime = start;
                    node->endTime = end;
                }
                else {
                    qDebug() << "Diagram::fromTrc: WARNING: Invalid forbid time: " <<
                        t.at(9) << Qt::endl;
                }
            }
        }
        else if (status == Status::Train) {
            //天津南,12:31,12:33,true,NA,0
            //站名, 到点, 开点, 营业, <不读取>, 站台
            auto t = line.split(",");
            if (t.size() >= 3) {
                bool business = true;
                if (t.size() >= 4) {
                    business = (t.at(3) == TRUE);
                }
                QString track;
                if (t.size() >= 6)
                    track = t.at(5);
                train->appendStation(t.at(0), qeutil::parseTime(t.at(1)),
                    qeutil::parseTime(t.at(2)), business, track);
            }
            else {
                TRC_WARNING << "Invalid train station line: " << line << Qt::endl;
            }
        }
        else if (status == Status::Color) {
            auto t = line.split(",");
            if (t.size() == 4) {
                auto train = _trainCollection.findFullName(t.at(0));
                if (train) {
                    QPen pen = train->pen();
                    pen.setColor(QColor(t.at(1).toInt(), t.at(2).toInt(), t.at(3).toInt()));
                    train->setPen(pen);
                }
                else {
                    TRC_WARNING << "Train not found for color: " << t.at(0) << Qt::endl;
                }
            }
            else {
                TRC_WARNING << "Invalid color line: " << line << Qt::endl;
            }
        }
        else {
            continue;
        }
    }
    //最后：线路插入处理
    if (railway) {
        railway->firstForbid()->copyUpToDown();
        // 2022.10.13: 增加Y坐标计算
        railway->calStationYCoeff();
        addRailway(railway);
    }
    
    using PR = QPair<int, std::shared_ptr<Train>>;
    for (auto p = rout_map.begin(); p != rout_map.end(); ++p) {
        auto rt = std::make_shared<Routing>();
        rt->setName(p.key());
        std::stable_sort(p.value().begin(), p.value().end(), [](const PR& p1, const PR& p2) {
            return p1.first < p2.first;
            });
        for (const auto& r : p.value()) {
            rt->appendTrain(r.second, true);
        }
        _trainCollection.routings().append(rt);
    }

    return !isNull();   //只要读入了数据，就算成功
}

void Diagram::__intervalFt(readruler::IntervalReport& itrep)
{
    for (auto p = itrep.raw.begin(); p != itrep.raw.end(); ++p) {
        itrep.types[p->second.second].count[p->second.first]++;
    }
}


namespace readruler {

    int typeCount(const IntervalTypeCount& cnt) 
    {
        return std::accumulate(cnt.begin(), cnt.end(), 0,
            [](auto x, auto y) { return x + y.second; });
    }

    /**
     * pyETRC.Graph.__intervalRulerMean.<lambda>moment()
     * 计算样本均值和标准差  保证输入非空
     */
    std::pair<double, double> moment(const IntervalTypeCount& rep)
    {
        if (rep.size() == 1) {
            return std::make_pair(rep.begin()->first + 0.0, 0.0);
        }
        int n = typeCount(rep);
        double ave = std::accumulate(rep.begin(), rep.end(), 0.0,
            [](auto x, auto y) {return x + y.first * y.second; }) / n;
        double s2 = std::accumulate(rep.begin(), rep.end(), 0.0,
            [=](auto x, auto y) {return x + std::pow(y.first - ave, 2) * y.second; })
            / (n - 1);
        return std::make_pair(ave, std::sqrt(s2));
    }

    /**
     * pyETRC.data.Graph.__intervalRulerMean.<lambda>furthest
     * 返回离均值最远的那个数据的迭代器，实际上只能是首末之一。
     * 保证rep的元素数量大于1.
     */
    IntervalTypeCount::iterator furthest(IntervalTypeCount& rep, double ave)
    {
        return std::fabs(rep.begin()->first - ave) > std::fabs(rep.rbegin()->first - ave) ?
            rep.begin() : std::prev(rep.end());
    }

    using _valCompTy = const std::map<int, int>::value_type&;
    bool valueCountComp(_valCompTy x, _valCompTy y) {
        if (x.second == y.second) {
            return x.first > y.first;
        }
        else {
            return x.second < y.second;
        }
    }
}

void Diagram::__intervalRulerMode(readruler::IntervalReport& itrep,
    int defaultStart, int defaultStop, int prec, int cutCount)
{
    std::map<TrainLine::IntervalAttachType, double> modes;
    for (auto tp = itrep.types.begin(); tp != itrep.types.end(); ++tp) {
        auto& tpcnt = tp->second.count;
        // 所选的数据迭代器  一定存在
        auto sel = std::max_element(tpcnt.begin(), tpcnt.end(), readruler::valueCountComp);
        tp->second.value = sel->first;
        tp->second.tot = sel->second;     //众数模式下的数据量就是最大的那个数据的数据量
        if (sel->second >= cutCount) {
            modes.emplace(tp->first, sel->first);
            tp->second.used = true;
        }
        else {
            tp->second.used = false;
        }
    }
    if (modes.size() == 4) {
        std::vector<std::pair<int, TrainLine::IntervalAttachType>> tycount;
        for (auto p = itrep.types.begin(); p != itrep.types.end(); ++p)
            tycount.emplace_back(p->second.tot, p->first);
        std::sort(tycount.begin(), tycount.end());
        if (tycount[0].first != tycount[1].first) {
            // 最少的那个的数值是唯一的，直接删了就行了
            modes.erase(tycount[0].second);
            itrep.types.at(tycount[0].second).used = false;
        }
        else {
            int n = 1; // n 是简并数
            while (tycount[n].first == tycount[0].first)
                n++;
            // 剔除不同数据导致的通通时分的打表
            std::vector<std::pair<int, TrainLine::IntervalAttachType>> trials;
            for (int i = 0; i < n; i++) {
                auto modes_copy = modes;    //copy construct
                modes_copy.erase(tycount[i].second);
                trials.emplace_back(
                    std::get<0>(__computeIntervalRuler(modes_copy, defaultStart, defaultStop, prec)),
                    tycount[i].second);
            }
            auto p = std::min_element(trials.begin(), trials.end());
            modes.erase(p->second);
            itrep.types.at(p->second).used = false;
        }
    }
    auto [x, y, z] = __computeIntervalRuler(modes, defaultStart, defaultStop, prec);
    itrep.interval = x;
    itrep.start = y;
    itrep.stop = z;
}


void Diagram::__intervalRulerMean(readruler::IntervalReport& itrep, 
    int defaultStart, int defaultStop, int prec, 
    int cutStd, int cutSec, int cutCount)
{
    std::map<TrainLine::IntervalAttachType, double> means;   //每种情况的采信数据 
    for (auto tp = itrep.types.begin(); tp != itrep.types.end(); ++tp) {
        // 注意：IntervalTypeReport是天然按照数值排列的
        auto& tpcnt = tp->second.count;
        if (cutSec) {
            while (tpcnt.size() > 1) {
                auto [ave, sigma] = readruler::moment(tpcnt);
                auto f = readruler::furthest(tpcnt, ave);
                if (std::abs(f->first - ave) > cutSec) {
                    tpcnt.erase(f);
                }
                else break;
            }
        }
        else if (cutStd) {
            while (tpcnt.size() > 1) {
                auto [ave, sigma] = readruler::moment(tpcnt);
                auto f = readruler::furthest(tpcnt, ave);
                if (sigma && std::fabs(f->first - ave) / sigma > cutStd) {
                    tpcnt.erase(f);
                }
                else break;
            }
        }
        tp->second.tot = readruler::typeCount(tpcnt);
        double ave = readruler::moment(tpcnt).first;
        tp->second.value = ave;
        
        //检查各类的最终数据量是否符合要求
        if (tp->second.tot >= cutCount) {
            means.emplace(tp->first,ave );
            tp->second.used = true;
        }
        else {
            tp->second.used = false;
        }
    }
    auto [x, y, z] = __computeIntervalRuler(means, defaultStart, defaultStop, prec);
    itrep.interval = x;
    itrep.start = y;
    itrep.stop = z;
}

namespace readruler {
    void getTypeValue(const std::map<TrainLine::IntervalAttachType, double>& values,
        TrainLine::IntervalAttachType tp, double& v) 
    {
        if (auto p = values.find(tp); p != values.end())
            v = p->second;
    }

    /**
     * 根据prec进行类似四舍五入的数值修约
     */
    int __round(int value, int prec)
    {
        if (value < 0)value = 0;
        int q = value % prec;
        if (q == 0)
            return value;
        else if (q >= prec / 2)  //int div
            return value + prec - q;
        else return value - q;
    }
}

std::tuple<int, int, int> 
    Diagram::__computeIntervalRuler(const std::map<TrainLine::IntervalAttachType, double>& values,
        int defaultStart, int defaultStop, int prec)
{
    double a = 0, b = 0, c = 0, d = 0;
    double x = 0, y = 0, z = 0;
    readruler::getTypeValue(values, TrainLine::AttachNone, a);
    readruler::getTypeValue(values, TrainLine::AttachStart, b);
    readruler::getTypeValue(values, TrainLine::AttachStop, c);
    readruler::getTypeValue(values, TrainLine::AttachBoth, d);
    if (values.size() == 4) {
        //伪逆求解
        x = 0.75 * a + 0.25 * b + 0.25 * c - 0.25 * d;
        y = -0.5 * a + 0.50 * b - 0.50 * c + 0.50 * d;
        z = -0.5 * a - 0.50 * b + 0.50 * c + 0.50 * d;
    }
    else if (values.size() == 3) {
        // 三个数据，唯一解
        if (!a) {
            x = b + c - d; y = -c + d; z = -b + d;
        }
        else if (!b) {
            x = a; y = -c + d; z = -a + c;
        }
        else if (!c) {
            x = a; y = -a + b; z = -b + d;
        }
        else {
            x = a; y = -a + b; z = -a + c;
        }
    }
    else if (values.size() == 2) {
        if (a) {
            if (b) {
                x = a; y = b - a; z = defaultStop;
            }
            else if (c) {
                x = a; y = defaultStart; z = c - a;
            }
            else {
                x = a; y = defaultStart; z = d - a - defaultStart;
            }
        }
        else {  // !a
            if (!b) {
                x = c - defaultStop; y = d - c; z = defaultStop;
            }
            else if (!c) {
                x = b - defaultStart; y = defaultStart; z = d - b;
            }
            else {
                x = b - defaultStart; y = defaultStart; z = c - b + defaultStart;
            }
        }
    }
    else {   // 只有一个数据
        y = defaultStart; z = defaultStop;
        if (a) x = a;
        else if (b) x = b - defaultStart;
        else if (c) x = c - defaultStop;
        else x = d - defaultStart - defaultStop;
    }
    return std::make_tuple(readruler::__round(x, prec), readruler::__round(y, prec),
        readruler::__round(z, prec));
}

[[deprecated]]
std::shared_ptr<const RailStationEvent> Diagram::findLastEvent(
    const RailStationEventList& lst, const TrainFilterCore& filter,
    const Direction& dir,
    RailStationEvent::Positions pos) const
{
    for (auto p = lst.crbegin(); p != lst.crend(); ++p) {
        if (!filter.check((*p)->line->train())) continue;
        if ((*p)->line->dir() == dir && (*p)->pos & pos) {
            return *p;
        }
    }
    return nullptr;
}

int readruler::IntervalReport::satisfiedCount()const
{
    int cnt = 0;
    for (auto p=raw.begin();p!=raw.end();++p) {
        if (p->second.first == stdInterval(p->second.second))
            cnt++;
    }
    return cnt;
}

int readruler::IntervalReport::stdInterval(TrainLine::IntervalAttachType type)const
{
    int res = interval;
    if (type & TrainLine::AttachStart)
        res += start;
    if (type & TrainLine::AttachStop)
        res += stop;
    return res;
}


bool Diagram::saveDefaultConfigs(const QString& filename) const
{
    QFile file(filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "Diagram::saveDefaultConfigs: WARNING: open file " << filename 
            << " failed" << Qt::endl;
        return false;
    }
    auto data = _defaultConfig.toJson();
    _defaultManager.toJson(data);
    QJsonDocument doc(data);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool Diagram::readDefaultConfigs(const QString& filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qWarning("Diagram::readDefaultConfigs: cannot open config file, will use built-in default.");
        _defaultManager.initDefaultTypes();
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject& obj = doc.object();
    _defaultConfig.fromJson(obj);
    _defaultManager.readForDefault(obj);
    return true;
}

bool Diagram::fromJson(const QString& filename)
{
    QFile f(filename);
    f.open(QFile::ReadOnly);
    if (!f.isOpen()) {
        qDebug() << "Diagram::fromJson: ERROR: open file " << filename << " failed. " << Qt::endl;
        return false;
    }
    auto contents = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(contents);
    bool flag = fromJson(doc.object());
    if (flag)
        _filename = filename;

    //2021.08.18：增加从trc读取的算法
    if (!flag) {
        f.seek(0);
        QTextStream fin(&f);
        flag = fromTrc(fin);
    }

    f.close();
    return flag;
}

bool Diagram::fromJson(const QJsonObject& obj)
{
    if (obj.empty())
        return false;
    railways().clear();

    //车次和Config直接转发即可
    _trainCollection.fromJson(obj, _defaultManager);
    bool flag = _config.fromJson(obj.value("config").toObject());
    if (!flag) {
        //缺配置信息，使用默认值
        _config = _defaultConfig;
    }
    _note = obj.value("markdown").toString();
    _version = obj.value("qetrc_version").toString();
    _releaseCode = obj.value("qetrc_release").toInt(qespec::RELEASE_CODE);

    //线路  line作为第一个，lines作为其他，不存在就是空
    auto rail0 = std::make_shared<Railway>();
    rail0->fromJson(obj.value("line").toObject());
    railways().append(rail0);

    //其他线路
    const QJsonArray& arrail = obj.value("lines").toArray();
    for (const auto& r : arrail) {
        auto tt = std::make_shared<Railway>();
        tt->fromJson(r.toObject());
        railways().append(tt);
    }

    //特殊：旧版排图标尺 （优先级低于rail中的）
    const auto& t = obj.value("config").toObject().value("ordinate");
    if (t.isString() && !t.toString().isEmpty()) {
        railways().at(0)->setOrdinate(t.toString());
        railways().at(0)->calStationYCoeff();
    }

    //新增 Page
    const QJsonArray& arpage = obj.value("pages").toArray();
    for (auto p = arpage.begin(); p != arpage.end(); ++p) {
        _pages.append(std::make_shared<DiagramPage>(p->toObject(), *this));
    }

    bindAllTrains();
    return true;
}

QJsonObject Diagram::toJson() const
{
    //车次信息表
    QJsonObject obj = _trainCollection.toJson();
    //线路信息
    if (!railways().isEmpty()) {
        auto p = railways().begin();
        obj.insert("line", (*p)->toJson());
        ++p;
        if (p != railways().end()) {
            QJsonArray arrail;
            for (; p != railways().end(); ++p) {
                arrail.append((*p)->toJson());
            }
            obj.insert("lines", arrail);
        }
    }
    //新增：Page的信息
    QJsonArray arpage;
    for (auto p : _pages) {
        arpage.append(p->toJson());
    }
    obj.insert("pages", arpage);

    //配置信息
    QJsonObject objconfig = _config.toJson();
    _trainCollection.typeManager().toJson(objconfig);
    obj.insert("config", objconfig);
    obj.insert("markdown", _note);
    obj.insert("qetrc_version", qespec::VERSION.data());
    obj.insert("qetrc_release", qespec::RELEASE_CODE);
    return obj;
}

QJsonObject Diagram::toSingleJson(std::shared_ptr<Railway> rail, bool localOnly) const
{
    //车次信息表
    QJsonObject obj = _trainCollection.toLocalJson(rail,localOnly);
    //线路信息
    obj.insert("line", rail->toJson());

    //配置信息
    QJsonObject objconfig = _config.toJson();
    _trainCollection.typeManager().toJson(objconfig);
    obj.insert("config", objconfig);
    obj.insert("markdown", _note);
    obj.insert("version", _version);
    return obj;
}

#undef TRC_WARNING
#define TRC_WARNING qDebug()<<"Diagram::toTrc: WARNING: "

bool Diagram::toTrc(const QString& filename, std::shared_ptr<Railway> rail, bool localOnly) const
{
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    if (!file.isOpen()) {
        TRC_WARNING << "open file " << filename << " failed." << Qt::endl;
        return false;
    }
    QTextStream fout(&file);
    //fout.setCodec("utf-8");
    using namespace etrc_consts;
    //线路
    fout << SPLIT_CIRCUIT << Qt::endl;
    fout << rail->name() << Qt::endl;
    fout << int(std::round(rail->railLength())) << Qt::endl;
    std::shared_ptr<RailStation> prev{};
    std::shared_ptr<Forbid> forbid;
    if (!rail->forbids().isEmpty()) forbid = rail->getForbid(0);
    for (auto p : rail->stations()) {
        fout << p->name.toSingleLiteral() << "," << int(std::round(p->mile)) << "," <<
            p->level << "," << (p->_show ? FALSE : TRUE)
            << ",,"
            << (p->prevSingle ? FALSE : TRUE)
            << ", 4, 1440, 1440, ";
        // todo: 天窗...
        if (forbid && prev) {
            //检索区间是否有天窗数据
            auto n = forbid->getNode(prev->name, p->name);
            if (!n->isNull()) {
                fout << n->beginTime.toString("hh:mm") << "-" <<
                    n->endTime.toString("hh:mm");
            }
        }
        fout << Qt::endl;

        prev = p;
    }

    //列车
    for (auto train : _trainCollection.trains()) {
        if (localOnly && !train->adapterFor(*rail))continue;
        fout << SPLIT_TRAIN << Qt::endl;
        fout << "trf2," << train->trainName().full() << "," << train->trainName().down() <<
            "," << train->trainName().up() << ",";
        if (train->hasRouting()) {
            auto rt = train->routing().lock();
            QString name = rt->name();
            name.replace("_", "-");
            fout << name << "_" << rt->trainIndex(train);
        }
        fout << Qt::endl;
        fout << train->starting().toSingleLiteral() << Qt::endl;
        fout << train->terminal().toSingleLiteral() << Qt::endl;
        for (const auto& t : train->timetable()) {
            //站名，到点，开点，true, NA, track
            fout << t.name.toSingleLiteral() << "," << t.arrive.toString("hh:mm:ss") << ","
                << t.depart.toString("hh:mm:ss") << ",true,NA," << t.track << Qt::endl;
        }
    }

    // Color
    fout << SPLIT_COLOR << Qt::endl;
    for (auto train : _trainCollection.trains()) {
        if (localOnly && !train->adapterFor(*rail))continue;
        const auto& color = train->pen().color();
        fout << train->trainName().full() << "," << color.red() << "," << color.green() << ","
            << color.blue() << Qt::endl;
    }
    fout << SPLIT_LINETYPE << Qt::endl;
    for (auto train : _trainCollection.trains()) {
        if (localOnly && !train->adapterFor(*rail))continue;
        // LineStyle 暂时不知道怎么转换，只管粗细
        fout << train->trainName().full() << ",0," << train->pen().widthF() << Qt::endl;
    }
    file.close();
    return true;
}

bool Diagram::toSinglePyetrc(const QString& filename, 
    std::shared_ptr<Railway> rail, bool localOnly) const
{
    QFile file(filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "Diagram::toSinglePyetrc: WARNING: open file " << filename << " failed. Nothing todo."
            << Qt::endl;
        return false;
    }
    QJsonDocument doc(toSingleJson(rail,localOnly));
    file.write(doc.toJson());
    file.close();
    return true;
}

bool Diagram::saveAs(const QString& filename)
{
    _filename = filename;
    return save();
}

bool Diagram::save() const
{
    QFile file(_filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "Diagram::save: WARNING: open file " << _filename << " failed. Nothing todo."
            << Qt::endl;
        return false;
    }
    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Compact));
    file.close();
    return true;
}

void Diagram::clear()
{
    _pages.clear();
    _trainCollection.clear(_defaultManager);
    railways().clear();
    _config = _defaultConfig;
    _note = "";
    _version = "";
}


