#include "predeftrainfiltercore.h"

#include "traintype.h"
#include "routing.h"
#include "data/diagram/diagram.h"
#include "data/train/traincollection.h"

#define LOAD_BOOL(_Key) this->_Key=obj.value(#_Key).toBool()

void PredefTrainFilterCore::fromJson(const QJsonObject &obj, const TrainCollection& coll)
{
    _name=obj.value("name").toString();
    _note=obj.value("note").toString();

    types.clear();
    includes.clear();
    excludes.clear();
    routings.clear();

    LOAD_BOOL(useType);
    LOAD_BOOL(useInclude);
    LOAD_BOOL(useExclude);
    LOAD_BOOL(useRouting);
    LOAD_BOOL(showOnly);
    LOAD_BOOL(useInverse);
    LOAD_BOOL(selNullRouting);

    passengerType=static_cast<TrainPassenger>(obj.value("passengerType").toInt(1));

    const auto& tps=obj.value("types").toArray();
    foreach(const auto& t,tps){
        auto tp=coll.typeManager().find(t.toString());
        assert(tp);
        types.insert(tp);
    }

    const auto& arInc=obj.value("includes").toArray();
    foreach(const auto& t,arInc){
        includes.push_back(QRegExp(t.toString()));
    }

    const auto& arExc=obj.value("excludes").toArray();
    foreach(const auto& t,arExc){
        excludes.push_back(QRegExp(t.toString()));
    }

    const auto& arRout=obj.value("routings").toArray();
    foreach(const auto& t,arRout){
        auto rt=coll.routingByName(t.toString());
        assert(rt);
        routings.insert(rt);
    }
}


#define DUMP(_Key) res.insert(#_Key, this->_Key)


QJsonObject PredefTrainFilterCore::toJson() const
{
    QJsonObject res{
        {"name",_name},
        {"note",_note},
    };

    DUMP(useType);
    DUMP(useInclude);
    DUMP(useExclude);
    DUMP(useRouting);
    DUMP(showOnly);
    DUMP(useInverse);
    DUMP(selNullRouting);
    res.insert("passengerType", static_cast<int>(passengerType));

    QJsonArray arTypes;
    foreach(auto tp, types) {
        arTypes.append(tp->name());
    }
    res.insert("types", arTypes);

    QJsonArray arInclude;
    foreach(const auto& reg, includes) {
        arInclude.append(reg.pattern());
    }
    res.insert("includes", arInclude);

    QJsonArray arExclude;
    foreach(const auto& reg, excludes) {
        arExclude.append(reg.pattern());
    }
    res.insert("excludes", arExclude);

    QJsonArray arRouting;
    foreach(const auto& rt, routings) {
        arRouting.append(rt->name());
    }
    res.insert("routings", arRouting);
    return res;
}
