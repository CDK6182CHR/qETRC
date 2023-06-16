#include "gapgroupabstract.h"
#include "gapsetabstract.h"

#pragma once


namespace gapset {

class TransparentGroup:
        public GapGroupAbstract
{
    typename TrainGap::GapTypesV2 _type;
public:
    TransparentGroup(const TrainGapType& type);
    bool matches(const TrainGapType &type) const override;
};

class TransparentSet:
        public GapSetAbstract
{
public:
    void buildSet() override;
};

} // namespace gapset
