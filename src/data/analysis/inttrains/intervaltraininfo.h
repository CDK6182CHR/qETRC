#pragma once

#include <memory>
#include <vector>
#include <map>

class RailStation;
class Train;

class TrainStation;
class TrainLine;

/**
 * @brief The IntervalTrainInfo struct
 * 区间列车信息的结点。
 * @see pyETRC.data.Graph.getIntervalTrain() 返回类型
 * @note 这里暂定保存TrainStation而不是AdapterStation.
 * 因为AdapterStation只要发生重新绑定就会失效，而TrainStation相对稳定一点。
 * 但如果本类存续期间发生列车信息更新，仍可能造成悬空指针。
 * 暂时应该用不到RailStation，所以暂时不给
 */
struct IntervalTrainInfo
{
    std::shared_ptr<const Train> train;
    const TrainStation* from;
    const TrainStation* to;
    bool isStarting, isTerminal;
public:
    IntervalTrainInfo(std::shared_ptr<const Train> train,
                      const TrainStation* from,
                      const TrainStation* to,
                      bool isStarting,
                      bool isTerminal):
        train(train), from(from), to(to), isStarting(isStarting),
        isTerminal(isTerminal)
    {}
};

using IntervalTrainList=std::vector<IntervalTrainInfo>;

/**
 * @brief The IntervalCountInfo class
 * 此类用于进行区间对数统计。
 * 包含给定区间（保证在同一线路上）两站之间的列车表，以及各种统计。
 */
class IntervalCountInfo
{
    IntervalTrainList _list;
    int _startCount=0,_endCount=0,_startEndCount=0;
public:
    IntervalCountInfo()=default;

    /**
     * 添加数据，同时统计
     */
    void add(IntervalTrainInfo&& data);

    void clear();

    const auto& list()const{return _list;}

    int startCount()const{return _startCount;}
    int endCount()const{return _endCount;}
    int startEndCount()const{return _startEndCount;}

};

using RailIntervalCount=std::map<std::shared_ptr<const RailStation>,
        IntervalCountInfo>;
