#pragma once

#include <memory>
#include <QList>
#include <QJsonObject>
#include <QHash>
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "routing.h"

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
    QList<std::shared_ptr<Routing>> _routings;

    /**
     * @brief 车次查找表  
     * 2021.06.30 全车次查找表还是改回用QString作为key。原因是读取交路等地方依赖这个的严格正确性
     * 注意全车次的QString形式也算进singleNameMap中
     */
    QHash<QString, std::shared_ptr<Train>> fullNameMap;
    QHash<QString, QList<std::shared_ptr<Train>>> singleNameMap;

    TypeManager _manager;
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
    TrainCollection(const QJsonObject& obj, const TypeManager& defaultManage);

    void fromJson(const QJsonObject& obj, const TypeManager& defaultManager);

    /**
     * @brief toJson  导出JSON
     * @return 原pyETRC.Graph对应的object，但缺Config等信息
     */
    QJsonObject toJson()const;

    auto& trains(){return _trains;}
    const auto& trains()const{return _trains;}
    auto& routings() { return _routings; }
    const auto& routings()const { return _routings; }

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
     * 使用full车次判定
     */
    bool trainNameExisted(const TrainName& name)const;

    /**
     * @brief trainFromFullName  全车次快速查找
     * @param name  全车次
     * 准常数时间
     */
    std::shared_ptr<Train> findFullName(const QString& name);

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

    auto& typeManager() { return _manager; }
    const auto& typeManager()const { return _manager; }

    inline auto size()const { return _trains.size(); }

    inline std::shared_ptr<Train> trainAt(int i) { return _trains.at(i); }
    inline std::shared_ptr<const Train> trainAt(int i)const { return _trains.at(i); }

    void clear(const TypeManager& defaultManager);

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


