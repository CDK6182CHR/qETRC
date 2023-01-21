#pragma once

#include <memory>
#include <QList>
#include <QJsonObject>
#include <QHash>
#include <QMap>

#include <deque>

#include "data/train/typemanager.h"
#include "data/diagram/diadiff.h"
#include "predeftrainfiltercore.h"   // not sure: is this neccesary?

//class PredefTrainFilterCore;
class Railway;
class Train;
class Routing;
class TrainType;
class RailCategory;
class TrainGroup;

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
#if 0
    QVector<std::shared_ptr<TrainGroup>> _groups;
#endif
    std::deque<std::unique_ptr<PredefTrainFilterCore>> _filters;

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

    /**
     * 导出与rail有交集的车次
     */
    QJsonObject toLocalJson(std::shared_ptr<Railway> rail, bool localOnly)const;

    auto& trains(){return _trains;}
    const auto& trains()const{return _trains;}
    auto& routings() { return _routings; }
    const auto& routings()const { return _routings; }
    auto& typeCount() { return _typeCount; }
    const auto& typeCount()const { return _typeCount; }
    auto routingAt(int i) { return _routings.at(i); }
#if 0
    auto& groups() { return _groups; }
    auto& groups()const { return _groups; }
#endif
    auto& filters(){return _filters;}

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
     * 清空列车和交路；除了typeManager外，和clear一样
     */
    void clearTrainsAndRoutings();

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

    /**
     * 2022年1月23日
     * 删除非运行图车次。用于导入车次。
     * 原实现removeUnboundTrains()并不合适，因为受到最大跨越站数的影响。
     */
    void removeNonLocal(const RailCategory& cat);

    inline bool isNull()const { return _trains.isEmpty(); }

    bool routingNameExisted(const QString& name, std::shared_ptr<Routing> ignore = {})const;

    QString validRoutingName(const QString& prefix);

    std::shared_ptr<const Routing> routingByName(const QString& name)const;

    inline int trainCount()const { return _trains.size(); }

    /**
     * 返回指定列车下标；如果找不到，返回-1.
     * 线性查找
     */
    int getTrainIndex(std::shared_ptr<Train> train)const;

    /**
     * 指定交路的序号，线性查找
     */
    int getRoutingIndex(std::shared_ptr<const Routing> routing)const;

    TrainName validTrainFullName(const QString& prefix)const;

    void invalidateAllTempData();

    int routingCount()const { return _routings.size(); }

    /**
     * 当发生大规模的类型变化时，直接刷新TypeCount
     */
    void refreshTypeCount();

    /**
     * 2021.10.17   更新车次查找表和类型统计表
     * 当列车信息更新后调用。调用之前，已经完成信息的修改。
     * info里面装的是旧版信息
     */
    void updateTrainInfo(std::shared_ptr<Train> train, std::shared_ptr<Train> info);

    /**
     * 2022.02.06  更新类型。
     * updateTrainInfo()的类型统计部分。
     * prev_type: 以前的类型。
     */
    void updateTrainType(std::shared_ptr<Train> train, std::shared_ptr<TrainType> prev_type);

    /**
     * 2022.02.06
     * 返回所有存在车次的类型名称。依据查找表。
     */
    QList<QString> typeNames()const;

    /**
     * 2022年2月8日
     * pyETRC.Graph.diffWith
     * 基于DP的运行图对比算法
     */
    diagram_diff_t diffWith(const TrainCollection& other);

    /**
     * 绑定到指定线路的列车集合
     */
    QList<std::shared_ptr<Train>> boundTrains(std::shared_ptr<Railway> railway);

    bool filterNameIsValid(const QString& name, const PredefTrainFilterCore* ignore=nullptr);

    QString validFilterName(const QString& prefix);

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
     * 2021.10.17  更新单个条目的单车次映射表。
     */
    void updateSingleNameMapItem(std::shared_ptr<Train> train,
        const QString& oldName, const QString& newName);

    /**
     * @brief resetMapInfo 重置所有映射表信息
     */
    void resetMapInfo();
};


