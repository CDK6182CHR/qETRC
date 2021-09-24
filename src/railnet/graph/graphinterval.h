#include <QString>
#include <QTime>
#include <QVector>
#pragma once

#include "data/common/direction.h"

class RailInterval;
class RulerNode;
class ForbidNode;


/**
 * @brief The GraphForbidNode struct
 * 天窗  暂定直接按照下标来合并
 */
struct GraphForbidNode{
    QTime beginTime,endTime;
    GraphForbidNode(const ForbidNode& node);
};

struct GraphRulerNode{
    QString name;
    int interval,start,stop;
    GraphRulerNode(const RulerNode& node);
};

/**
 * @brief The GraphInterval class
 * 地位类似RailInterval；但数据都必须和那边独立，采用深拷贝。
 * 需包含必要的线路信息。
 */
class GraphInterval
{
public:
    QString railName;
    Direction dir;
    QVector<GraphForbidNode> forbidNodes;
    QVector<GraphRulerNode> rulerNodes;
public:
    GraphInterval()=default;
    GraphInterval(const QString& name, const RailInterval& interval);
};

