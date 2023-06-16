#include "crgroups.h"

#include <QDebug>

namespace gapset {
namespace cr {

bool GroupTrack::matches(const TrainGapType &type) const
{
    // 2022.05.16：待避不能算
    // 2023.06.16：新增条件，只有同侧的才算
    return TrainGap::isSamePositionType(type) &&
        bool(type & TrainGap::LeftDown) == bool(type & TrainGap::RightDown) &&
        !bool(type & TrainGap::Avoid);
}

bool GroupMeet::matches(const TrainGapType &type) const
{
    return TrainGap::isSamePositionType(type) &&
        bool(type & TrainGap::LeftDown) != bool(type & TrainGap::RightDown) &&
        !bool(type & TrainGap::Avoid);
}

bool GroupArrive::matches(const TrainGapType &type) const
{
    //return (bool(type.second & TrainGap::LeftDown) != bool(type.second & TrainGap::RightDown))
    //        && TrainGap::isTypeLeftFormer(type.second,type.first)
    //        && TrainGap::isTypeRightFormer(type.second,type.first);
    bool a = (bool(type & TrainGap::LeftDown) != bool(type & TrainGap::RightDown)),
        b = TrainGap::isTypeLeftFormer(type),
        c = TrainGap::isTypeRightFormer(type);
    return !TrainGap::isSamePositionType(type) &&  a && b && c;
}


bool GroupDepart::matches(const TrainGapType& type) const
{
    bool a = (bool(type & TrainGap::LeftDown) != bool(type & TrainGap::RightDown)),
        b = TrainGap::isTypeLeftFormer(type),
        c = TrainGap::isTypeRightFormer(type);
    return !TrainGap::isSamePositionType(type) && a && !b && !c;
}

bool GroupArrDep::matches(const TrainGapType& type) const
{
    bool a = (bool(type & TrainGap::LeftDown) == bool(type & TrainGap::RightDown)),
        b = TrainGap::isTypeLeftFormer(type),
        c = TrainGap::isTypeRightFormer(type);
    return !TrainGap::isSamePositionType(type) && a && b && !c;
}

bool GroupDepArr::matches(const TrainGapType& type) const
{
    bool a = (bool(type & TrainGap::LeftDown) == bool(type & TrainGap::RightDown)),
        b = TrainGap::isTypeLeftFormer(type),
        c = TrainGap::isTypeRightFormer(type);
    return !TrainGap::isSamePositionType(type) && a && !b && c;
}

} // namespace cr
} // namespace gapset
