#pragma once

#include <memory>
#include <QList>
#include <QJsonObject>
#include <QHash>
#include "data/train/train.h"

/**
 * @brief The TrainCollection class
 * qETRC新增一级抽象，列车的集合，承担原Graph类中不需要线路信息的部分
 * 以后的列车数据库可以继承这一部分实现
 * 这里也不保存Config.
 * 但列车类型系统由这里维护
 */
class TrainCollection
{
    QList<std::shared_ptr<Train>> _trains;
    //todo: 交路部分
    //todo: 类型系统

    /**
     * @brief 车次查找表
     * 注意全车次的QString形式也算进singleNameMap中
     */
    QHash<TrainName, std::shared_ptr<Train>> fullNameMap;
    QHash<QString, QList<std::shared_ptr<Train>>> singleNameMap;
public:
    TrainCollection() = default;
    TrainCollection(const TrainCollection&) = delete;
    TrainCollection(TrainCollection&&)noexcept = default;
    TrainCollection& operator=(const TrainCollection&) = delete;
    TrainCollection& operator=(TrainCollection&&)noexcept = default;

    /**
     * @brief TrainCollection  从JSON读取
     * @param obj  原pyETRC中Graph对应的object。数据不会全用掉
     */
    TrainCollection(const QJsonObject& obj);

    void fromJson(const QJsonObject& obj);

    /**
     * @brief toJson  导出JSON
     * @return 原pyETRC.Graph对应的object，但缺Config等信息
     */
    QJsonObject toJson()const;

    auto& trains(){return _trains;}
    const auto& trains()const{return _trains;}

    /**
     * @brief appendTrain 添加车次
     * 同时更新查找表。如果车次冲突，引起未定义行为
     */
    void appendTrain(std::shared_ptr<Train> train);

    /**
     * @brief removeTrain  删除车次 通过查找表
     * 如果找不到，不做任何事
     * 注意  线性复杂度
     */
    void removeTrain(std::shared_ptr<Train> train);

    /**
     * @brief trainNameExisted  所给车次是否存在，即是否允许添加的判据
     * 调用TrainName::operator==判定
     */
    bool trainNameExisted(const TrainName& name)const;

    /**
     * @brief trainFromFullName  全车次快速查找
     * @param name  全车次
     * 准常数时间
     */
    std::shared_ptr<Train> findFullName(const TrainName& name);

    /**
     * @brief trainsFromSingleName  单车次查找
     * @param name  full(), down(), up()之一的严格匹配
     * @return  所有符合条件的列表
     */
    QList<std::shared_ptr<Train>>
        findAllSingleName(const QString& name);

    /**
     * @brief findFirstSingleName  单车次查找
     * @return 第一个符合条件的列表
     */
    std::shared_ptr<Train> findFirstSingleName(const QString& name);

private:
    /**
     * @brief addMapInfo
     * 添加新车次时，增加映射表信息
     */
    void addMapInfo(const std::shared_ptr<Train>& t);

    /**
     * @brief removeMapInfo
     * 删除车次时，删除映射表信息
     */
    void removeMapInfo(std::shared_ptr<Train> t);

    /**
     * @brief resetMapInfo 重置所有映射表信息
     */
    void resetMapInfo();
};


