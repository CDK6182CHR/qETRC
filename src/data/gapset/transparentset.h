#include "gapgroupabstract.h"
#include "gapsetabstract.h"

#pragma once


namespace gapset {

class TransparentGroup:
        public GapGroupAbstract
{
    TrainGapTypePair _type;
public:
    TransparentGroup(const TrainGapTypePair& type);
    bool matches(const TrainGapTypePair &type) const override;
};

class TransparentSet:
        public GapSetAbstract
{
public:
    void buildSet() override;
};

} // namespace gapset
