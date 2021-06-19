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

//#include "rulernode.h"

#include "railintervaldata.hpp"

class Railway;

class Ruler:
        public RailIntervalData<RulerNode, Ruler>
{
    QString _name;

    friend class Railway;

    /*
     * 构造函数原则上仅允许Railway::addEmptyRuler调用
     */
    Ruler(Railway& railway, const QString& name, bool different, int index);
public:

    inline const QString& name()const { return _name; }
    inline void setName(const QString& name) { _name = name; }

    QJsonObject toJson()const;

    void show()const;
};

#endif // RULER_H
