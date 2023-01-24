#pragma once

#include "railnet/graph/xtl_graph.hpp"
#include "railnet/graph/graphstation.h"
#include "railnet/graph/graphinterval.h"

class Railway;
class TrainLine;
class Train;
class TrainStation;

/**
 * @brief The TrainLineNet class
 * Like RailNet, but use const TrainStation* as key type, for all the trainlines of a
 * given train. Used for TrainIntervalStat.
 */
class TrainLineNet : public xtl::di_graph<const TrainStation*, GraphStation, GraphInterval>
{
    using di_graph::sssp;
    using di_graph::dump_path;
public:
    TrainLineNet()=default;

    /**
     * @brief fromBoundTrain
     * Load net from train (that has already been bound to railways)
     * The data is not cleared.
     */
    void fromBoundTrain(const Train& train);

    struct TrainLinePathResult{
        bool exists;
        double mile;
        QString path_s;
    };

    TrainLinePathResult pathBetween(const TrainStation* from, const TrainStation* to);

private:
    void addTrainLine(const Railway& rail, const TrainLine& line);
};
