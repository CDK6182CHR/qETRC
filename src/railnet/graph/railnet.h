#pragma once
#include "xtl_graph.hpp"

#include <QString>
//#include "qhashfunctions.h"
#include "graphstation.h"
#include "graphinterval.h"

namespace std {
    template <>
    struct hash<StationName> {
    public:
        size_t operator()(const StationName& key)const {
            return qHash(key);
        }
    };
}

class Railway;
class RailCategory;
class RailNet: private xtl::di_graph<StationName, GraphStation, GraphInterval>
{
public:
    using di_graph::clear;
    using di_graph::size;
    RailNet()=default;

    /**
     * 从RailCategory中的线路，提取数据到有向图中。
     * 不持有cat的所有权，因此采用裸指针
     * 注意：不会清空既有数据。
     */
    void fromRailCategory(const RailCategory* cat);

    /**
     * 采用关键点最短路径算法，获取径路切片。
     * 正反分别执行一次最短路算法，然后进行合并。
     * 如果出错，直接返回空。
     */
    std::shared_ptr<Railway> sliceByPath(QVector<QString> points,
                                         QString* report);


private:
    void addRailway(const Railway* railway);

    /**
     * @brief 由points所给关键点表返回单向的Railway对象。所有站都只有下行通过。
     * 如果查找失败，返回空；并在report中报告错误原因。
     * 返回数据包含所有标尺、天窗数据。
     */
    std::shared_ptr<Railway>
        singleRailFromPath(const QVector<QString>& points, QString* report);

};

