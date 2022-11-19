#pragma once
#include <memory>

class Train;

class ITrainFilter{
public:
    virtual bool check(std::shared_ptr<const Train> train)const = 0;
    virtual ~ITrainFilter()=default;
};
