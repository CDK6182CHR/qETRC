#pragma once

#include "gapgroupabstract.h"

namespace gapset::cr {

/**
 * @brief The GroupTrack class
 * 追踪间隔，所有同向的间隔不论附加全部算进来
 */
class GroupTrack : public ::gapset::GapGroupAbstract
{
public:
    GroupTrack():
        ::gapset::GapGroupAbstract("追踪间隔",
                                   "所有同向列车间隔，不区分起停附加情况"){}
    virtual bool matches(const TrainGapTypePair &type) const override;
};

/**
 * @brief
 * 会车间隔，即左事件为出发性质，右事件为到达性质
 */
class GroupMeet: public ::gapset::GapGroupAbstract
{
public:
    GroupMeet():
        ::gapset::GapGroupAbstract("会车间隔",
                                   //"对向的前车到达（通过）和后车发车（通过）之间的间隔"
            "所有对向列车间隔"
        
        ){}
    virtual bool matches(const TrainGapTypePair &type) const override;
};

/**
 * @brief
 * 不同时到达间隔，即两边都是到达（通过）性质的事件
 */
class GroupArrive : public ::gapset::GapGroupAbstract
{
public:
    GroupArrive():
        ::gapset::GapGroupAbstract("不同时到达间隔",
                                   "车站一端两不同方向列车先后到达之间的间隔"){}
    virtual bool matches(const TrainGapTypePair &type) const override;
};



}

