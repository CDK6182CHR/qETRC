#pragma once
#include "xtl_graph.hpp"

#include <QString>
#include "graphstation.h"
#include "graphinterval.h"

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

private:
    void addRailway(const Railway* railway);
};

