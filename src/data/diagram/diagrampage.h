#pragma once

#include <memory>
#include <QList>
#include <QHash>


#include "data/train/train.h"
#include "config.h"
#include "data/diagram/routelinklayer.h"

class Railway;
class Diagram;
class TrainItem;
class TrainLine;
class Forbid;
class QGraphicsRectItem;

/**
 * @brief The LabelPositionInfo struct
 * 2021.07.02  移动到DiagramPage 中
 * 图形界面的部分的数据，为了方便，也写在RailStation中。
 * 车次标签占位信息。参考点的横坐标是放在key中的。
 */
struct LabelPositionInfo {
    /**
     * @brief height  标签高度
     */
    double height;

    /**
     * @brief left  参考点左侧的宽度
     */
    double left;

    /**
     * @brief right  参考点右侧的宽度
     */
    double right;

    LabelPositionInfo(double height_, double left_, double right_) :
        height(height_), left(left_), right(right_) {}
};

//std::multimap<double, LabelPositionInfo> _overLabels, _belowLabels;

/**
 * @brief The DiagramPage class
 * Diagram的一个/页面/视图  显示一组Railway的运行图
 * DiagramWidget针对这个类绘图
 * 注意不保存实际数据；只持有Item的指针
 * 
 * 2021.07.03：为了使Diagram的默认移动/拷贝行为正确，这里不能维护反向指针（引用）。
 * 2021.10.08：增加Config的值类型，构造时从Diagram里面拿。
 * 读取时，优先从文件里读，如果失败，再拷贝Diagram中的
 */
class DiagramPage
{
    Config _config;
    QList<std::shared_ptr<Railway>> _railways;
    QList<double> _startYs;
    QString _name;
    QString _note;
    QHash<TrainLine*, TrainItem*> _itemMap;
    QHash<const Forbid*, QList<QGraphicsRectItem*>> _forbidDMap, _forbidUMap;   //天窗的item映射，分为上下行
    
    /**
     * 每个站的标签高度数据表，从RailStation迁移过来
     */
    using label_map_t = std::multimap<double, LabelPositionInfo>;
    QHash<const RailStation*, label_map_t> _overLabels, _belowLabels;
    std::map<const RailStation*, RouteLinkLayerManager> _overLinks, _belowLinks;

public:
    DiagramPage(const Config& config, const QList<std::shared_ptr<Railway>>& railways,
        const QString& name, const QString& note = "");
    /**
     * 注意这个Diagram只是借过来构造的，并不维护
     */
    DiagramPage(const QJsonObject& obj, Diagram& _diagram);

    /**
     * 2021.12.18 NOTE: 构建副本应使用clone()方法。
     */
    DiagramPage(const DiagramPage& other) = delete;

    DiagramPage(DiagramPage&& other) = delete;

