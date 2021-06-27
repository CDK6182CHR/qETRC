#pragma once

#include <memory>
#include <list>
#include <optional>
#include <tuple>
#include <QJsonObject>
#include <cstdint>
#include <QPair>
#include <QList>
#include "data/rail/rail.h"
#include "data/train/trainstation.h"
#include "data/common/direction.h"
#include "trainevents.h"


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



class TrainCollection;


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

    using AdaPtr = std::list<AdapterStation>::iterator;
    using ConstAdaPtr = std::list<AdapterStation>::const_iterator;

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
    const Train& train()const;

    TrainItem* item() { return _item; }
    void setItem(TrainItem* item) { _item = item; }

    inline const StationName& firstStationName()const { return _stations.front().trainStation->name; }
    inline const StationName& lastStationName()const { return _stations.back().trainStation->name; }

    inline std::shared_ptr<RailStation>
        firstRailStation(){
        return _stations.front().railStation.lock();
    }

    inline std::shared_ptr<const RailStation>
        firstRailStation()const {
        return _stations.front().railStation.lock();
    }

    inline auto firstTrainStation() {
        return _stations.front().trainStation;
    }

    inline std::shared_ptr<RailStation>
        lastRailStation(){
        return _stations.back().railStation.lock();
    }

    inline std::shared_ptr<const RailStation>
        lastRailStation()const {
        return _stations.back().railStation.lock();
    }

    inline bool isNull()const { return _stations.empty(); }

    bool startAtThis()const;

    bool endAtThis()const;

    /**
     * @brief listLineEvents 列出本运行线事件表
     * 遍历其他所有运行线获取数据。
     */
    LineEventList listLineEvents(const TrainCollection& coll)const;

private:

    /**
     * @brief listStationEvents
     * 列出每个站的到开时刻，这个很简单
     */
    void listStationEvents(LineEventList& res)const;

    /**
     * @brief detectPassStations  推定区间通过站时刻
     * 注意通过站时刻是属于IntervalEvent的
     * @param index  区间左端点下标，即插入位置
     * @param itr 区间左端点 迭代器
     */
    void detectPassStations(LineEventList& res, int index, ConstAdaPtr itr)const;

    /**
     * @brief eventsWithSameDir  同向列车事件表：越行、待避、共线
     * @param another  已知同向的另一段列车运行线
     */
    void eventsWithSameDir(LineEventList& res, const TrainLine& another, const Train& antrain)const;

    void eventsWithCounter(LineEventList& res, const TrainLine& another, const Train& antrain)const;

    /**
     * @brief 当前运行线的y最小值，等于首站或者末站（取决于行别）的y值
     * 前提条件是已经铺画过（y数值非0），否则会出错
     */
    double yMin()const;

    double yMax()const;

    /**
     * @brief yComp 两个站的y坐标比较
     * 如果y(st1)<y(st2)，返回-1.
     */
    int yComp(ConstAdaPtr st1, ConstAdaPtr st2)const;

    /**
     * @brief xComp 两个时刻，也即横坐标的比较
     * 如果tm1在tm2之前，返回-1.
     * 注意周期边界条件PBC下的歧义性。总是采取使得tm1, tm2距离绝对值最小的理解来消除歧义
     */
    int xComp(const QTime& tm1, const QTime& tm2)const;

    /**
     * @brief sameDirStep 同向的条件下，依据y坐标比较情况和行别，判定谁该进一步。
     * @param ycond: 0, +1, -1 三种情况
     * @param pme: this的迭代器  注意是引用，这里会更改！！
     * @param phe: 对方的迭代器 引用！
     * 保证传入的不是end()
     */
    void sameDirStep(int ycond, ConstAdaPtr& pme, ConstAdaPtr& phe,
        ConstAdaPtr& mylast, ConstAdaPtr& hislast,int& index)const;

    /**
     * @brief getPrevousPassedTime 推定前一站通过的时刻。
     * 用在同向事件表生成时，判定是否有区间越行情况。按照y值计算
     * @param st 跨越区间后的一站。这一站到其前序的区间，包含目标站。保证不是begin()
     * @param target 要推定时刻的目标车站。
     * @return 目标站的时刻，以【毫秒数】表示！！
     */
    int getPrevousPassedTime(ConstAdaPtr st, std::shared_ptr<RailStation> target)const;

    /**
     * @brief findIntervalIntersectionSameDIr 判断同向区间交点。
     * 解析几何方法判定交点是否存在，用斜率判断是越行还是待避。
     * @return 如果存在合法交点，返回交点的里程、时刻以及类型；否则返回std::nullopt
     */
    std::optional<std::tuple<double, QTime, TrainEventType>>
        findIntervalIntersectionSameDir(ConstAdaPtr mylast, ConstAdaPtr mythis,
            ConstAdaPtr hislast, ConstAdaPtr histhis)const;

};


//以前的版本...
/*
 * 暂定不维护Train指针或引用。
 * 这里Train&只用来读取车站信息，构建weak_ptr
 * 注意这里需要遍历一遍时刻表获得指针，因此较慢
 */
//    TrainLine(const QJsonObject& obj, Train& train);

//    void fromJson(const QJsonObject& obj, Train& train);
//    QJsonObject toJson()const;


