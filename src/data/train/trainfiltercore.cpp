#include "trainfiltercore.h"

#include "traintype.h"
#include "routing.h"
#include "train.h"
#include "traintag.h"


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
        if (p.match(n.full()).hasMatch() || p.match(n.down()).hasMatch() || p.match(n.up()).hasMatch()){
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
        if (p.match(n.full()).hasMatch() || p.match(n.down()).hasMatch() || p.match(n.up()).hasMatch()) {
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

bool TrainFilterCore::checkStarting(std::shared_ptr<const Train> train) const
{
    if (!useStarting) return true;
    foreach(const auto & reg, startings) {
        if (reg.match(train->starting().toSingleLiteral()).hasMatch()) {
            return true;
        }
    }
    return false;
}

bool TrainFilterCore::checkTerminal(std::shared_ptr<const Train> train) const
{
    if (!useTerminal) return true;
    foreach(const auto & reg, terminals) {
        if (reg.match(train->terminal().toSingleLiteral()).hasMatch()) {
            return true;
        }
    }
    return false;
}

bool TrainFilterCore::checkIncludeTag(std::shared_ptr<const Train> train) const
{
    if (!useIncludeTag) return true;
    return std::ranges::find_first_of(train->tags(), includeTags, 
        {}, [](const std::shared_ptr<TrainTag>& t) {return t->name(); }, {}
		) != train->tags().end();
}

bool TrainFilterCore::checkExcludeTag(std::shared_ptr<const Train> train) const
{
    // Special for EXCLUDE checking (similar to checkExclude): return false if the train is EXCLUDED.
    if (!useExcludeTag) return false;   // Special for EXCLUDE
    return std::ranges::find_first_of(train->tags(), excludeTags,
        {}, [](const std::shared_ptr<TrainTag>& t) {return t->name(); }, {}
	) != train->tags().end();
}

bool TrainFilterCore::check(std::shared_ptr<const Train> train) const
{
    bool res = (checkType(train)
        && checkRouting(train) && checkPassenger(train) && checkShow(train)
        && checkStarting(train) && checkTerminal(train) && checkIncludeTag(train)
        && !checkExclude(train) && !checkExcludeTag(train)
        ) || checkInclude(train);
    if (useInverse)return !res;
    return res;
}
