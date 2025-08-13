#pragma once

#include <memory>
#include <QVector>
#include <QSet>
#include <QJsonObject>

#include "trainpassenger.h"
#include "itrainfilter.h"

class Routing;
class TrainType;

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
    friend class TrainFilterBasicWidget;

protected:
    //Diagram& diagram;

    bool useType=false, useInclude=false, useExclude=false;
    bool useStarting = false, useTerminal = false;
    bool useRouting=false, showOnly=false, useInverse=false;
	bool useIncludeTag = false, useExcludeTag = false;
    TrainPassenger passengerType = TrainPassenger::Auto;

    QSet<std::shared_ptr<const TrainType>> types;
    QVector<QRegularExpression> includes, excludes;
    QVector<QRegularExpression> startings, terminals;
	QVector<QString> includeTags, excludeTags;
    QSet<std::shared_ptr<const Routing>> routings;
    bool selNullRouting=false;


public:

    /**
     * 2022.04.30
     * 从private改为public
     * 直接构造出来的结果将是空白筛选器，对任何列车都通过。
     */
    explicit TrainFilterCore()=default;
    TrainFilterCore(const TrainFilterCore&) = default;
    TrainFilterCore& operator=(const TrainFilterCore&) = default;
    TrainFilterCore(TrainFilterCore&&) = default;
    TrainFilterCore& operator=(TrainFilterCore&&) = default;

    bool check(std::shared_ptr<const Train> train)const override;
private:
    bool checkType(std::shared_ptr<const Train> train)const;
    bool checkInclude(std::shared_ptr<const Train> train)const;
    bool checkExclude(std::shared_ptr<const Train> train)const;
    bool checkRouting(std::shared_ptr<const Train> train)const;
    bool checkPassenger(std::shared_ptr<const Train> train)const;
    bool checkShow(std::shared_ptr<const Train> train)const;
    bool checkStarting(std::shared_ptr<const Train> train)const;
    bool checkTerminal(std::shared_ptr<const Train> train)const;
	bool checkIncludeTag(std::shared_ptr<const Train> train) const;
	bool checkExcludeTag(std::shared_ptr<const Train> train) const;
};
