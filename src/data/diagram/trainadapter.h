#pragma once

#include <memory>
#include <QVector>

#include "trainline.h"

class TrainCollection;
struct Config;
class TrainPath;

/**
 * @brief 与线路数据相结合的列车信息
 * 持有列车的所有权 shared_ptr<Train>
 * 但不持有线路的所有权 weak_ptr<Railway>
 * 每次铺画重新生成对象！
 */
class TrainAdapter
{
    std::weak_ptr<Railway> _railway;
    std::weak_ptr<Train> _train;

    /**
     * 运行线数据描述以及对象指针
     * 对数据结构似乎没有特殊要求，暂定QList<shared_ptr>的结构
     * Item部分好好考虑，特别是I/O (暂时不实现手动Item的IO)
     * 注意默认的拷贝行为是不正确的
     */
    QVector<std::shared_ptr<TrainLine>> _lines;

    /**
     * This version, not auto lines.
     */
    TrainAdapter(std::weak_ptr<Train> train, std::weak_ptr<Railway> railway);

public:

    /**
     * @brief 将线路与列车绑定，生成相关信息
     * @param config  暂时不保存，只用来生成信息
     */
    TrainAdapter(std::weak_ptr<Train> train, std::weak_ptr<Railway> railway,
                 const Config& config);
    TrainAdapter(const TrainAdapter&) = delete;
    TrainAdapter(TrainAdapter&&) = default;

    TrainAdapter& operator=(const TrainAdapter& another) = delete;

    /**
     * @brief move assign
     * 基于同一组railway, train对象的实现
     * 暂定不处理对象指针
     */
    TrainAdapter& operator=(TrainAdapter&& another)noexcept;

    void print()const;

    /**
     * 2022.05.28：用在交换Train指针的操作，
     * 主要是Train::swapTimetableWithAdapters()调用。
     * 注意此为危险操作，想好再调用！！
     */
    auto& trainRef() { return _train; }

    inline bool isNull()const { return _lines.empty(); }

    inline auto railway()const { return _railway.lock(); }
    std::shared_ptr<Train> train() { return _train.lock(); }
    std::shared_ptr<const Train> train()const { return _train.lock(); }
    inline auto& lines() { return _lines; }
    inline const auto& lines()const { return _lines; }

    inline bool isInSameRailway(const TrainAdapter& another)const {
        return _railway.lock() == another._railway.lock();
    }

    inline bool isInSameRailway(std::shared_ptr<const Railway> rail)const {
        return _railway.lock() == rail;
    }

    /**
     * 2023.08.22  New core implementation for binding train with the help of TrainPath.
     */
    static void bindTrainByPath(std::shared_ptr<Train> train, const TrainPath* path);

    /**
     * @brief listAdapterEvents 列出本次列车在本线的事件表
     * 逐段运行线计算。实际上只是个转发
     */
    AdapterEventList listAdapterEvents(const TrainCollection& coll)const;

    /**
     * 返回最后一个绑定的车站。
     * 如果为空（应该不存在这种情况），返回空指针
     */
    const AdapterStation* lastStation()const;

    const AdapterStation* firstStation()const;

    int totalSecs()const;
    std::pair<int, int> runStaySecs()const;
    double totalMile()const;

    /**
     * 注意TrainAdapter并没有show的属性。 这个只是方便一次性设置所有运行线的显示与否
     */
    void setIsShow(bool on);

    /**
     * 设置所有方向为dir的运行线的_show属性为on。
     * 非dir方向的运行线不受影响。
     */
    void setIsShowLineWise(Direction dir, bool on);

    /**
     * 入图行别，即第一运行线的行别
     */
    Direction firstDirection()const;

    Direction lastDirection()const;

    /**
     * 所给车站是否是当前的第一运行线的第一站。不考虑始发站的问题
     */
    bool isFirstStation(const AdapterStation* st)const;

    bool isLastStation(const AdapterStation* st)const;

    /**
     * 标尺排图中，初始化选择起始站使用。
     * 线性查找
     */
    std::pair<const AdapterStation*,std::shared_ptr<TrainLine>>
        stationByTrainLinear(std::list<TrainStation>::const_iterator st)const;

    int adapterStationCount()const;

    /**
     * 推定通过站时刻  pyETRC.data.Train.detectPassStation
     * precondition: 本次列车已经绑定到线路。
     * 不要求删除非铺画站点，但杂项可能导致插入的站点位置不对。
     * 直接修改this对象（以及相应的train对象）。
     * 主要的逻辑还是分发给TrainLine去搞
     * toStart, toEnd是指线路的首站末站。
     */
    void timetableInterpolation(std::shared_ptr<const Ruler> ruler, 
        bool toRailStart, bool toRailEnd, int prec);

    /**
     * 用于时刻插值的相对误差计算。
     */
    double relativeError(std::shared_ptr<const Ruler> ruler)const;

    /**
     * 推定通过站时刻 但使用事件表算法
     * 同样要求已绑定到线路，且直接修改this指向对象。
     * @return 新插入车站个数
     */
    int timetableInterpolationSimple();

private:

    /**
     * @brief autoLines
     * 自动生成运行线数据  包括原来的bindToRail()功能
     */
    void autoLines(const Config& config);

    /**
     * Return the relative position of the station comparing to the mile range (specified by segment).
     * Specifically: 
     * -1  if the station is encountered BEFORE the range, 
     * 0   if the station is in the range, and
     * +1  if the station is encountered AFTER the range.
     * Here, before/after is in the view of the train running direction.
     * Mind: how to process the counter mile?
     */
    static int stationInSegment(double mile_start, double mile_end, Direction dir, const RailStation& st);
};


