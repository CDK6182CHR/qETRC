#pragma once

#include <memory>
#include <list>
#include "trainlinenet.h"
#include "trainintervalstatresult.h"

class RailCategory;
class Train;
class TrainStation;
/**
 * @brief The TrainIntervalStat class
 * 2023.01.24  For the train interval running stat,
 * like IntervalWidget in pyETRC, but much more complicated for multi-railway architechture.
 * For current version, the digraph model for railways is used,
 * and each time the analysis is performed on a single specified train.
 * The train should be bounded.
 */
class TrainIntervalStat
{
    TrainLineNet net;

    std::shared_ptr<const Train> train;
    // for computation temporary:
    int _startIndex,_endIndex;
    std::list<TrainStation>::const_iterator _startIter,_endIter;
    bool _include_ends = false;   // 2024.05.03: whether to include stop time of first and last station
public:
    TrainIntervalStat(const std::shared_ptr<const Train>& train=nullptr);
    void setTrain(std::shared_ptr<const Train> t) { this->train = t; }

    /**
     * 2024.05.03 note: the range includes both sides.
     * The stopping time in both start and end are considered.
     */
    void setRange(int start, int end);

    void setIncludeEnds(bool on) { _include_ends = on; }

    /**
     * @brief compute
     * The main API, compute all the required data and store in a newly created
     * object.
     * setTrain(), setRange() must be called before.
     */
    TrainIntervalStatResult compute();

private:
    void updateDigraph();

    /**
     * @brief computeTrainPart
     * Compute the part that does not relate to Railway, and is always valid.
     * Mainly boring things...
     */
    void computeTrainPart(TrainIntervalStatResult& res);

    /**
     * @brief computeRailPart
     * Compute the part that relate to Railway. Only valid if there is bound info
     * throughout the whole path.
     * If valid, store data in result. Otherwise, store std::nullopt.
     */
    void computeRailPart(TrainIntervalStatResult& res);
};

