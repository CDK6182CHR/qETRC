#pragma once
#include "xtl_graph.hpp"

#include <QString>
#include <memory>
//#include "qhashfunctions.h"
#include "graphstation.h"
#include "graphinterval.h"


class Railway;
class RailCategory;
class RailNet: public xtl::di_graph<StationName, GraphStation, GraphInterval>
{
    using di_graph::sssp;
    using di_graph::dump_path;
public:
    RailNet()=default;

    struct rail_ret_t{
        std::shared_ptr<Railway> railway;
        path_t downPath,upPath;
        static const rail_ret_t null;
    };

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
    rail_ret_t sliceBySinglePath(QVector<QString> points, bool withRuler,
                                         QString* report, int rulerCount)const;

    /**
     * 同时给出正反径路的关键节点算法。对正反径路进行合并，而不考虑正反径路起点是否一致。
     * 但如果有一边的径路为空，肯定过不了。
     */
    rail_ret_t sliceByDoublePath(const QVector<QString>& downPoints,
                                               const QVector<QString>& upPoints,
                                                bool withRuler,
                                               QString* report,
                                               int rulerCount)const;

    rail_ret_t sliceBySymmetryPath(const QVector<QString>& points,
                                                  bool withRuler,
                                                 QString* report, int rulerCount)const;

    /**
     * 将径路转换为字符串。
     */
    QString pathToString(const path_t& path)const;


private:
    void addRailway(const Railway* railway);

    /**
     * @brief 由points所给关键点表返回单向的Railway对象。所有站都只有下行通过。
     * 如果查找失败，返回空；并在report中报告错误原因。
     * 返回数据包含所有标尺、天窗数据。
     */
    std::pair<std::shared_ptr<Railway>,path_t>
        singleRailFromPath(const QVector<QString>& points, bool withRuler,
                           QString* report)const;

    QString intervalPathToString(std::shared_ptr<const vertex> pre,
        const std::shared_ptr<const vertex> post, const QString& railName,
        double mile)const;

};

Q_DECLARE_METATYPE(std::shared_ptr<RailNet::vertex>)
Q_DECLARE_METATYPE(std::shared_ptr<RailNet::edge>)

