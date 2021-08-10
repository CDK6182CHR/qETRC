#pragma once

#include <memory>
#include <QList>
#include <QJsonObject>
#include <QHash>
#include <QMap>
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
    QMap<std::shared_ptr<TrainType>, int> _typeCount;

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
     * 读取Diagram文件，但只要TrainCollection的部分。
     * 这个版本用来处理导入列车。
     */
    bool fromJson(const QString& filename, const TypeManager& defaultManager);

    /**
     * @brief toJson  导出JSON
     * @return 原pyETRC.Graph对应的object，但缺Config等信息
     */
    QJsonObject toJson()const;

    auto& trains(){return _trains;}
    const auto& trains()const{return _trains;}
    auto& routings() { return _routings; }
    const auto& routings()const { return _routings; }
    auto& typeCount() { return _typeCount; }
    const auto& typeCount()const { return _typeCount; }

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
     * 车次是否非空且不冲突
     */
    bool trainNameIsValid(const TrainName& name, std::shared_ptr<Train> train)const;

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

    /**
     * pyETRC.Graph.multiSearch()  模糊查找车次
     * 全车次或分方向车次包含目标串即可
     */
    QList<std::shared_ptr<Train>>
        multiSearchTrain(const QString& name);

    auto& typeManager() { return _manager; }
    const auto& typeManager()const { return _manager; }

    inline auto size()const { return _trains.size(); }

    inline std::shared_ptr<Train> trainAt(int i) { return _trains.at(i); }
    inline std::shared_ptr<const Train> trainAt(int i)const { return _trains.at(i); }

    void clear(const TypeManager& defaultManager);

    /**
     * 仅删除所有车次，不管其他的
     */
    void clearTrains();

    /**
     * 给TrainListWidget提供的删除API
     * 删除和返回指定列车的指针  注意更新映射表
     * 
     * 关于交路：这里做的是可撤销删除。原则上被“拿掉”的列车对象已经不再需要了，
     * 因此这边的数据不用删掉，方便撤销的时候恢复。但是必须把交路数据设为virtual。
     */
    std::shared_ptr<Train> takeTrainAt(int i);

    /**
     * 删除最后一个车次，并且支持撤销。
     * 用于撤销添加车次
     */
    std::shared_ptr<Train> takeLastTrain();

    /**
     * 用在导入车次时。
     * 删除车次，且不考虑撤销。直接把交路信息也删掉
     */
    void removeTrainAt(int i);

    /**
     * 按照index插入车次 用于撤销删除车次
     * ！！注意特殊操作：恢复交路信息。如果存在交路信息，则使得指向的结点恢复non-virtual状态
     * seealso: takeTrainAt
     */
    void insertTrainForUndo(int i, std::shared_ptr<Train> train);

    /**
     * 删除所有没有绑定到线路的车次。
     * 注意只能在绑定操作执行之后，否则相当于清空。
     * 用于导入车次。
     */
    void removeUnboundTrains();

    inline bool isNull()const { return _trains.isEmpty(); }

    bool routingNameExisted(const QString& name)const;

    QString validRoutingName(const QString& prefix);

    inline int trainCount()const { return _trains.size(); }

    /**
     * 返回指定列车下标；如果找不到，返回-1.
     * 线性查找
     */
    int getTrainIndex(std::shared_ptr<Train> train)const;

    TrainName validTrainFullName(const QString& prefix)const;

    void invalidateAllTempData();

private:
    /**
     * @brief addMapInfo
     * 添加新车次时，增加映射表信息
     * 同时完成列车类型统计；此操作结束后，保证类型非空
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


