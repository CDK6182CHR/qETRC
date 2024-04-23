#include "predeftrainfiltercore.h"

#include "traintype.h"
#include "routing.h"
#include "data/train/traincollection.h"

#include <QJsonArray>

#define LOAD_BOOL(_Key) this->_Key=obj.value(#_Key).toBool()

void PredefTrainFilterCore::fromJson(const QJsonObject &obj, const TrainCollection& coll)
{
    _name=obj.value("name").toString();
    _note=obj.value("note").toString();

    types.clear();
    includes.clear();
    excludes.clear();
    routings.clear();
    startings.clear();
    terminals.clear();

    LOAD_BOOL(useType);
    LOAD_BOOL(useInclude);
    LOAD_BOOL(useExclude);
    LOAD_BOOL(useRouting);
    LOAD_BOOL(showOnly);
    LOAD_BOOL(useInverse);
    LOAD_BOOL(selNullRouting);
    LOAD_BOOL(useStarting);
    LOAD_BOOL(useTerminal);

    passengerType=static_cast<TrainPassenger>(obj.value("passengerType").toInt(1));

    const auto& tps=obj.value("types").toArray();
    for (const auto& t:tps){
        auto tp=coll.typeManager().find(t.toString());
        assert(tp);
        types.insert(tp);
    }

    const auto& arInc=obj.value("includes").toArray();
    foreach(const auto& t,arInc){
        includes.push_back(QRegularExpression(t.toString()));
    }

    const auto& arExc=obj.value("excludes").toArray();
    foreach(const auto& t,arExc){
        excludes.push_back(QRegularExpression(t.toString()));
    }

    const auto& arStart = obj.value("startings").toArray();
    foreach(const auto& t, arStart) {
        startings.push_back(QRegularExpression(t.toString()));
    }

    const auto& arTerm = obj.value("terminals").toArray();
    foreach(const auto& t, arTerm) {
        terminals.push_back(QRegularExpression(t.toString()));
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
    DUMP(useStarting);
    DUMP(useTerminal);
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

    QJsonArray arStarting;
    foreach(const auto& reg, startings) {
        arStarting.append(reg.pattern());
    }
    res.insert("startings", arStarting);

    QJsonArray arTerminal;
    foreach(const auto& reg, terminals) {
        arTerminal.append(reg.pattern());
    }
    res.insert("terminals", arTerminal);

    return res;
}

std::array<std::unique_ptr<const PredefTrainFilterCore>, PredefTrainFilterCore::MAX_FILTERS> PredefTrainFilterCore::sysFilters{};
std::array<QString, PredefTrainFilterCore::MAX_FILTERS> PredefTrainFilterCore::sysFilterNames{
    QObject::tr("旅客列车"),QObject::tr("已显示列车")
};

const PredefTrainFilterCore* PredefTrainFilterCore::getSysFilter(SysFilterId id)
{
    const auto& a = sysFilters.at(id);
    if (!a) {
        // create new 
        sysFilters[id] = makeSysFilter(id);
    }
    return sysFilters.at(id).get();
}

const QString& PredefTrainFilterCore::getSysFilterName(SysFilterId id)
{
    return sysFilterNames[id];
}

void PredefTrainFilterCore::swapWith(PredefTrainFilterCore& other)
{
    std::swap(*this, other);
}

std::unique_ptr<const PredefTrainFilterCore> PredefTrainFilterCore::makeSysFilter(SysFilterId id)
{
    std::unique_ptr<PredefTrainFilterCore> res(new PredefTrainFilterCore);
    switch (id)
    {
    case PredefTrainFilterCore::PassengerTrains: res->passengerType = TrainPassenger::True;
        break;
    case PredefTrainFilterCore::ShownTrains: res->showOnly = true;
        break;
    default:
        break;
    }
    return res;
}
