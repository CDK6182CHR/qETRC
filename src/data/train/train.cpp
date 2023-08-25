#include "train.h"
#include <QDebug>

#include "data/rail/railcategory.h"
#include "data/rail/railway.h"
#include "data/diagram/trainline.h"
#include "data/diagram/trainadapter.h"
#include "data/train/traintype.h"
#include "routing.h"
#include "util/utilfunc.h"
#include "typemanager.h"
#include "data/trainpath/trainpath.h"
#include "log/IssueManager.h"
#include <QFile>
#include <QTextStream>

Train::Train(const TrainName &trainName,
             const StationName &starting,
             const StationName &terminal,
             TrainPassenger passenger):
    _trainName(trainName),_starting(starting),_terminal(terminal),
    _passenger(passenger),_show(true),_autoLines(true)
{

}

Train::Train(const QJsonObject &obj, TypeManager& manager)
{
    fromJson(obj, manager);
}

Train::Train(const Train& another):
    std::enable_shared_from_this<Train>(another),
    _trainName(another._trainName),_starting(another._starting),_terminal(another._terminal),
    _type(another._type),_pen(another._pen),
    _passenger(another._passenger),_show(another._show),
    _timetable(another._timetable),
    _autoLines(another._autoLines)
{
    //TrainAdapter not copied
}

Train::Train(Train&& another) noexcept:
    _trainName(std::move(another._trainName)), _starting(std::move(another._starting)),
    _terminal(std::move(another._terminal)),
    _type(std::move(another._type)), _pen(std::move(another._pen)),
    _passenger(another._passenger), _show(another._show),
    _timetable(std::move(another._timetable)),
    _autoLines(another._autoLines)
{
    //TrainAdapter not moved
}

void Train::fromJson(const QJsonObject &obj, TypeManager& manager)
{
    const QJsonArray& archeci=obj.value("checi").toArray();
    _trainName.fromJson(archeci);

    setType(obj.value("type").toString(), manager);
    _starting=StationName::fromSingleLiteral( obj.value("sfz").toString());
    _terminal=StationName::fromSingleLiteral(obj.value("zdz").toString());
    _show=obj.value("shown").toBool(true);
    const auto& valpass = obj.value("passenger");
    if(valpass.isBool())
        _passenger = static_cast<TrainPassenger>(obj.value("passenger").toBool());
    else
        _passenger = static_cast<TrainPassenger>(obj.value("passenger").toInt(1));
    const QJsonArray& artable=obj.value("timetable").toArray();
    _autoLines = obj.value("autoItem").toBool(true);

    for (auto p=artable.cbegin();p!=artable.cend();++p){
        _timetable.emplace_back(p->toObject());
    }
    const QJsonObject ui = obj.value("UI").toObject();
    if (!ui.isEmpty()) {
        QPen pen = QPen(QColor(ui.value("Color").toString()), 
            ui.value("LineWidth").toDouble(type()->pen().widthF()),
            static_cast<Qt::PenStyle>(ui.value("LineStyle").toInt(Qt::SolidLine)));
        if (pen.width() == 0) {
            //线宽为0表示采用默认
            pen.setWidthF(_type->pen().widthF());
        }
        _pen = pen;
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
        {"type",_type->name()},
        {"sfz",_starting.toSingleLiteral()},
        {"zdz",_terminal.toSingleLiteral()},
        {"shown",_show},
        {"passenger",static_cast<int>(_passenger)},
        {"timetable",ar},
        {"autoItem",_autoLines}
    };

    QJsonObject ui;
    if (_pen.has_value()) {
        const auto& p = _pen.value();
        ui.insert("Color", p.color().name());
        ui.insert("LineWidth", p.widthF());
        ui.insert("LineStyle", p.style());   //新增
    }
    obj.insert("UI", ui);
    
    if (!_autoLines) {
        //QJsonArray items;
        //for (auto p = _lines.begin(); p != _lines.end(); ++p) {
        //    items.append((*p)->toJson());
        //}
        //obj.insert("itemInfo", items);
    }
    return obj;
}

bool Train::getIsPassenger()const
{
    switch (_passenger)
    {
    case TrainPassenger::True:return true;
    case TrainPassenger::False:return true;
    case TrainPassenger::Auto:return type()->isPassenger();
    default: return false;
    }
}

void Train::setLineShow(bool on)
{
    for (auto adp : _adapters) {
        for (auto line : adp->lines())
            line->setIsShow(on);
    }
}

void Train::setType(const QString& _typeName, TypeManager& manager)
{
    _type = manager.findOrCreate(_typeName);
}

