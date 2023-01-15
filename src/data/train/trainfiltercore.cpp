#include "trainfiltercore.h"

#include "traintype.h"
#include "routing.h"

TrainFilterCore::TrainFilterCore(Diagram& diagram_):
    diagram(diagram_)
{}

#define DUMP(_Key) res.insert(#_Key, this->_Key)

void TrainFilterCore::fromJson(const QJsonObject& obj)
{

}

QJsonObject TrainFilterCore::toJson() const
{
    QJsonObject res;
    DUMP(useType);
    DUMP(useInclude);
    DUMP(useExclude);
    DUMP(useRouting);
    DUMP(showOnly);
    DUMP(useInverse);
    DUMP(selNullRouting);
    res.insert("passengerType", static_cast<int>(TrainPassenger::Auto));
    
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

bool TrainFilterCore::checkType(std::shared_ptr<const Train> train) const
{
    if(!useType) return true;
    return types.contains(train->type());
}

bool TrainFilterCore::checkInclude(std::shared_ptr<const Train> train) const
{
    if (!useInclude) return false;   // 这个特殊
    foreach (auto& p, includes){
        const auto& n=train->trainName();
        if (p.indexIn(n.full())==0 || p.indexIn(n.down())==0 || p.indexIn(n.up())==0){
            return true;
        }
    }
    return false;
}

bool TrainFilterCore::checkExclude(std::shared_ptr<const Train> train) const
{
    if(!useExclude) return false;
    foreach (auto& p, excludes){
        const auto& n=train->trainName();
        if (p.indexIn(n.full())==0 || p.indexIn(n.down())==0 || p.indexIn(n.up())==0){
            return true;
        }
    }
    return false;
}

bool TrainFilterCore::checkRouting(std::shared_ptr<const Train> train) const
{
    if (!useRouting) return true;
    if (train->hasRouting()){
        return routings.contains(train->routing().lock());
    }else{
        return selNullRouting;
    }
}

bool TrainFilterCore::checkPassenger(std::shared_ptr<const Train> train) const
{
    switch (passengerType)
    {
    case TrainPassenger::False:return !train->getIsPassenger();
        break;
    case TrainPassenger::Auto: return true;
        break;
    case TrainPassenger::True:return train->getIsPassenger();
        break;
    default:return false;
        break;
    }
}

bool TrainFilterCore::checkShow(std::shared_ptr<const Train> train) const
{
    if (!showOnly) return true;
    else return train->isShow();
}

bool TrainFilterCore::check(std::shared_ptr<const Train> train) const
{
    bool res = (checkType(train)
        && checkRouting(train) && checkPassenger(train) && checkShow(train)
        && !checkExclude(train)) || checkInclude(train);
    if (useInverse)return !res;
    return res;
}
