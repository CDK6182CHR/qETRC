#pragma once

#include <memory>
#include <QVector>
#include <QSet>

#include "train.h"

class TrainFilter;
class Diagram;

/**
 * 列车筛选器中，不带任何图形界面和SIGLAL/SLOT的数据部分。
 * 包含筛选逻辑。本身不提供修改接口；只能通过TrainFilter来修改。
 */
class TrainFilterCore
{
    friend class TrainFilter;
    Diagram& diagram;

    bool useType, useInclude, useExclude;
    bool useRouting, showOnly, useInverse;
    TrainPassenger passengerType;

    QSet<std::shared_ptr<const TrainType>> types;
    QVector<QRegExp> includes, excludes;
    QSet<std::shared_ptr<const Routing>> routings;
    bool selNullRouting;

    explicit TrainFilterCore(Diagram& diagram_);
public:
    TrainFilterCore(const TrainFilterCore&) = delete;
    TrainFilterCore& operator=(const TrainFilterCore&) = delete;
    TrainFilterCore(TrainFilterCore&&) = delete;
    TrainFilterCore& operator=(TrainFilterCore&&) = delete;

    bool check(std::shared_ptr<const Train> train)const;
private:
    bool checkType(std::shared_ptr<const Train> train)const;
    bool checkInclude(std::shared_ptr<const Train> train)const;
    bool checkExclude(std::shared_ptr<const Train> train)const;
    bool checkRouting(std::shared_ptr<const Train> train)const;
    bool checkPassenger(std::shared_ptr<const Train> train)const;
    bool checkShow(std::shared_ptr<const Train> train)const;

};
