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
 * @brief The DiagramPage class
 * Diagram的一个/页面/视图  显示一组Railway的运行图
 * DiagramWidget针对这个类绘图
 * 注意不保存实际数据；只持有Item的指针
 */
class DiagramPage
{
    Diagram & _diagram;
    QList<std::shared_ptr<Railway>> _railways;
    QList<double> _startYs;
    QString _name;
    QHash<TrainLine*, TrainItem*> _itemMap;
    QHash<const Forbid*, QList<QGraphicsRectItem*>> _forbidDMap, _forbidUMap;   //天窗的item映射，分为上下行
public:
    DiagramPage(Diagram& diagram, const QList<std::shared_ptr<Railway>>& railways,
        const QString& name);
    DiagramPage(Diagram& diagram, const QJsonObject& obj);
    auto& diagram(){return _diagram;}
    const auto& diagram()const{return _diagram;}
    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}
    auto& startYs(){return _startYs;}
    auto railwayAt(int i)const { return _railways.at(i); }
    const QString& name()const { return _name; }
    void setName(const QString& s) { _name = s; }

    auto& itemMap() { return _itemMap; }
    const auto& itemMap()const { return _itemMap; }

    QString railNameString()const;

    /**
     * 暂时实现为直接返回Diagram的数据 （不保存副本）
     */
    const Config& config()const;
    const MarginConfig& margins()const;

    inline int railwayCount()const { return _railways.size(); }

    /**
     * 所给线路的下标。通过地址比较。线性算法。
     * 如果找不到，返回-1
     */
    int railwayIndex(const Railway& rail)const;

    double railwayStartY(const Railway& rail)const;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    void addItemMap(TrainLine* line, TrainItem* item) {
        _itemMap.insert(line, item);
    }

    void clearTrainItems(const Train& train);

    void clearAllItems();

    void highlightTrainItems(const Train& train);

    void unhighlightTrainItems(const Train& train);

    QList<QGraphicsRectItem*>& dirForbidItem(const Forbid* forbid, Direction dir);

    void addForbidItem(const Forbid* forbid, Direction dir, QGraphicsRectItem* item);

};


