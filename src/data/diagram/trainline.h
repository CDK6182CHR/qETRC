#pragma once

#include <memory>
#include <list>
#include "data/rail/rail.h"
#include "data/train/trainstation.h"
#include "data/common/direction.h"

/**
 * @brief TrainLine  车次运行线的数据描述
 * 原Train中Item的描述类
 * 按照结构体组织
 */
struct TrainLine
{
    /**
     * @brief start
     * 车次中起始结束站的指针。由此可以进一步访问线路上的站。
     */
    std::list<TrainStation>::iterator start,end;
    Direction dir;
    bool show;
    bool start_label,end_label;
    //todo: 可能需要指向Item的指针
};


