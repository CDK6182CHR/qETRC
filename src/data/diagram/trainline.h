#pragma once

#include <memory>
#include <list>
#include <QJsonObject>
#include "data/rail/rail.h"
#include "data/train/trainstation.h"
#include "data/common/direction.h"

class Train;

/**
 * @brief The AdapterNode struct
 * 一个铺画的车站
 */
struct AdapterStation{
    std::list<TrainStation>::iterator trainStation;
    std::weak_ptr<RailStation> railStation;
    AdapterStation(std::list<TrainStation>::iterator trainStation_,
        std::weak_ptr<RailStation> railStation_):
        trainStation(trainStation_),railStation(railStation_){}
};

/**
 * @brief TrainLine  车次运行线的数据描述  按照struct组织
 * 由TrainAdapter管理
 * 扩展原Train中Item的描述类
 * 2021.06.22  增加绑定序列类
 */
struct TrainLine
{
    std::list<AdapterStation> stations;
    Direction dir;
    bool show;
    bool startLabel, endLabel;
    //todo: 可能需要指向Item的指针

    //根据需要写构造函数！

    /**
     * 构造空的运行线，默认显示Item和双标签
     */
    TrainLine();
    TrainLine(const TrainLine&) = default;
    TrainLine(TrainLine&&) = default;

    inline void addStation(std::list<TrainStation>::iterator trainStation,
        std::weak_ptr<RailStation> rs) {
        stations.emplace_back(trainStation, rs);
    }

    inline void pop_back() {
        stations.pop_back();
    }

    inline auto count()const { return stations.size(); }

    void print()const;

    /*
     * 暂定不维护Train指针或引用。
     * 这里Train&只用来读取车站信息，构建weak_ptr
     * 注意这里需要遍历一遍时刻表获得指针，因此较慢
     */
//    TrainLine(const QJsonObject& obj, Train& train);

//    void fromJson(const QJsonObject& obj, Train& train);
//    QJsonObject toJson()const;
};


