#pragma once

#include <memory>
#include <list>
#include <QJsonObject>
#include "data/rail/rail.h"
#include "data/train/trainstation.h"
#include "data/common/direction.h"

class Train;

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
    std::list<TrainStation>::iterator start, end;
    Direction dir;
    bool show;
    bool startLabel, endLabel;
    //todo: 可能需要指向Item的指针

    //根据需要写构造函数！

    TrainLine() = delete;

    /**
     * 暂定不维护Train指针或引用。
     * 这里Train&只用来读取车站信息，构建weak_ptr
     * 注意这里需要遍历一遍时刻表获得指针，因此较慢
     */
    TrainLine(const QJsonObject& obj, Train& train);

    void fromJson(const QJsonObject& obj, Train& train);
    QJsonObject toJson()const;
};