    /**
     * 删除指定Railway后的副本。用于删除线路的操作。
     * 保证：删除后非空；不检查；
     * 注意新生成的对象不包含Item
     */
    std::shared_ptr<DiagramPage> takenRailCopy(std::shared_ptr<Railway> rail)const;

    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}
    auto& startYs(){return _startYs;}
    auto railwayAt(int i)const { return _railways.at(i); }
    const QString& name()const { return _name; }
    void setName(const QString& s) { _name = s; }

    const auto& config()const { return _config; }
    auto& configRef() { return _config; }
    const auto& margins()const { return _config.margins; }

    const QString& note()const { return _note; }
    QString& note() { return _note; }
    void setNote(const QString& n) { _note = n; }

    auto& itemMap() { return _itemMap; }
    const auto& itemMap()const { return _itemMap; }

    QString railNameString()const;

    inline int railwayCount()const { return _railways.size(); }

    inline bool isNull()const { return _railways.isEmpty(); }

    /**
     * 所给线路的下标。通过地址比较。线性算法。
     * 如果找不到，返回-1
     */
    int railwayIndex(const Railway& rail)const;

    double railwayStartY(const Railway& rail)const;

    void fromJson(const QJsonObject& obj, Diagram& _diagram);
    QJsonObject toJson()const;

    void addItemMap(TrainLine* line, TrainItem* item) {
        _itemMap.insert(line, item);
    }

    TrainItem* getTrainItem(TrainLine* line);

    TrainItem* takeTrainItem(TrainLine* line);

    void clearTrainItems(const Train& train);

    void clearAllItems();

    void highlightTrainItems(const Train& train);

    void unhighlightTrainItems(const Train& train);

    void highlightTrainItemsWithLink(const Train& train);
    void unhighlightTrainItemsWithLink(const Train& train);

    QList<QGraphicsRectItem*>& dirForbidItem(const Forbid* forbid, Direction dir);

    void addForbidItem(const Forbid* forbid, Direction dir, QGraphicsRectItem* item);

    inline label_map_t::iterator overNullLabel(const RailStation* st) { return _overLabels[st].end(); }
    inline label_map_t::iterator belowNullLabel(const RailStation* st) { return _belowLabels[st].end(); }
    inline auto startingNullLabel(const RailStation* st, Direction dir) {
        return dir == Direction::Down ? overNullLabel(st) : belowNullLabel(st);
    }
    inline auto terminalNullLabel(const RailStation* st, Direction dir) {
        return dir == Direction::Down ? belowNullLabel(st) : overNullLabel(st);
    }
    inline auto& overLabels(const RailStation* st) { return _overLabels[st]; }
    inline auto& belowLabels(const RailStation* st) { return _belowLabels[st]; }
    inline auto& startingLabels(const RailStation* st, Direction dir) {
        return dir == Direction::Down ? overLabels(st) : belowLabels(st);
    }
    inline auto& terminalLabels(const RailStation* st, Direction dir) {
        return dir == Direction::Down ? belowLabels(st) : overLabels(st);
    }

    inline auto& overLinks(const RailStation* st) { return _overLinks[st]; }
    inline auto& belowLinks(const RailStation* st) { return _belowLinks[st]; }

    /**
    * 当关联的窗口被关闭（删除）时，清理掉相关联的Page中的Item指针。
    * 防止撤销/重做操作出问题。
    */
    void clearGraphics();

    /**
     * 在TrainItem清除LabelInfo前的保护
     * LabelInfo有可能已经全部被清楚了；这种情况下，析构的时候不要在那边删除。
     */
    inline bool hasLabelInfo()const { return !_belowLabels.isEmpty() || !_overLabels.isEmpty(); }
    
    inline bool hasLinkInfo()const { return !_belowLinks.empty() || !_overLinks.empty(); }

    bool containsRailway(std::shared_ptr<const Railway> rail)const;
    bool containsRailway(const Railway& railway)const;

    /**
     * 交换名称和note，但不交换其他
     */
    void swapBaseInfo(DiagramPage& other);

    /**
     * 交换所有数据；包括Item这些。
     */
    void swap(DiagramPage& other);

    /**
     * 列车是否在本运行图所含线路中铺画。
     */
    bool hasTrain(const Train& train)const;

    /**
     * 将指定线路从本运行图的线路表中移除。如果本线没有它，则不管。
     */
    void removeRailway(std::shared_ptr<Railway> rail);

    /**
     * 创建副本。2021年12月18日用于实现dulplicate功能
     * 实际上和直接调用一个构造函数没多大区别。
     * 因为图元部分（语义上）不能复制，所以没有写成拷贝构造。
     */
    std::shared_ptr<DiagramPage> clone()const;

};

//inline auto overNullLabel() { return _overLabels.end(); }
//inline auto belowNullLabel() { return _belowLabels.end(); }
//inline auto startingNullLabel(Direction dir) {
//    return dir == Direction::Down ? overNullLabel() : belowNullLabel();
//}
//inline auto terminalNullLabel(Direction dir) {
//    return dir == Direction::Down ? belowNullLabel() : overNullLabel();
//}
//inline auto& overLabels() { return _overLabels; }
//inline auto& belowLabels() { return _belowLabels; }
//inline auto& startingLabels(Direction dir) {
//    return dir == Direction::Down ? _overLabels : _belowLabels;
//}
//inline auto& terminalLabels(Direction dir) {
//    return dir == Direction::Down ? _belowLabels : _overLabels;
//}


