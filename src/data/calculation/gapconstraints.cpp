#include "gapconstraints.h"

int GapConstraints::correlationRange() const
{
    int res=GLOBAL_DEFAULT;
    for(const auto& t:*this){
        if (t.second>res)
            res=t.second;
    }
    return res;
}

QString GapConstraints::toString() const
{
    QString res;
    int i = 0;
    for (auto itr = begin(); itr != end(); ++itr) {
        res.append(QObject::tr("%1. %2: %3 s\n").arg(++i)
            .arg(TrainGap::posTypeToString(itr->first).replace('\n',' '))
            .arg(itr->second));
    }
    return res;
}

bool GapConstraints::checkConflict(TrainGap::GapTypesV2 type, int secs) const
{
    auto posLeft = TrainGap::gapTypeToPosLeft(type);
    auto posRight = TrainGap::gapTypeToPosRight(type);
    auto posCross = (posLeft & posRight);

    if (posCross == RailStationEvent::Both) {
        // 拆解成两个间隔
        auto typePre = type;
        auto typePost = type;
        typePre &= ~(TrainGap::PostMask);
        typePost &= ~TrainGap::PreMask;
        return secs < at(typePre) || secs < at(typePost);
    }
    else if (posLeft == RailStationEvent::Both || posRight == RailStationEvent::Both) {
        // 直接转写成正则
        auto typeCan = type;
        if (posLeft == RailStationEvent::Both) {
            typeCan = TrainGap::setLeftPos(type, posCross);
        }
        else {
            typeCan = TrainGap::setRightPos(type, posCross);
        }
        return secs < at(typeCan);
    }
    else {
        // 基本情况
        return secs < at(type);
    }
}

int GapConstraints::maxConstraint(TrainGap::GapTypesV2 type) const
{
    auto posLeft = TrainGap::gapTypeToPosLeft(type);
    auto posRight = TrainGap::gapTypeToPosRight(type);
    auto posCross = (posLeft & posRight);

    if (posCross == RailStationEvent::Both) {
        // 拆解成两个间隔
        auto typePre = type;
        auto typePost = type;
        typePre &= ~(TrainGap::PostMask);
        typePost &= ~TrainGap::PreMask;
        return std::max(at(typePre), at(typePost));
    }
    else if (posLeft == RailStationEvent::Both || posRight == RailStationEvent::Both) {
        // 直接转写成正则
        auto typeCan = type;
        if (posLeft == RailStationEvent::Both) {
            typeCan = TrainGap::setLeftPos(type, posCross);
        }
        else {
            typeCan = TrainGap::setRightPos(type, posCross);
        }
        return  at(typeCan);
    }
    else {
        // 基本情况
        return at(type);
    }
}
