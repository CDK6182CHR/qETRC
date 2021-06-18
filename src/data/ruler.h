/*
 * 与标尺有关的数据结构，以及标尺封装（代理）类
 * 与pyETRC不同，Ruler类并不包含真正的数据，更像一个View
 * 尽量去实现旧有的API
 * 新增index，要求必须等于在QList中的坐标，采用这个去定位
 */
#ifndef RULER_H
#define RULER_H

#include <QString>
#include <QJsonObject>
#include <QList>

#include "rulernode.h"
#include "stationname.h"

class Railway;

class Ruler
{
    Railway& _railway;
    QString _name;
    bool _different;
    int _index;

    friend class Railway;

    /*
     * 构造函数原则上仅允许Railway::addEmptyRuler调用
     */
    Ruler(Railway& railway, const QString& name, bool different, int index);
public:
    std::shared_ptr<RulerNode> firstDownNode();
    std::shared_ptr<RulerNode> firstUpNode();
    std::shared_ptr<const RulerNode> firstDownNode()const;
    std::shared_ptr<const RulerNode> firstUpNode()const;

    QJsonObject toJson()const;

    inline int index()const{return _index;}
    inline bool different()const{return _different;}

    /*
     * 在仅考虑近邻的情况下，获取区间信息
     */
    std::shared_ptr<RulerNode> getNode(const StationName& from,const StationName& to);
    std::shared_ptr<const RulerNode>
        getNode(const StationName& from,const StationName& to)const;

    void show()const;
};

#endif // RULER_H
