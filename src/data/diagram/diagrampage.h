#pragma once

#include <memory>
#include <QList>
#include <QHash>

#include "config.h"
#include "data/train/train.h"

class Railway;
class Diagram;
struct Config;
struct MarginConfig;
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
 */
class DiagramPage
{
    QList<std::shared_ptr<Railway>> _railways;
    QList<double> _startYs;
    QString _name;
    QHash<TrainLine*, TrainItem*> _itemMap;
    QHash<const Forbid*, QList<QGraphicsRectItem*>> _forbidDMap, _forbidUMap;   //天窗的item映射，分为上下行
    
    /**
     * 每个站的标签高度数据表，从RailStation迁移过来
     */
    using label_map_t = std::multimap<double, LabelPositionInfo>;
    QHash<const RailStation*, label_map_t> _overLabels, _belowLabels;

public:
    DiagramPage(const QList<std::shared_ptr<Railway>>& railways,
        const QString& name);
    /**
     * 注意这个Diagram只是借过来构造的，并不维护
     */
    DiagramPage(const QJsonObject& obj, Diagram& _diagram);

    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}
    auto& startYs(){return _startYs;}
    auto railwayAt(int i)const { return _railways.at(i); }
    const QString& name()const { return _name; }
    void setName(const QString& s) { _name = s; }

    auto& itemMap() { return _itemMap; }
    const auto& itemMap()const { return _itemMap; }

    QString railNameString()const;

    inline int railwayCount()const { return _railways.size(); }

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

    /**
    * 当关联的窗口被关闭（删除）时，清理掉相关联的Page中的Item指针。
    */
    void clearGraphics();

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


