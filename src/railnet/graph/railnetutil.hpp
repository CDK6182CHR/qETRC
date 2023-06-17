#pragma once

#include <QString>
#include "xtl_graph.hpp"
#include "graphinterval.h"
#include "graphstation.h"

namespace railnetutil{

//template <typename _Key>
//QString RailNet::intervalPathToString(std::shared_ptr<const vertex> pre,
//    const std::shared_ptr<const vertex> post, const QString& railName, double mile) const
//{
//    return QString("%1-%2  %3  %4 km").arg(pre->data.name.toSingleLiteral(),
//        post->data.name.toSingleLiteral(), railName, QString::number(mile, 'f', 3));
//}

template <typename _Key>
QString pathToString(
    const typename xtl::di_graph<_Key,GraphStation,GraphInterval>::path_t& path)
{
    using graph_t=xtl::di_graph<_Key,GraphStation,GraphInterval>;
    using vertex_t=typename graph_t::vertex;

    auto intervalPathToString=[](std::shared_ptr<const vertex_t> pre,
            std::shared_ptr<const vertex_t> post, const QString& railName,
            double mile)->QString{
        return QString("%1-%2  %3  %4 km").arg(pre->data.name.toSingleLiteral(),
            post->data.name.toSingleLiteral(), railName, QString::number(mile, 'f', 3));
    };

    if (path.empty())return {};
    QString res;
    double mile = 0;
    double lastMile = 0;
    std::shared_ptr<const typename graph_t::vertex> lastVert{};
    QString lastRailName;

    // 循环中：如果线名不同，则总结上一步，开辟新的。
    for (const auto& ed : path) {
        if (ed->data.railName != lastRailName) {
            // 总结上一段径路
            if (!lastRailName.isEmpty()) {
                // 非第一次
                res.append(intervalPathToString(lastVert, ed->from.lock(),
                    lastRailName, mile - lastMile));
                res.append('\n');
            }
            lastRailName = ed->data.railName;
            lastMile = mile;
            lastVert = ed->from.lock();
        }
        mile += ed->data.mile;
    }

    // 2023.06.17 FIX: lastVert may be empty
    if (lastVert && lastVert != path.back()->to.lock()) {
        // 最后一段的处理
        res.append(intervalPathToString(lastVert, path.back()->to.lock(),
            lastRailName, mile - lastMile));
    }
    return res;
}
}
