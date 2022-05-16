#include "crgroups.h"

#include <QDebug>

namespace gapset {
namespace cr {

bool GroupTrack::matches(const TrainGapTypePair &type) const
{
    // 2022.05.16：待避不能算
    return bool(type.second & TrainGap::LeftDown)
        == bool(type.second & TrainGap::RightDown) &&
        !bool(type.second & TrainGap::Avoid);
}

bool GroupMeet::matches(const TrainGapTypePair &type) const
{
    //return (bool(type.second & TrainGap::LeftDown) != bool(type.second & TrainGap::RightDown))
    //        && TrainGap::isTypeLeftFormer(type.second,type.first)
    //        && !TrainGap::isTypeRightFormer(type.second,type.first);
    return bool(type.second & TrainGap::LeftDown)
        != bool(type.second & TrainGap::RightDown) &&
        !bool(type.second & TrainGap::Avoid);
}

bool GroupArrive::matches(const TrainGapTypePair &type) const
{
    //return (bool(type.second & TrainGap::LeftDown) != bool(type.second & TrainGap::RightDown))
    //        && TrainGap::isTypeLeftFormer(type.second,type.first)
    //        && TrainGap::isTypeRightFormer(type.second,type.first);
    qDebug() << TrainGap::typeToString(type.second, type.first) << Qt::endl;
    bool a = (bool(type.second & TrainGap::LeftDown) != bool(type.second & TrainGap::RightDown)),
        b = TrainGap::isTypeLeftFormer(type.second, type.first),
        c = TrainGap::isTypeRightFormer(type.second, type.first);
    return a && b && c;
}


} // namespace cr
} // namespace gapset
