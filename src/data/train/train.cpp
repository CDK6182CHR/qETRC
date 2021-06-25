#include "train.h"
#include <QDebug>

#include "data/rail/rail.h"
#include "data/diagram/trainline.h"
#include "data/diagram/trainadapter.h"


Train::Train(const TrainName &trainName,
             const StationName &starting,
             const StationName &terminal,
             TrainPassenger passenger):
    _trainName(trainName),_starting(starting),_terminal(terminal),
    _passenger(passenger),_show(true),_autoLines(true)
{

}

Train::Train(const QJsonObject &obj)
{
    fromJson(obj);
}

Train::Train(const Train& another):
    _trainName(another._trainName),_starting(another._starting),_terminal(another._terminal),
    _type(another._type),_ui(another._ui),
    _passenger(another._passenger),_show(another._show),
    _timetable(another._timetable),
    _autoLines(another._autoLines)
{
    //TrainAdapter not copied
}

Train::Train(Train&& another) noexcept:
    _trainName(std::move(another._trainName)), _starting(std::move(another._starting)),
    _terminal(std::move(another._terminal)),
    _type(std::move(another._type)), _ui(std::move(another._ui)),
    _passenger(another._passenger), _show(another._show),
    _timetable(std::move(another._timetable)),
    _autoLines(another._autoLines)
{
    //TrainAdapter not moved
}

void Train::fromJson(const QJsonObject &obj)
{
    const QJsonArray& archeci=obj.value("checi").toArray();
    _trainName.fromJson(archeci);

    _type=obj.value("type").toString();
    _starting=StationName::fromSingleLiteral( obj.value("sfz").toString());
    _terminal=StationName::fromSingleLiteral(obj.value("zdz").toString());
    _show=obj.value("shown").toBool();
    _passenger=static_cast<TrainPassenger>(obj.value("passenger").toInt());
    const QJsonArray& artable=obj.value("timetable").toArray();
    _autoLines = obj.value("autoItem").toBool(true);
    //if (!_autoLines) {
    //    _lines.clear();
    //    const QJsonArray& aritems = obj.value("itemInfo").toArray();
    //    for (auto p = aritems.begin(); p != aritems.end(); ++p) {
    //        _lines.append(std::make_shared<TrainLine>(p->toObject(), *this));
    //    }
    //}
    //todo: UI, circuit, item
    for (auto p=artable.cbegin();p!=artable.cend();++p){
        _timetable.emplace_back(p->toObject());
    }
}

QJsonObject Train::toJson() const
{
    QJsonArray ar;
    for(const auto& p:_timetable){
        ar.append(p.toJson());
    }
    
    QJsonObject obj{
        {"checi",_trainName.toJson()},
        {"type",_type},
        {"sfz",_starting.toSingleLiteral()},
        {"zdz",_terminal.toSingleLiteral()},
        {"shown",_show},
        {"passenger",static_cast<int>(_passenger)},
        {"timetable",ar},
        {"autoItem",_autoLines}
    };
    
    if (!_autoLines) {
        //QJsonArray items;
        //for (auto p = _lines.begin(); p != _lines.end(); ++p) {
        //    items.append((*p)->toJson());
        //}
        //obj.insert("itemInfo", items);
    }
    return obj;
}

void Train::appendStation(const StationName &name,
                          const QTime &arrive, const QTime &depart,
                          bool business, const QString &track, const QString &note)
{
    _timetable.emplace_back(name, arrive, depart, business, track, note);
}

void Train::prependStation(const StationName& name, 
    const QTime& arrive, const QTime& depart,
    bool business, const QString& track, const QString& note)
{
    _timetable.emplace_front(name, arrive, depart, business, track, note);
}

typename Train::StationPtr
    Train::findFirstStation(const StationName& name)
{
    auto p = _timetable.begin();
    for (; p != _timetable.end(); ++p) {
        if (p->name == name)
            break;
    }
    return p;
}

QList<typename Train::StationPtr> Train::findAllStations(const StationName& name)
{
    QList<StationPtr> res;
    for (auto p = _timetable.begin(); p != _timetable.end(); ++p) {
        if (p->name == name) {
            res.append(p);
        }
    }
    return res;
}

Train::ConstStationPtr Train::findFirstStation(const StationName& name) const
{
    auto p = _timetable.begin();
    for (; p != _timetable.end(); ++p) {
        if (p->name == name)
            break;
    }
    return p;
}

QList<Train::ConstStationPtr> Train::findAllStations(const StationName& name) const
{
    QList<ConstStationPtr> res;
    for (auto p = _timetable.begin(); p != _timetable.end(); ++p) {
        if (p->name == name) {
            res.append(p);
        }
    }
    return res;
}

