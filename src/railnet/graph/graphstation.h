#pragma once

#include <QList>
#include <QString>
#include "data/common/stationname.h"

class RailStation;
/**
 * @brief The GraphStation class
 * 和RailStation地位类似  Graph中的车站结点
 * 采用类似struct的组织模式，field都公开。
 */
class GraphStation
{
public:
    StationName name;
    int level;
    bool passenger, freight;
    QList<QString> tracks;

    /**
     * @brief GraphStation
     * 构造函数  根据需要写
     */
    GraphStation()=default;
    GraphStation(const RailStation& station);
};

