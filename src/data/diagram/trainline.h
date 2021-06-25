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

class TrainItem;
class TrainAdapter;

/**
 * @brief TrainLine  车次运行线的数据描述  按照class组织
 * 由TrainAdapter管理
 * 扩展原Train中Item的描述类
 * 2021.06.22  增加绑定序列类
 * 原则上只允许TrainAdapter修改；外面的类只读。
 */
class TrainLine
{
    friend class TrainAdapter;
    TrainAdapter& _adapter;
    std::list<AdapterStation> _stations;
    Direction _dir;
    bool _show;
    bool _startLabel, _endLabel;
    TrainItem* _item;

public:
    /**
     * 构造空的运行线，默认显示Item和双标签
     */
    TrainLine(TrainAdapter& adapter);
    TrainLine(const TrainLine&) = default;
    TrainLine(TrainLine&&) = default;

    inline void addStation(std::list<TrainStation>::iterator trainStation,
        std::weak_ptr<RailStation> rs) {
        _stations.emplace_back(trainStation, rs);
    }

    inline void pop_back() {
        _stations.pop_back();
    }

    inline auto count()const { return _stations.size(); }

    void print()const;

    inline bool show()const { return _show; }

    inline Direction dir()const { return _dir; }
    inline bool startLabel()const { return _startLabel; }
    inline bool endLabel()const { return _endLabel; }
    auto& stations() { return _stations; }
    auto& adapter() { return _adapter; }
    Train& train();

    TrainItem* item() { return _item; }
    void setItem(TrainItem* item) { _item = item; }

    inline const StationName& firstStationName()const { return _stations.front().trainStation->name; }
    inline const StationName& lastStationName()const { return _stations.back().trainStation->name; }

    inline std::shared_ptr<RailStation>
        firstRailStation(){
        return _stations.front().railStation.lock();
    }

    inline auto firstTrainStation() {
        return _stations.front().trainStation;
    }

    inline std::shared_ptr<RailStation>
        lastRailStation(){
        return _stations.back().railStation.lock();
    }

    inline bool isNull()const { return _stations.empty(); }


    /*
     * 暂定不维护Train指针或引用。
     * 这里Train&只用来读取车站信息，构建weak_ptr
     * 注意这里需要遍历一遍时刻表获得指针，因此较慢
     */
//    TrainLine(const QJsonObject& obj, Train& train);

//    void fromJson(const QJsonObject& obj, Train& train);
//    QJsonObject toJson()const;
};


