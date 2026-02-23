#pragma once

#include <list>
#include <memory>

class TrainStation;
class RailStation;

/**
 * @brief The AdapterNode struct
 * 一个铺画的车站
 */
struct AdapterStation {
    std::list<TrainStation>::iterator trainStation;
    std::weak_ptr<RailStation> railStation;
    AdapterStation(std::list<TrainStation>::iterator trainStation_,
        std::weak_ptr<RailStation> railStation_) :
        trainStation(trainStation_), railStation(railStation_) {
    }
    bool operator==(const AdapterStation& other)const;
    bool operator<(double y)const;
    double yCoeff()const;
};

bool operator<(double y, const AdapterStation& adp);
