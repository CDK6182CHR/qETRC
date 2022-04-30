#include "gapsetabstract.h"



void gapset::GapSetAbstract::setSingleLineAndBuild(bool on)
{
    if (on != _singleLine){
        _singleLine=on;
        buildSet();
    }
}

std::map<const gapset::GapGroupAbstract *, int>
    gapset::GapSetAbstract::minimalGapByGroup(
        const std::map<TrainGapTypePair, int> &mingap, int minSecs, int maxSecs) const
{
    std::map<const GapGroupAbstract*,int> res{};
    for(auto p=mingap.begin();p!=mingap.end();++p){
        int cur_sec=std::max(minSecs, p->second);
        for(const auto& g: *this){
            if (g->matches(p->first)){
                // 设置g的最小值之上界为p->second所示数值
                if (auto itr=res.find(g.get());itr!=res.end()){
                    itr->second=std::min(itr->second, cur_sec);

                }else{
                    res.emplace(g.get(), p->second);
                }
                break;
            }
        }
    }

    // 最后：检查是否所有类型都有值了，否则给兜底值
    for(const auto& g:*this){
        if (auto itr=res.find(g.get());itr!=res.end()){
            if (itr->second > maxSecs)
                itr->second=maxSecs;
        }
        else{
            res.emplace(g.get(),minSecs);
        }
    }
    return res;
}

void gapset::GapSetAbstract::setConstraintFromMinimal(
        const std::map<TrainGapTypePair, int> &mingap,
        int minSecs, int maxSecs)
{
    auto data=minimalGapByGroup(mingap,minSecs,maxSecs);
    for(const auto& gp:*this){
        gp->setLimit(data.at(gp.get()));
    }
}
