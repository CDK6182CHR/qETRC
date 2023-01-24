#pragma once
#include "xtl_graph.hpp"

#include <QString>
#include <memory>
//#include "qhashfunctions.h"
#include "graphstation.h"
#include "graphinterval.h"

class PathOperationSeq;

class Railway;
class RailCategory;
class RailNet: public xtl::di_graph<StationName, GraphStation, GraphInterval>
{
    using di_graph::sssp;
    using di_graph::dump_path;

public:
    RailNet()=default;

    using path_t=path_t;

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
     * @brief stationByGeneralName
     * 2023.01.24  find a vertex that could be bound to givene station name.
     * That is, that exatly matches the name, or that contains only the station (not field).
     */
    std::shared_ptr<vertex> stationByGeneralName(const StationName& name);

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

    /**
     * 径路转换为单行的字符串。
     */
    QString pathToStringSimple(const path_t& path)const;

    /**
     * 按最短路算法生成部分路径。此混搭签名是为了用于经由选择器。
     */
    path_t shortestPath(const std::shared_ptr<const vertex>& from,
                        const std::shared_ptr<const vertex> & to,
                        QString* report) const;

    /**
     * 计算径路的总长度，简单累加每段的里程。
     */
    static double pathMile(const path_t& path);

    /**
     * 由所给边，按照当前线名向前追踪至线路终点，返回整个径路，包含起始。
     */
    path_t railPathFrom(const std::shared_ptr<const edge>& start)const;

    /**
     * 根据指定线名、方向，查找始点至末点的路径。
     * 如果找不到目标点，返回空。
     * 用于添加反向路径。
     */
    path_t railPathTo(const std::shared_ptr<const vertex>& start,
                      const std::shared_ptr<const vertex>& target,
                      const QString& railName, Direction dir)const;

    /**
     * 由经由选择操作生成线路。
     */
    std::shared_ptr<Railway> singleRailFromPathOperations(
            const PathOperationSeq& seq, bool withRuler)const;


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

    std::shared_ptr<Railway> singleRailFromPath(const path_t& path, bool withRuler)const;

    /**
     * 将子路径所示的内容添加到rail所指向的线路中。
     * 可用于从PathOperationSeq中生成线路。
     */
    void appendSubPathToRail(const std::shared_ptr<Railway>& rail,
                             const path_t& subpath,
                             bool withRuler,
                             QMap<QString,int>& rulerMap)const;



    QString intervalPathToString(std::shared_ptr<const vertex> pre,
        const std::shared_ptr<const vertex> post, const QString& railName,
        double mile)const;

};

Q_DECLARE_METATYPE(std::shared_ptr<RailNet::vertex>)
Q_DECLARE_METATYPE(std::shared_ptr<RailNet::edge>)

