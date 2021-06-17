#include "railway.h"
#include "../util/qeexceptions.h"
#include <cmath>
#include <algorithm>
#include <QPair>


void RailInfoNote::fromJson(const QJsonObject &obj)
{
    author=obj.value("author").toString();
    version=obj.value("version").toString();
    note=obj.value("note").toString();
}

QJsonObject RailInfoNote::toJson() const
{
    return QJsonObject({
       {"author",author},
       {"version",version},
       {"note",note}
    });
}


Railway::Railway(const QString& name):
    _name(name), numberMapEnabled(false)
{
}

Railway::Railway(const QJsonObject &obj):
    numberMapEnabled(false)
{
    fromJson(obj);
}

void Railway::moveStationInfo(Railway&& another)
{
    _name = std::move(another._name);
    _stations = std::move(another._stations);
    nameMap = std::move(another.nameMap);
    fieldMap = std::move(another.fieldMap);
    numberMapEnabled = another.numberMapEnabled;
    numberMap = std::move(another.numberMap);
}

void Railway::fromJson(const QJsonObject &obj)
{
    _name=obj.value("name").toString();
    _notes.fromJson(obj.value("notes").toObject());
    const QJsonArray& ar=obj.value("stations").toArray();
    for(auto t=ar.cbegin();t!=ar.cend();t++){
        _stations.append(std::make_shared<RailStation>(t->toObject()));
    }

    /*
     * TODOs:

        try:
            self.rulers
        except:
            self.rulers = []
        for ruler_dict in origin["rulers"]:
            new_ruler = Ruler(origin=ruler_dict,line=self)
            self.rulers.append(new_ruler)
        for route_dict in origin.get('routes',[]):
            r = Route(self)
            r.parseData(route_dict)
            self.routes.append(r)

        self.forbid.loadForbid(origin.get("forbid",None))
        self.forbid2.loadForbid(origin.get("forbid2",None))
        self.setNameMap()
        self.setFieldMap()
        self.verifyNotes()
        self.resetRulers()
      */
}

QJsonObject Railway::toJson() const
{
    auto obj= QJsonObject({
         {"name",_name},
         {"notes",_notes.toJson()}
       });
    QJsonArray ar;
    for(const auto& t:_stations){
        ar.append(t->toJson());
    }
    obj.insert("stations",ar);
    return obj;


        /*
        todos:
        info = {
        "rulers":[],
        "routes":[],
        "stations":self.stations,
        "forbid":self.forbid.outInfo(),
        "forbid2":self.forbid2.outInfo(),
        "notes":self.notes,
    }
    try:
        self.rulers
    except:
        self.rulers = []

    for ruler in self.rulers:
        info["rulers"].append(ruler.outInfo())
    for route in self.routes:
        info['routes'].append(route.outInfo())
    */
}

void Railway::appendStation(const StationName &name, double mile, int level, std::optional<double> counter, PassedDirection direction)
{
    auto&& t=std::make_shared<RailStation>(
                name,mile,level,counter,direction);
    _stations.append(t);
    addMapInfo(t);
}

void Railway::insertStation(int index, const StationName &name, double mile,
                            int level, std::optional<double> counter, PassedDirection direction)
{
    auto&& t=std::make_shared<RailStation>(name,mile,level,counter,direction);
    if(index==-1)
        _stations.append(t);
    else
        _stations.insert(index, t);
    addMapInfo(t);
}

std::shared_ptr<RailStation> Railway::stationByName(const StationName &name)
{
    return nameMap.value(name);
}

const std::shared_ptr<RailStation> Railway::stationByName(const StationName &name) const
{
    return nameMap.value(name);
}

std::shared_ptr<RailStation>
    Railway::stationByGeneralName(const StationName &name)
{
    const QList<StationName>& t=fieldMap.value(name.station());
    for(const auto& p:t){
        if(p.equalOrContains(name)){
            return stationByName(p);
        }
    }
    return std::shared_ptr<RailStation>(nullptr);
}

const std::shared_ptr<RailStation>
    Railway::stationByGeneralName(const StationName &name) const
{
    const QList<StationName>& t=fieldMap.value(name.station());
    for(const auto& p:t){
        if(p.equalOrContains(name)){
            return stationByName(p);
        }
    }
    return std::shared_ptr<RailStation>(nullptr);
}

bool Railway::containsStation(const StationName &name) const
{
    return nameMap.contains(name);
}

