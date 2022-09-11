#include "transparentset.h"


gapset::TransparentGroup::TransparentGroup(const TrainGapTypePair &type):
    GapGroupAbstract(TrainGap::posTypeToString(type.second,type.first).replace("\n"," ")),
    _type(type)
{

}

bool gapset::TransparentGroup::matches(const TrainGapTypePair &type) const
{
    return type==this->_type;
}

void gapset::TransparentSet::buildSet()
{
    // 2022.09.11  删除singleLine属性，暂定直接true
    auto alltypes=TrainGap::allPossibleGaps(true);
    for(auto type:alltypes){
        auto t=std::make_unique<TransparentGroup>(type);
        t->push_back(type);
        emplace_back(std::move(t));
    }
}
