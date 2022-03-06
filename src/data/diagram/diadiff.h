#pragma once

#include <memory>
#include <list>
#include <vector>
#include <optional>
#include "xtl_matrix.hpp"

class TrainStation;

struct StationDiff{
    enum DiffType{
        Unchanged = 0b0,  // 站名，时刻完全一致
        ArriveModified = 0b01,  // 名字没变，但时刻变了
        DepartModified = 0b10,
        BothModified = 0b11,
        NameChanged = 0b100,  // 站名改了，但时刻没变，一般用不到。
        NewAdded = 0b1000,  // 原来的没有，新增的
        Deleted = 0b10000,  //本车次有，对方删了
    };
    DiffType type;
    using station_t=std::optional<std::list<TrainStation>::const_iterator>;
    station_t station1, station2;

    static DiffType stationCompareType(const TrainStation& st1, const TrainStation& st2);

    StationDiff(DiffType type, station_t itr1, station_t itr2);
};

class Train;
class TrainName;
/**
 * @brief The TrainDifference class
 * pyETRC.Train.globalDiff()
 * 实现动态规划的列车时刻表对比算法，以及保存对比结果。
 */
class TrainDifference
{
public:
    enum DiffType{
        Unchanged,
        Changed,
        NewAdded,
        Deleted
    };
    DiffType type;
    std::shared_ptr<const Train> train1, train2;
    std::vector<StationDiff> stations;
    int similarity,difference;

    /**
     * 双车次构造：用于列车修改或不变
     */
    TrainDifference(std::shared_ptr<const Train> train1,
                    std::shared_ptr<const Train> train2);

    /**
     * 单车次构造：用于增删，
     * train如何解释取决于type
     */
    TrainDifference(DiffType type, std::shared_ptr<const Train> train);

    const TrainName& trainName()const;

private:
    int calSimilarity(const TrainStation& st1, const TrainStation& st2);

    /**
     * 计算的总入口函数。将结果直接保存在类内。原则上，只能调用一次。
     */
    void compute();

    /**
     * @brief solve  递归求解。pyETRC.Train.globalDiff.solve()
     * @param s1 train1车次起始下标子问题
     * @param s2 train2车次起始下标子问题
     * @param table DP字典
     * @param next_i DP路径
     * @param next_j
     * @return 相似度
     */
    int solve(int s1, int s2, std::list<TrainStation>::const_iterator itr1,
              std::list<TrainStation>::const_iterator itr2,
              xtl::matrix<int>& table,
              xtl::matrix<int>& next_i, xtl::matrix<int>& next_j);

    /**
     * @brief genResult  pyETRC.Train.globalDiff.generate_result()
     * itr (1,2) 是与s (1,2)对应的迭代器，用来回避下标运算。
     * 如果s1, s2之间有一个是-1，那么立即返回，此时不用考虑迭代器。
     * 迭代器总是单向移动的。
     * @return
     */
    int genResult(int s1,int s2,
                  std::list<TrainStation>::const_iterator itr1,
                  std::list<TrainStation>::const_iterator itr2,
                  xtl::matrix<int>& next_i,
                  xtl::matrix<int>& next_j);

    /**
     * @brief nextItr  用迭代器作差的方法，计算与nx对应的下一个迭代器。
     * @param s 初始下标 （非-1）
     * @param itr 初始迭代器
     * @param nx 目标下标
     * @return 新的迭代器，与nx对应
     */
    std::list<TrainStation>::const_iterator
        nextItr(int s, std::list<TrainStation>::const_iterator itr, int nx);

    int addStation(std::optional<std::list<TrainStation>::const_iterator> si,
                   std::optional<std::list<TrainStation>::const_iterator> sj);
};

using diagram_diff_t=std::vector<std::shared_ptr<TrainDifference>>;

