#pragma once

#include <memory>
#include <QList>
#include <list>

#include "data/train/train.h"
#include "data/rail/railway.h"
#include "data/diagram/config.h"
#include "trainline.h"

class TrainCollection;

/**
 * @brief 与线路数据相结合的列车信息
 * 持有列车的所有权 shared_ptr<Train>
 * 但不持有线路的所有权 weak_ptr<Railway>
 * 每次铺画重新生成对象！
 */
class TrainAdapter
{
    Railway& _railway;
    Train& _train;

    /**
     * 运行线数据描述以及对象指针
     * 对数据结构似乎没有特殊要求，暂定QList<shared_ptr>的结构
     * Item部分好好考虑，特别是I/O (暂时不实现手动Item的IO)
     * 注意默认的拷贝行为是不正确的
     */
    QList<std::shared_ptr<TrainLine>> _lines;
public:

    /**
     * @brief 将线路与列车绑定，生成相关信息
     * @param config  暂时不保存，只用来生成信息
     */
    TrainAdapter(Train& train, Railway& railway,
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

    inline bool isNull()const { return _lines.empty(); }

    inline auto& railway() { return _railway; }
    inline const auto& railway()const { return _railway; }
    inline auto& train() { return _train; }
    inline const auto& train()const { return _train; }
    inline auto& lines() { return _lines; }
    inline const auto& lines()const { return _lines; }

    /**
     * @brief clearItems
     * 清除所附的所有运行线指针
     */
    void clearItems();

    void highlightItems();

    void unhighlightItems();

    inline bool isInSameRailway(const TrainAdapter& another)const {
        return &_railway == &(another._railway);
    }

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

private:

    /**
     * @brief autoLines
     * 自动生成运行线数据  包括原来的bindToRail()功能
     */
    void autoLines(const Config& config);
};