const QPen& Train::pen() const
{
    //注意不能用value_or，因为返回的是值类型。
    if (_pen.has_value())
        return _pen.value();
    else
        return type()->pen();
}

bool Train::autoPen() const
{
    return !_pen.has_value();
}

void Train::appendStation(const StationName &name,
                          const QTime &arrive, const QTime &depart,
                          bool business, const QString &track, const QString &note)
{
    _timetable.emplace_back(name, arrive, depart, business, track, note);
}

void Train::setPen(const QPen& pen)
{
    _pen = pen;
}

void Train::resetPen()
{
    _pen.reset();
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

int Train::getPathIndex(const TrainPath* path) const
{
    for (int n = 0; n < _paths.size(); n++) {
        if (path == _paths.at(n)) {
            return n;
        }
    }
    return -1;
}

bool Train::pathsContainRailway(std::shared_ptr<const Railway> rail) const
{
    for (auto* p : _paths) {
        if (p->containsRailway(rail.get())) {
            return true;
        }
    }
    return false;
}

void Train::bindWithPath()
{
    qDebug() << "bindWithPath " << _trainName.full();
    IssueManager::get()->clearIssuesForTrain(this);
    _adapters.clear();
    for (const auto& p : _paths) {
        TrainAdapter::bindTrainByPath(shared_from_this(), p);
    }
    invalidateTempData();
}

std::shared_ptr<TrainAdapter> Train::bindToRailway(std::shared_ptr<Railway> railway, const Config& config)
{
    if (!_paths.empty()) {
        //qInfo() << "Train " << trainName().full() << " should be bound using TrainPath (not implemented)";
        // 2023.08.25: no process here (not sure); the bind with path operation should be called outer
        // (usually by qecmd::RebindTrainsByPath)
        //bindWithPath();
        return nullptr;
    }
    //2021.06.24 新的实现 基于Adapter
    for (auto p = _adapters.begin(); p != _adapters.end(); ++p) {
        if (((*p)->railway()) == railway) {
            invalidateTempData();
            return *p;
        }
    }
    auto adp = std::make_shared<TrainAdapter>(shared_from_this(), railway, config);
    if (!adp->isNull()) {
        _adapters.append(adp);
        invalidateTempData();
        return adp;
    }
    return nullptr;
}


void Train::updateBoundRailway(std::shared_ptr<Railway> railway, const Config& config)
{
    if (!_paths.empty()) {
        // 2023.08.25: no operation here (not sure)
        //bindWithPath();
        return;
    }
    //2021.06.24  基于Adapter新的实现
    //2021.07.04  TrainLine里面有Adapter的引用。不要move assign，直接删了重来好了
    unbindToRailway(railway);
    bindToRailway(railway, config);
}

void Train::unbindToRailway(std::shared_ptr<const Railway> railway)
{
    for (auto p = _adapters.begin(); p != _adapters.end(); ++p) {
        if (((*p)->railway()) == railway) {
            _adapters.erase(p);
            invalidateTempData();
            return;
        }
    }
}

bool Train::isBoundToRailway(std::shared_ptr<const Railway> railway)
{
    foreach(auto adp, _adapters) {
        if (adp->railway() == railway) {
            return true;
        }
    }
    return false;
}

void Train::clearBoundRailways()
{
    _adapters.clear();
    invalidateTempData();
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

void Train::adjustTimetable(int startIndex, int endIndex, bool includeFirst,
                            bool includeLast, int secs)
{
    if (!indexValid(startIndex) || !indexValid(endIndex) || startIndex > endIndex) {
        qDebug() << "Train::adjustTimetable: WARNING: invalid index: "
            << startIndex << ", " << endIndex << Qt::endl;
        return;
    }
    auto itstart = _timetable.begin(); std::advance(itstart, startIndex);
    auto itend = _timetable.begin(); std::advance(itend, endIndex);  // 右闭区间
    for (auto p = itstart; p != std::next(itend); ++p) {
        if(includeFirst || p!=itstart){
            //调整到达时刻
            p->arrive = p->arrive.addSecs(secs);
        }
        if (includeLast || p != itend) {
            p->depart = p->depart.addSecs(secs);
        }
    }
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

bool Train::isStartingStation(const AdapterStation* st)const
{
    if (st->trainStation->name != _starting)return false;
    for (auto adp : _adapters)
        if (adp->isFirstStation(st))
            return true;
    return false;
}

bool Train::isTerminalStation(const AdapterStation* st)const
{
    if (st->trainStation->name != _terminal)return false;
    for (auto adp : _adapters)
        if (adp->isLastStation(st))
            return true;
    return false;
}

void Train::show() const
{
    _trainName.show();
    int i = 0;
    for(const auto& p:_timetable){
        qDebug() << i++ << " " << p << Qt::endl;
    }
}

bool Train::isStartingStation(const TrainStation* st) const
{
    if (empty())return false;
    return st == &_timetable.front() && isStartingStation(st->name);
}

bool Train::isTerminalStation(const TrainStation* st) const
{
    if (empty())return false;
    return st == &_timetable.back() && isTerminalStation(st->name);
}

void Train::intervalExchange(Train& train2, StationPtr start1, StationPtr end1,
    StationPtr start2, StationPtr end2, bool includeStart, bool includeEnd)
{
    if (!includeStart) {
        std::swap(start1->arrive, start2->arrive);
    }
    if (!includeEnd) {
        std::swap(end1->depart, end2->depart);
    }

    auto& table2 = train2._timetable;
    ++end1; ++end2;

    std::list<TrainStation> tmp;
    tmp.splice(tmp.begin(), _timetable, start1, end1);
    _timetable.splice(end1, table2, start2, end2);
    table2.splice(end2, tmp);
}

void Train::setRouting(std::weak_ptr<Routing> rout, std::list<RoutingNode>::iterator node)
{
    resetRouting();   //设置之前清理掉旧的
    _routing = rout;
    _routingNode = node;
    //这里如果用shared_from_this会出错，不知道为什么
}

void Train::setRoutingSimple(std::weak_ptr<Routing> rout, std::list<RoutingNode>::iterator node)
{
    _routing=rout;
    _routingNode=node;
}

void Train::resetRouting()
{
    if (_routingNode.has_value()) {
        _routingNode.value()->makeVirtual();
        _routingNode.reset();
    }
    _routing.reset();
}

void Train::resetRoutingSimple()
{
    _routingNode.reset();
    _routing.reset();
}

const AdapterStation* Train::boundLast() const
{
    if (_timetable.empty())return nullptr;
    auto lastIter = _timetable.end(); --lastIter;
    for (auto p : _adapters) {
        auto* last = p->lastStation();
        if (last && last->trainStation == lastIter)
            return last;
    }
    return nullptr;
}

const AdapterStation* Train::boundTerminal() const
{
    if (_timetable.empty())return nullptr;
    auto lastIter = _timetable.end(); --lastIter;
    for (auto p : _adapters) {
        auto* last = p->lastStation();
        if (last && last->trainStation == lastIter &&
            isTerminalStation(last->trainStation->name))
            return last;
    }
    return nullptr;
}

std::shared_ptr<RailStation> Train::boundTerminalRail() const
{
    auto* last = boundTerminal();
    if (last)
        return last->railStation.lock();
    return nullptr;
}

const AdapterStation* Train::boundFirst() const
{
    if (_timetable.empty())
        return nullptr;
    for (auto p : _adapters) {
        auto* first = p->firstStation();
        if (first && first->trainStation == _timetable.begin())
            return first;
    }
    return nullptr;
}

const AdapterStation* Train::boundStarting() const
{
    if (_timetable.empty())
        return nullptr;
    for (auto p : _adapters) {
        auto* first = p->firstStation();
        if (first && first->trainStation == _timetable.begin() 
            && isStartingStation(first->trainStation->name))
            return first;
    }
    return nullptr;
}

const AdapterStation* Train::boundStartingAt(const Railway& rail) const
{
    if (_timetable.empty())
        return nullptr;
    for (auto p : _adapters) {
        if (p->railway().get() == &rail) {
            auto* first = p->firstStation();
            if (first && first->trainStation == _timetable.begin()
                && isStartingStation(first->trainStation->name))
                return first;
        }
    }
    return nullptr;
}

std::shared_ptr<RailStation> Train::boundStartingRail() const
{
    auto* first = boundStarting();
    if (first)
        return first->railStation.lock();
    return nullptr;
}


std::shared_ptr<RailStation> Train::boundStartingAtRail(const Railway& rail) const
{
    if (auto* first = boundStartingAt(rail)) {
        return first->railStation.lock();
    }
    else return nullptr;
}

double Train::localMile() 
{
    if (_locMile.has_value())
        return _locMile.value();
    double res = 0;
    for (auto p : _adapters)
        res += p->totalMile();
    _locMile = res;
    return res;
}

int Train::localSecs() 
{
    auto&& t = localRunStaySecs();
    return t.first + t.second;
}

std::pair<int, int> Train::localRunStaySecs()
{
    if (_locRunSecs.has_value())
        return std::make_pair(_locRunSecs.value(), _locStaySecs.value());
    int run = 0, stay = 0;
    for (auto p : _adapters) {
        auto d = p->runStaySecs();
        run += d.first;
        stay += d.second;
    }
    _locRunSecs = run;
    _locStaySecs = stay;
    return std::make_pair(run, stay);
}

int Train::localRunSecs()
{
    return localRunStaySecs().first;
}

double Train::localTraverseSpeed() 
{
    double mile = localMile();
    int secs = localSecs();
    //secs如果是0则得inf，C++中不会出问题
    return mile / secs * 3600;  
}

double Train::localTechSpeed()
{
    return localMile() / localRunSecs() * 3600;
}

void Train::invalidateTempData()
{
    _locMile = std::nullopt;
    _locRunSecs = std::nullopt;
    _locStaySecs = std::nullopt;
}

bool Train::timetableSame(const Train& other)const
{
    const auto& tab1 = _timetable, & tab2 = other._timetable;
    if (tab1.size() != tab2.size())
        return false;
    for (auto p = tab1.begin(), q = tab2.begin();
        p != tab1.end(); ++p, ++q) {
        if (*p != *q)
            return false;
    }
    return true;
}


void Train::swapTimetable(Train& other)
{
    std::swap(_timetable, other._timetable);
    invalidateTempData();
}

#define SWAP(_key) std::swap(_key,other._key)

void Train::swapBaseInfo(Train& other)
{
    std::swap(_trainName, other._trainName);
    std::swap(_starting, other._starting);
    std::swap(_terminal, other._terminal);
    SWAP(_type);
    SWAP(_passenger);
    SWAP(_pen);
}

#if 0
void Train::swapTimetableWithAdapters(Train& other)
{
    swapTimetable(other);
    std::swap(_adapters, other._adapters);
    for (int i = 0; i < _adapters.size(); i++) {
        auto ad1 = _adapters.at(i);
        auto ad2 = other._adapters.at(i);
        ad1->trainRef().swap(ad2->trainRef());
    }
}
#endif

void Train::swap(Train &other)
{
    swapTimetable(other);
    swapBaseInfo(other);
    std::swap(_adapters,other._adapters);
}

std::shared_ptr<const TrainAdapter> Train::adapterFor(const Railway& railway)const
{
    for(auto adp:_adapters){
        if((adp->railway().get())==&railway){
            return adp;
        }
    }
    return {};
}

std::shared_ptr<TrainAdapter> Train::adapterFor(const Railway& railway)
{
    for (auto adp : _adapters) {
        if ((adp->railway().get()) == &railway) {
            return adp;
        }
    }
    return {};
}

void Train::checkLinesShow()
{
    _show=anyLineShown();
}

int Train::adapterStationCount() const
{
    int  res=0;
    foreach(auto adp,_adapters){
        res+=adp->adapterStationCount();
    }
    return res;
}

int Train::lineCount() const
{
    int res=0;
    foreach(auto adp,_adapters){
        res+=adp->lines().size();
    }
    return res;
}

int Train::deltaDays(bool byTimetable) const
{
    if(empty()) return 0;
    if(byTimetable){
        auto pr=_timetable.begin(),p=_timetable.begin();
        int day=0;
        for(;p!=_timetable.end();pr=p,++p){
            if(pr!=p){
                // 比较上一区间
                if (p->arrive < pr->depart)
                    day++;
            }
            if (p->depart < p->arrive)
                day++;
        }
        return day;
    }else{
        if (_timetable.back().arrive >= _timetable.front().depart)
            return 0;
        else return 1;
    }
}

int Train::totalMinSecs() const
{
    if(empty()) return 0;
    return qeutil::secsTo(_timetable.front().arrive,_timetable.back().depart);
}

void Train::refreshStationFlags()
{
    // 第一次遍历：设置停车Flag
    for (auto& p: _timetable) {
        if (p.isStopped())
            p.flag = TrainStation::Stopped;
        else
            p.flag = TrainStation::NoFlag;
        if (p.business)
            p.flag |= TrainStation::Business;
    }
    // 第二次遍历：增加绑定站信息 遍历所有Line
    foreach(auto adp, _adapters) {
        foreach(auto line, adp->lines()) {
            for (auto& p : line->stations()) {
                p.trainStation->flag |= TrainStation::Bound;
            }
        }
    }
    //最后：补充始发站和终到站
    if (!_timetable.empty()) {
        if (isStartingStation(_timetable.front().name)) {
            _timetable.front().flag |= TrainStation::Starting;
        }
        if (isTerminalStation(_timetable.back().name)) {
            _timetable.back().flag |= TrainStation::Terminal;
        }
    }
}

bool Train::removeDetected()
{
    bool flag = false;
    for (auto p = _timetable.begin(); p != _timetable.end(); ) {
        if (p->note == QObject::tr("推定")) {
            flag = true;
            p = _timetable.erase(p);
        }
        else {
            ++p;
        }
    }
    return flag;
}

bool Train::autoBusiness()
{
    bool flag=false;
    foreach(auto adp,_adapters){
        foreach(auto line,adp->lines()){
            bool a=line->autoBusiness();
            flag=(flag||a);
        }
    }
    return flag;
}

void Train::autoBusinessWithoutBound(const Railway& rail, bool isPassenger)
{
    for (auto& st : _timetable) {
        auto railst = rail.stationByName(st.name);
        assert(railst);   // this MUST be valid in case of rulerpaint
        bool business = false;
        if (st.name == starting() || st.name == terminal() || st.isStopped()) {
            if ((isPassenger && railst->passenger) || (!isPassenger && railst->freight)) {
                business = true;
            }
        }
        st.business = business;
    }
}

bool Train::anyLineShown() const
{
    for (auto adp : _adapters) {
        for (auto line : adp->lines()) {
            if (line->show())
                return true;
        }
    }
    return false;
}

bool Train::indexValid(int i) const
{
    return 0 <= i && i < static_cast<int>(_timetable.size());
}

bool Train::isLocalTrain(const RailCategory& cat)const
{
    for (const auto& tst : _timetable) {
        foreach(auto rail, cat.railways()) {
            if (rail->containsGeneralStation(tst.name)) {
                return true;
            }
        }
    }
    return false;
}

bool Train::removeNonBound()
{
    refreshStationFlags();
    bool flag = false;
    for (auto itr = _timetable.begin(); itr != _timetable.end();) {
        if (!(itr->flag & TrainStation::Bound)) {
            itr = _timetable.erase(itr);
            flag = true;
        }
        else {
            ++itr;
        }
    }
    return flag;
}

void Train::clear()
{
    _timetable.clear();
    _starting = StationName();
    _terminal = StationName();
}

bool Train::ltName(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->trainName().full() < t2->trainName().full();
}

bool Train::ltStarting(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->starting() < t2->starting();
}

bool Train::ltTerminal(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->terminal() < t2->terminal();
}

bool Train::ltShow(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->isShow() < t2->isShow();
}

bool Train::ltTravSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2)
{
    return t1->localTraverseSpeed() < t2->localTraverseSpeed();
}

bool Train::ltTechSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2)
{
    return t1->localTechSpeed() < t2->localTechSpeed();
}

bool Train::ltMile(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2)
{
    return t1->localMile() < t2->localMile();
}

bool Train::ltType(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->type()->name() < t2->type()->name();
}

bool Train::gtName(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->trainName().full() > t2->trainName().full();
}

bool Train::gtStarting(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->starting() > t2->starting();
}

bool Train::gtTerminal(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->terminal() > t2->terminal();
}

bool Train::gtShow(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->isShow() > t2->isShow();
}

bool Train::gtType(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2)
{
    return t1->type()->name() > t2->type()->name();
}

bool Train::gtTravSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2)
{
    return t1->localTraverseSpeed() > t2->localTraverseSpeed();
}

