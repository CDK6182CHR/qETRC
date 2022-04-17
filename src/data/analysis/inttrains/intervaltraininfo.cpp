#include "intervaltraininfo.h"


void IntervalCountInfo::add(IntervalTrainInfo &&data)
{

    if(data.isStarting&&data.isTerminal){
        _startEndCount++;
        _startCount++;
        _endCount++;
    }
    else if(data.isStarting){
        _startCount++;
    }
    else if(data.isTerminal){
        _endCount++;
    }

    _list.emplace_back(std::move(data));
}

void IntervalCountInfo::clear()
{
    _startCount=_endCount=_startEndCount=0;
    _list.clear();
}