Train::StationPtr Train::findFirstGeneralStation(const StationName &name)
{
    auto p=findFirstStation(name);
    if(p!=nullStation())
        return p;
    for(p=_timetable.begin();p!=_timetable.end();++p){
        if (p->name.equalOrBelongsTo(name))
            break;
    }
    return p;
}

QList<Train::StationPtr> Train::findAllGeneralStations(const StationName &name)
{
    QList<StationPtr> res;
    for (auto p = _timetable.begin(); p != _timetable.end(); ++p) {
        if (p->name.equalOrBelongsTo(name)) {
            res.append(p);
        }
    }
    return res;
}

std::shared_ptr<TrainAdapter> Train::bindToRailway(Railway& railway, const Config& config)
{
    //2021.06.24 新的实现 基于Adapter
    for (auto p = _adapters.begin(); p != _adapters.end(); ++p) {
        if (&((*p)->railway()) == &railway)
            return *p;
    }
    auto adp = std::make_shared<TrainAdapter>(*this, railway, config);
    if (!adp->isNull()) {
        _adapters.append(adp);
        return adp;
    }
    return nullptr;
}


std::shared_ptr<TrainAdapter> Train::updateBoundRailway(Railway& railway, const Config& config)
{
    //2021.06.24  基于Adapter新的实现
    for (auto p = _adapters.begin(); p != _adapters.end(); ++p) {
        if (&((*p)->railway()) == &railway) {
            TrainAdapter adp(*this, railway, config);
            if (!adp.isNull()) {
                //原位替代原有对象
                (*p)->operator=(std::move(adp));
                return *p;
            }
            else {
                _adapters.erase(p);
                return nullptr;
            }
        }
    }
    //没找到，则创建
    auto p = std::make_shared<TrainAdapter>(*this, railway, config);
    if (!p->isNull()) {
        _adapters.append(p);
        return p;
    }
    return nullptr;
}

void Train::unbindToRailway(const Railway& railway)
{
    for (auto p = _adapters.begin(); p != _adapters.end(); ++p) {
        if (&((*p)->railway()) == &railway) {
            _adapters.erase(p);
            return;
        }
    }
}




#if 0
void Train::bindToRailway(std::shared_ptr<Railway> railway)
{
    if (_boundRail.lock() == railway)
        return;
    else
        unbindToRailway();
    _boundRail=railway;
    auto last=nullStation();   //上一个成功绑定的车站
    std::shared_ptr<RailStation> raillast;
    auto p=_timetable.begin();
    Direction locDir=Direction::Undefined;
    int cnt=0;   //绑定计数器
    for(;p!=nullStation();++p){
        //在Railway中找车站是常数复杂度
        auto railst=railway->stationByGeneralName(p->name);
        if(railst){
            //非空指针表示搜索成功。现在考虑是否绑定
            //目前唯一阻止绑定的事由是经由方向不对
            //原则上，这是不大可能发生的事情；因此只有显示知道行别不对时，才拒绝绑定
            if (locDir==Direction::Undefined ||
                    railst->isDirectionVia(locDir)){
                //成功绑定到车站
                cnt++;
                p->bindToRailStation(railst);
                //现在：计算当前区间上下行情况
                if (raillast){
                    locDir=railway->gapDirection(raillast,railst);
                }
                if (cnt==2){
                    //第二个站时，还是检查一下第一个站的绑定是否合适
                    if(!raillast->isDirectionVia(locDir)){
                        //发现方向不对，撤销绑定
                        //相当于当前是第一个
                        cnt--;
                        last->unbindToRailStation();
                    }
                }
                raillast=railst;
                last=p;
            }
        }
    }
}

void Train::updateBoundRailway(std::shared_ptr<Railway> railway)
{
    unbindToRailway();
    bindToRailway(railway);
}

void Train::unbindToRailway()
{
    _boundRail.reset();
    for(auto& p:_timetable){
        p.unbindToRailStation();
    }
}

Train::StationPtr Train::prevBoundStation(StationPtr st)
{
    //注意reverse_iterator的各种坑
    auto s = std::make_reverse_iterator(st);
    for (; s != _timetable.rend(); ++s) {
        if (s->isBoundToRail()) {
            return --(s.base());
        }
    }
    return nullStation();
}

Train::StationPtr Train::nextBoundStation(StationPtr st)
{
    for (++st; !isNullStation(st); ++st) {
        if (st->isBoundToRail())
            return st;
    }
    return nullStation();
}