bool Railway::containsGeneralStation(const StationName &name) const
{
    if(!fieldMap.contains(name.station()))
        return false;
    const auto& t=fieldMap.value(name.station());
    for(const auto& p:t){
        if(p.isBare() || p==name)
            return true;
    }
    return false;
}

int Railway::stationIndex(const StationName &name) const
{
    if (numberMapEnabled){
        auto t=localName(name);
        if(numberMap.contains(t))
            return numberMap.value(t);
        return -1;
    }
    else{
        return stationIndexBrute(name);
    }
}

void Railway::removeStation(const StationName &name)
{
    if(numberMapEnabled){
        if(numberMap.contains(name)){
            int i=numberMap.value(name);
            _stations.removeAt(i);
            numberMap.remove(name);
            removeMapInfo(name);
            return;
        }
    }
    //QList.remove应当是线性算法，可能还不如直接手写来得快
    for(int i=0;i<_stations.count();i++){
        const auto& t=_stations[i];
        if(t->name == name){
            removeMapInfo(name);
            _stations.removeAt(i);
            break;
        }
    }
}

void Railway::adjustMileToZero()
{
    if(_stations.empty())
        return;
    double m0=_stations[0]->mile;
    std::optional<double> c0=_stations[0]->counter;
    for(auto& t:_stations){
        t->mile-=m0;
    }
    if(c0.has_value()){
        for(auto& t:_stations){
            if(t->counter.has_value()){
                t->counter.value()-=c0.value();
            }
        }
    }
}

bool Railway::isDownGap(const StationName& s1,const StationName& s2) const
{
    const auto& t1=stationByGeneralName(s1),
            t2=stationByGeneralName(s2);
    if(!t1 || ! t2)
        return true;
    return t1->mile <= t2->mile;
}

bool Railway::isDownGap(const std::shared_ptr<RailStation> &s1,
                        const std::shared_ptr<RailStation> &s2) const
{
    return s1->mile<=s2->mile;
}

double Railway::mileBetween(const StationName &s1,
                            const StationName &s2) const
{
    const auto& t1=stationByGeneralName(s1),
            t2=stationByGeneralName(s2);
    if(!t1)
        throw StationNotInRailException(s1);
    if(!t2)
        throw StationNotInRailException(s2);
    if (!isDownGap(t1,t2)){
        if(t1->counter.has_value()&&t2->counter.has_value()){
            return std::abs(t1->counter.value()-t2->counter.value());
        }
    }
    return std::abs(t1->mile-t2->mile);
}

bool Railway::isSplitted() const
{
    for (const auto& p : _stations) {
        if (p->direction == PassedDirection::DownVia ||
            p->direction == PassedDirection::UpVia)
            return true;
    }
    return false;
}


void Railway::changeStationName(const StationName& oldname, 
    const StationName& newname)
{
    auto p = stationByName(oldname);
    p->name = newname;
    //todo: 更新标尺天窗中站名

    //更新映射表
    removeMapInfo(oldname);
    addMapInfo(p);
}

double Railway::counterLength() const
{
    if (_stations.empty())
        return 0;
    const auto& last = _stations.last();
    return last->counter.has_value() ? 
        last->counter.value() : last->mile;
}

void Railway::reverse()
{
    double length = railLength(), ctlen = counterLength();
    for (auto p : _stations) {
        p->mile = length - p->mile;
        if (p->counter.has_value()) {
            p->counter = ctlen - p->counter.value();
        }
        else {
            p->counter = p->mile;
        }
        std::swap(p->counter.value(), p->mile);
        switch (p->direction) {
        case PassedDirection::DownVia:
            p->direction = PassedDirection::UpVia; break;
        case PassedDirection::UpVia:
            p->direction = PassedDirection::DownVia; break;
        default:break;
        }
    }
    std::reverse(_stations.begin(), _stations.end());
}

QList<QPair<StationName,StationName>> Railway::adjIntervals(bool down) const
{
    QList<QPair<StationName,StationName>> res;
    res.reserve(stationCount());   //预留空间

    if (empty()) return res;

    if (down) {
        auto p = _stations.cbegin();
        StationName last = (*p)->name;
        for (++p; p != _stations.cend(); ++p) {
            auto& q = *p;
            if (!q->isDownVia())
                continue;
            res.append(qMakePair(last, q->name));
            last = q->name;
        }
    }
    else {  //not down
        auto p = _stations.crbegin();
        StationName last = (*p)->name;
        for (++p; p != _stations.crend(); ++p) {
            auto& q = *p;
            if (!q->isUpVia())continue;
            res.append(qMakePair(last,q->name));
            last = q->name;
        }
    }
    return res;
}

