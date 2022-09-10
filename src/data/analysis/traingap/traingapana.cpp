#include "traingapana.h"

#include <data/diagram/diagram.h>

TrainGapAna::TrainGapAna(Diagram &diagram, const TrainFilterCore &filter):
    diagram(diagram), filter(filter)
{

}

std::map<TrainGapTypePair, int> TrainGapAna::globalMinimal(
        std::shared_ptr<Railway> rail) const
{
    std::map<TrainGapTypePair,int> res{};
    auto events=diagram.stationEventsForRail(rail);
    for(auto _p=events.begin();_p!=events.end();++_p){
        const RailStationEventList& lst=_p->second;
        // todo: the single line
        auto gaps=diagram.getTrainGaps(lst, filter,_p->first, _singleLine);
        TrainGapStatistics stat=diagram.countTrainGaps(gaps, _cutSecs);

        for (auto q = stat.begin(); q != stat.end(); ++q) {
            const TrainGapTypePair& tp = q->first;
            std::shared_ptr<TrainGap> gap = q->second.begin().operator*();

            // 统计全局最小
            if (auto curmin = res.find(tp); curmin != res.end()) {
                curmin->second = std::min(curmin->second, gap->secs());
            }
            else {
                res.emplace(tp, gap->secs());
            }
        }
    }
    return res;
}
