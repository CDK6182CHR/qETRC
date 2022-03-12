#include "crset.h"
#include "crgroups.h"

namespace gapset {
namespace cr {

CRSet::CRSet()
{

}

void CRSet::buildSet()
{
    clear();
    emplace_back(std::make_unique<GroupTrack>());
    emplace_back(std::make_unique<GroupMeet>());
    emplace_back(std::make_unique<GroupArrive>());

    auto alltypes=TrainGap::allPossibleGaps(_singleLine);
    for(auto itr=alltypes.begin();itr!=alltypes.end();){
        bool found=false;
        for(const auto& t:*this){
            if(t->matches(*itr)){
                t->emplace_back(*itr);
                itr=alltypes.erase(itr);
                found=true;
                break;   // 结束type轮询
            }
        }
        if(!found){
            ++itr;
        }
    }

    // 清除空的
    for(auto itr=begin();itr!=end();){
        if((*itr)->empty()){
            itr=erase(itr);
        }else{
            ++itr;
        }
    }

    _remainTypes=alltypes;
}

} // namespace cr
} // namespace gapset