void Railway::mergeCounter(const Railway& another)
{
    //注意插入会导致迭代器失效，因此不可用迭代器
    int i = 0;
    int j = another.stationCount() - 1;
    while (true) {
        if (i >= stationCount() || j < 0)
            break;
        const auto& si = _stations[i];
        const auto& sj = another.stations().at(j);
        if (si->name == sj->name) {
            si->direction = PassedDirection::BothVia;
            si->counter = another.railLength() - sj->mile;
            i++; j--;
        }
        else {
            if (!containsStation(sj->name)) {
                //上行单向站，插入处理
                double m = another.railLength() - sj->mile;
                insertStation(i, sj->name, m, sj->level, 
                    m, PassedDirection::UpVia);
                i++; j--;
            }
            else {
                //下行单向站
                i++;
            }
        }
    }
    //[对里程]的修正：使得零点和正里程一样
    if (empty())return;
    if (!_stations.at(0)->isUpVia()) {
        int i = 0;
        while (i < stationCount() && !_stations.at(i)->isUpVia())
            i++;
        if (i < stationCount()) {
            double m0 = _stations.at(i)->mile;
            for (int j = i; j < stationCount(); j++) {
                if (_stations.at(j)->counter.has_value()) {
                    _stations.at(j)->counter.value() += m0;
                }
            }
        }
    }
}

Railway Railway::slice(int start, int end) const
{
    Railway rail;
    rail._stations.reserve(end - start);
    for (int i = start; i < end; i++) {
        rail._stations.append(_stations.at(i));
    }
    // todo ruler
    rail.setMapInfo();
    return rail;
}

void Railway::jointWith(const Railway& another, bool former, bool reverse)
{
    if (reverse) {
        this->reverse();
    }
    if (former) {
        //对方在前
        double len = another.railLength();
        for (auto& p : _stations) {
            p->mile += len;
        }
        for (auto p = another.stations().crbegin();
            p != another.stations().crend(); p++) {
            if (!containsStation((*p)->name)) {
                insertStation(0, **p);
            }
        }
    }
    else {  //not former
        double length = railLength();
        double ctlen = counterLength();
        for (const auto& t : another.stations()) {
            appendStation(*t);
            auto& last = stations().last();
            last->mile += length;
            if (last->counter.has_value())
                last->counter.value() += ctlen;
        }
    }

    //todo: 标尺天窗...
}

void Railway::addMapInfo(const std::shared_ptr<RailStation> &st)
{
    //nameMap  直接添加
    const auto& n=st->name;
    nameMap.insert(n, st);
    fieldMap[n.station()].append(n);
}

void Railway::removeMapInfo(const StationName &name)
{
    nameMap.remove(name);

    auto t=fieldMap.find(name.station());
    if(t==fieldMap.end())
        return;
    else if(t.value().count()==1){
        fieldMap.remove(name.station());
    }else{
        QList<StationName>& lst=t.value();
        lst.removeAll(name);
    }
}

void Railway::setMapInfo()
{
    nameMap.clear();
    fieldMap.clear();
    nameMap.reserve(stationCount());
    fieldMap.reserve(stationCount());

    for (const auto& p : _stations) {
        nameMap.insert(p->name, p);
        fieldMap[p->name.station()].append(p->name);
    }
}

void Railway::enableNumberMap()
{
    if(numberMapEnabled)
        return;
    numberMap.clear();
    for(int i=0;i<_stations.count();i++){
        const auto& t=_stations[i];
        numberMap.insert(t->name,i);
    }
    numberMapEnabled=true;
}

void Railway::disableNumberMap()
{
    numberMap.clear();
    numberMapEnabled=false;
}

int Railway::stationIndexBrute(const StationName &name) const
{
    for(int i=0;i<_stations.count();i++){
        if(_stations[i]->name == name)
            return i;
    }
    return -1;
}

StationName Railway::localName(const StationName &name) const
{
    const auto& t=stationByGeneralName(name);
    if(t)
        return t->name;
    else
        return name;
}

void Railway::insertStation(int i, const RailStation& station)
{
    auto&& t = std::make_shared<RailStation>(station);    //copy constructed!!
    if (i == -1)
        _stations.append(t);
    else
        _stations.insert(i, t);
    addMapInfo(t);
}

void Railway::appendStation(const RailStation& station)
{
    auto&& t = std::make_shared<RailStation>(station);    //copy constructed!!
    _stations.append(t);
    addMapInfo(t);
}


