#pragma once

#include <memory>
#include <QVector>
#include <QSet>
#include <QJsonObject>

#include "train.h"
#include "itrainfilter.h"


class TrainFilter;
class Diagram;

/**
 * 列车筛选器中，不带任何图形界面和SIGLAL/SLOT的数据部分。
 * 包含筛选逻辑。本身不提供修改接口；只能通过TrainFilter来修改。
 */
class TrainFilterCore:
    public ITrainFilter
{
    friend class TrainFilter;

protected:
    //Diagram& diagram;

    bool useType=false, useInclude=false, useExclude=false;
    bool useRouting=false, showOnly=false, useInverse=false;
    TrainPassenger passengerType = TrainPassenger::Auto;

    QSet<std::shared_ptr<const TrainType>> types;
    QVector<QRegExp> includes, excludes;
    QSet<std::shared_ptr<const Routing>> routings;
    bool selNullRouting=false;


public:

    /**
     * 2022.04.30
     * 从private改为public
     * 直接构造出来的结果将是空白筛选器，对任何列车都通过。
     */
    explicit TrainFilterCore()=default;
    TrainFilterCore(const TrainFilterCore&) = delete;
    TrainFilterCore& operator=(const TrainFilterCore&) = delete;
    TrainFilterCore(TrainFilterCore&&) = delete;
    TrainFilterCore& operator=(TrainFilterCore&&) = delete;

    bool check(std::shared_ptr<const Train> train)const override;
private:
    bool checkType(std::shared_ptr<const Train> train)const;
    bool checkInclude(std::shared_ptr<const Train> train)const;
    bool checkExclude(std::shared_ptr<const Train> train)const;
    bool checkRouting(std::shared_ptr<const Train> train)const;
    bool checkPassenger(std::shared_ptr<const Train> train)const;
    bool checkShow(std::shared_ptr<const Train> train)const;

};
