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
                                   "所有同侧同向列车间隔，不区分起停附加情况"){}
    virtual bool matches(const TrainGapType &type) const override;
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
            "所有同侧对向列车间隔"
        
        ){}
    virtual bool matches(const TrainGapType &type) const override;
};

/**
 * @brief
 * 不同时到达间隔，即两边都是到达（通过）性质的事件
 * 2023.06.16  API V2  正式启用；注意仅用于对侧的。
 */
class GroupArrive : public ::gapset::GapGroupAbstract
{
public:
    GroupArrive():
        ::gapset::GapGroupAbstract("不同时到达间隔",
                                   "车站对侧两不同方向列车先后到达之间的间隔"){}
    virtual bool matches(const TrainGapType &type) const override;
};


class GroupDepart : public ::gapset::GapGroupAbstract
{
public:
    GroupDepart():
        ::gapset::GapGroupAbstract("不同时出发间隔",
            "车站对侧两不同方向列车先后出发之间的间隔"){}
    virtual bool matches(const TrainGapType& type)const override;
};


class GroupArrDep :public ::gapset::GapGroupAbstract
{
public:
    GroupArrDep():
        ::gapset::GapGroupAbstract("不同时到发间隔","车站对侧同方向列车到达、出发之间隔"){}
    virtual bool matches(const TrainGapType& type)const override;
};


class GroupDepArr :public ::gapset::GapGroupAbstract
{
public:
    GroupDepArr():
        ::gapset::GapGroupAbstract("不同时发到间隔","车站对侧同方向列车出发、到达之间隔"){}
    virtual bool matches(const TrainGapType& type)const override;
};


}

