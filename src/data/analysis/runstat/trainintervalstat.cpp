#include "trainintervalstat.h"
#include "data/rail/railcategory.h"
#include "data/train/train.h"
#include "util/utilfunc.h"

#include <QObject>

TrainIntervalStat::TrainIntervalStat(RailCategory &railcat,
                                     const std::shared_ptr<const Train>& train):
    railcat(railcat),train(train)
{

}

void TrainIntervalStat::setRange(int start, int end)
{
    _startIndex=start;
    _endIndex=end;
}

TrainIntervalStatResult TrainIntervalStat::compute()
{
    updateDigraph();
    TrainIntervalStatResult res;
    computeTrainPart(res);  // this is responsible for initializing _startIter
    computeRailPart(res);
    return res;
}

void TrainIntervalStat::updateDigraph()
{
    railnet.clear();
    railnet.fromRailCategory(&railcat);
}

void TrainIntervalStat::computeTrainPart(TrainIntervalStatResult &res)
{
    assert(_startIndex < _endIndex);
    res.settledStationsCount = _endIndex-_startIndex+1;
    _startIter=train->timetable().begin(); std::advance(_startIter, _startIndex);
    auto itr_last=_startIter;
    auto itr=std::next(_startIter);

    int stopCount=0;
    int runSecs=0,stopSecs=0;
    // loop invariant: each time the loop processes the interval ENDED at itr
    for (int i=0;i<res.settledStationsCount-1;i++){
        int intervalSecs=qeutil::secsTo(itr_last->depart, itr->arrive);
        runSecs+=intervalSecs;

        if (i<res.settledStationsCount-2){
            // not last
            if (itr->isStopped()){
                stopCount++;
                stopSecs+=itr->stopSec();
            }
        }


        itr_last=itr;
        ++itr;
    }
    // at the end of the loop, itr_last is just _endIter; itr may be end().
    _endIter=itr_last;

    res.settledStopCount=stopCount;
    res.runSecs=runSecs;
    res.stopSecs=stopSecs;
    res.totalSecs=runSecs+stopSecs;
}

void TrainIntervalStat::computeRailPart(TrainIntervalStatResult &res)
{
    auto v_start=railnet.stationByGeneralName(_startIter->name);
    auto v_end=railnet.stationByGeneralName(_endIter->name);
    if (!v_start || !v_end){
        res.railResults.isValid=false;
        res.railResults.path_s=QObject::tr("起始站或结束站未铺画");
        return;
    }

    QString report;
    auto t=railnet.shortestPath(v_start,v_end,&report);
    if (t.empty()){
        // fail
        res.railResults.isValid=false;
        res.railResults.path_s=report;
    }else{
        // success
        res.railResults.isValid=true;
        res.railResults.totalMiles = railnet.pathMile(t);
        res.railResults.path_s = railnet.pathToString(t);
        res.railResults.travelSpeed = res.railResults.totalMiles / res.totalSecs * 3600;
        res.railResults.techSpeed = res.railResults.totalMiles / res.runSecs * 3600;
    }
}
