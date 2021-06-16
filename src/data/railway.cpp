#include "railway.h"


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


Railway::Railway(const QJsonObject &obj)
{
    fromJson(obj);
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
        if(p.isBare() || p.operator==(name)){
            return stationByName(name);
        }
    }
    return std::shared_ptr<RailStation>(nullptr);
}

const std::shared_ptr<RailStation> Railway::stationByGeneralName(const StationName &name) const
{
    const QList<StationName>& t=fieldMap.value(name.station());
    for(const auto& p:t){
        if(p.isBare() || p.operator==(name)){
            return stationByName(name);
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


