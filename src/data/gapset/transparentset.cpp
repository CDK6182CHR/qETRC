#include "transparentset.h"


gapset::TransparentGroup::TransparentGroup(const TrainGapType &type):
    GapGroupAbstract(TrainGap::posTypeToString(type).replace("\n"," ")),
    _type(type)
{

}

bool gapset::TransparentGroup::matches(const TrainGapType &type) const
{
    // 2023.06.16  for API V2: 
    // 匹配分为两部分。低位部分（API V1部分）需要完全相等；
    // 高位部分（Position部分）只要两段皆有非零交集即可。
    auto cross = type & _type;
    return (_type & TrainGap::BasicMask) == (type & TrainGap::BasicMask) &&
        bool(cross & TrainGap::LeftPositionMask) &&
        bool(cross & TrainGap::RightPositionMask);
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
