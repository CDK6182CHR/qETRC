#pragma once

class TrainFilterCore;

/**
 * @brief The TrainFilterSelectorCore class
 * A simple wrapper of TrainFilterCore*, in case the actual object
 * would change in new TrainFilterSelector API.
 * This could only be modified by TrainFilterSelector.
 */
class TrainFilterSelectorCore
{
    friend class TrainFilterSelector;
    const  TrainFilterCore* _core;
public:
    TrainFilterSelectorCore()=default;
    auto* filter()const{return _core;}
};

