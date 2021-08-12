#pragma once

#include <memory>
#include "railintervaldata.hpp"

#include <QJsonObject>
#include <QString>
#include <QVariant>

class Ruler;
class RailInterval;

class RulerNode:
        public RailIntervalNode<RulerNode,Ruler>
{
public:
    int interval,start,stop;
    RulerNode(Ruler& ruler,RailInterval& railint,
              int interval=0,int start=0,int stop=0);

    //需要时实现
    RulerNode(const RulerNode&)=delete;
    RulerNode(RulerNode&&)=default;

    /**
     * 实现添加对方线路标尺（mergeIntervalData）时调用
     * 注意只复制了数据，没包含引用
     */
    RulerNode& operator=(const RulerNode& other);
    RulerNode& operator=(RulerNode&&)=delete;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /**
     * 区间通通时分的字符串表示。
     * 如果没数据，返回NA
     */
    QString intervalString()const;

    inline bool isNull()const { return interval == 0 && start == 0 && stop == 0; }

    void swap(RulerNode& other);
};


Q_DECLARE_METATYPE(std::shared_ptr<const RulerNode>);