bool Train::gtTechSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2)
{
    return t1->localTechSpeed() > t2->localTechSpeed();
}

#define TRC_WARNING qDebug()<<"Train::fromTrf: WARNING: "

std::shared_ptr<Train> Train::fromTrf(const QString& filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    [[unlikely]] if (!file.isOpen()) {
        TRC_WARNING << "open file " << filename << " failed" << Qt::endl;
        return {};
    }
    QTextStream fin(&file);
    //fin.setCodec("utf-8");
    QString line;

    // 标题行
    fin.readLineInto(&line);

    auto s = line.split(",");
    if (s.size() < 4) {
        TRC_WARNING << "Invalid train header: " << line << Qt::endl;
        return {};
    }

    const QString& star = fin.readLine(), term = fin.readLine();
    auto train = std::make_shared<Train>(TrainName(s.at(1), s.at(2), s.at(3)),
        star, term);
    
    while (!fin.atEnd()) {
        fin.readLineInto(&line);
        line = line.trimmed();
        //天津南,12:31,12:33,true,NA,0
           //站名, 到点, 开点, 营业, <不读取>, 站台
        auto t = line.split(",");
        if (t.size() >= 3) {
            bool business = true;
            if (t.size() >= 4) {
                business = (t.at(3) == "true");
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

    return train;
}

bool Train::gtMile(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2)
{
    return t1->localMile() > t2->localMile();
}