Train::ConstStationPtr Train::prevBoundStation(ConstStationPtr st) const
{
    auto s = std::make_reverse_iterator(st);
    for (; s != _timetable.rend(); ++s) {
        if (s->isBoundToRail()) {
            return --(s.base());
        }
    }
    return nullStation();
}

Train::ConstStationPtr Train::nextBoundStation(ConstStationPtr st) const
{
    for (++st; !isNullStation(st); ++st) {
        if (st->isBoundToRail())
            return st;
    }
    return nullStation();
}

Direction Train::stationDirection(const StationName& station)
{
    if (!isBoundToRailway())
        return Direction::Undefined;
    auto p = findFirstGeneralStation(station);
    if (isNullStation(p))
        return Direction::Undefined;
    else
        return stationDirection(p);
}

Direction Train::stationDirection(ConstStationPtr station)
{
    if (isNullStation(station) || !isBoundToRailway() ||
        !station->isBoundToRail())
        return Direction::Undefined;
    std::shared_ptr<Railway> rail = _boundRail.lock();
    auto prev = prevBoundStation(station);
    if (!isNullStation(prev)) {
        Direction d = rail->gapDirection(prev->boundRailStation().lock(),
            station->boundRailStation().lock());
        if (DirFunc::isValid(d))
            return d;
    }
    //只要没有返回就是左区间判定失败，改为右区间
    auto next = nextBoundStation(station);
    if (!isNullStation(next)) {
        Direction d = rail->gapDirection(station->boundRailStation().lock(),
            next->boundRailStation().lock());
        return d;
    }
    return Direction::Undefined;
}
#endif

Train Train::translation(TrainName name, int sec)
{
    Train train(*this);   //copy construct
    train.setTrainName(name);
    for(auto& p:train._timetable){
        p.arrive=p.arrive.addSecs(sec);
        p.depart=p.depart.addSecs(sec);
    }
    return train;
}

#if 0
void Train::removeNonLocal()
{
    if (!isBoundToRailway())
        return;
    auto p = _timetable.begin();
    while (p != _timetable.end()) {
        if (!p->isBoundToRail()) {
            auto p0 = p;
            ++p;
            _timetable.erase(p0);
        }
        else {
            ++p;
        }
    }
}
#endif

void Train::jointTrain(Train&& train, bool former)
{
    constexpr int max_cross_depth = 10;
    auto& table2 = train.timetable();
    if (former) {
        auto p1 = _timetable.begin();
        auto p2 = table2.rbegin();
        ++p1; ++p2;
        bool crossed = false;
        int cnt;
        for (cnt = 1; cnt < max_cross_depth; cnt++) {
            if (std::equal(_timetable.begin(), p1, p2.base(), table2.end(),TrainStation::nameEqual)) {
                crossed = true;
                break;
            }
            if (p1 == _timetable.end() || p2 == table2.rend())
                break;
            ++p1; ++p2;
        }
        if (crossed) {
            for (int i = 0; i < cnt; i++) {
                table2.pop_back();
            }
        }
        //交叉判定结束
        while (!table2.empty()) {
            TrainStation ts = std::move(table2.back());
            table2.pop_back();
            _timetable.push_front(std::move(ts));
        }
    }
    else {
        auto p1 = _timetable.rbegin();
        auto p2 = table2.begin();
        ++p1; ++p2;
        int cnt;
        bool cross = false;
        for (cnt = 1; cnt < max_cross_depth; cnt++) {
            if (std::equal(p1.base(), _timetable.end(), table2.begin(), p2, TrainStation::nameEqual)) {
                cross = true;
                break;
            }
            if (p1 == _timetable.rend() || p2 == table2.end())
                break;
            ++p1; ++p2;
        }
        if (cross)
            for (int i = 0; i < cnt; i++)
                table2.pop_front();
        while (!table2.empty()) {
            TrainStation st = std::move(table2.front());
            table2.pop_front();
            _timetable.push_back(std::move(st));
        }
    }
}

void Train::show() const
{
    _trainName.show();
    for(const auto& p:_timetable){
        qDebug() << p << Qt::endl;
    }
}

void Train::intervalExchange(Train& train2, StationPtr start1, StationPtr end1, 
    StationPtr start2, StationPtr end2)
{
    auto& table2 = train2._timetable;
    ++end1; ++end2;

    std::list<TrainStation> tmp;
    tmp.splice(tmp.begin(), _timetable, start1, end1);
    _timetable.splice(end1, table2, start2, end2);
    table2.splice(end2, tmp);
}

void Train::clearItems()
{
    for (auto p : _adapters) {
        p->clearItems();
    }
}
