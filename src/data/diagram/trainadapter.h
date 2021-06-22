#pragma once

#include <memory>
#include <QList>
#include <list>

#include "data/train/train.h"
#include "data/rail/railway.h"
#include "data/diagram/config.h"
#include "trainline.h"



/**
 * @brief 与线路数据相结合的列车信息
 * 持有列车的所有权 shared_ptr<Train>
 * 但不持有线路的所有权 weak_ptr<Railway>
 * 每次铺画重新生成对象！
 */
class TrainAdapter
{
    const std::weak_ptr<Railway> _railway;
    const std::shared_ptr<Train> _train;

    /**
     * 运行线数据描述以及对象指针
     * 对数据结构似乎没有特殊要求，暂定QList<shared_ptr>的结构
     * Item部分好好考虑，特别是I/O (暂时不实现手动Item的IO)
     */
    QList<std::shared_ptr<TrainLine>> _lines;
public:

    /**
     * @brief 将线路与列车绑定，生成相关信息
     * @param config  暂时不保存，只用来生成信息
     */
    TrainAdapter(std::shared_ptr<Train> train, std::weak_ptr<Railway> railway,
                 const Config& config);

private:

    /**
     * @brief autoLines
     * 自动生成运行线数据  包括原来的bindToRail()功能
     * 2021.6.22  TODO here
     */
    void autoLines(std::shared_ptr<Train> train,std::weak_ptr<Railway> railway,
                   const Config& config);
};


