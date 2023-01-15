#include "trainfiltercore.h"

#include "traintype.h"
#include "routing.h"

#if 0
TrainFilterCore::TrainFilterCore(Diagram& diagram_):
    diagram(diagram_)
{}
#endif

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
