#include "diadiff.h"
#include <data/train/train.h>


StationDiff::DiffType StationDiff::stationCompareType(
        const TrainStation &st1, const TrainStation &st2)
{
    if (st1.name == st2.name){
        if (st1.arrive == st2.arrive && st1.depart == st2.depart)
            return Unchanged;
        else if (st1.arrive == st2.arrive)
            return DepartModified;
        else if (st1.depart == st2.depart)
            return ArriveModified;
        else
            return BothModified;
    }else{
        if (st1.arrive==st2.arrive && st1.depart==st2.depart)
            return NameChanged;
    }
    // 其他情况，无法判定，返回这个占位
    return NewAdded;
}

StationDiff::StationDiff(DiffType type, station_t itr1, station_t itr2):
    type(type), station1(itr1),station2(itr2)
{

}

TrainDifference::TrainDifference(std::shared_ptr<const Train> train1,
                                 std::shared_ptr<const Train> train2):
    type(Changed), train1(train1),train2(train2),similarity(-1),difference(-1)
{
    compute();
}

TrainDifference::TrainDifference(DiffType type, std::shared_ptr<const Train> train):
    type(type),similarity(0),difference(0)
{
    if (type==NewAdded)
        train2=train;
    else if (type==Deleted)
        train1=train;
}

const TrainName& TrainDifference::trainName() const
{
    return train1 ? train1->trainName() : train2->trainName();
}

int TrainDifference::calSimilarity(const TrainStation &st1, const TrainStation &st2)
{
    if (st1.name == st2.name)
        return 1;
    else return 0;
}

void TrainDifference::compute()
{
    if (train1->empty() || train2->empty()){
        // 特殊情况？
        return;
    }
    xtl::matrix<int> table(train1->stationCount(),train2->stationCount()),
            next_i(train1->stationCount(),train2->stationCount()),
            next_j(train1->stationCount(),train2->stationCount());
    table.fill(-1);
    next_i.fill(-1);
    next_j.fill(-1);

    similarity=solve(0,0,train1->timetable().begin(),train2->timetable().begin(),
                     table,next_i,next_j);
    difference=genResult(0,0,train1->timetable().begin(),train2->timetable().begin(),
                         next_i,next_j);
    if (difference==0)
        type=Unchanged;
}

int TrainDifference::solve(int s1, int s2, std::list<TrainStation>::const_iterator itr1,
                           std::list<TrainStation>::const_iterator itr2, xtl::matrix<int> &table,
                           xtl::matrix<int> &next_i, xtl::matrix<int> &next_j)
{
    if (s1>=train1->stationCount() && s2>=train2->stationCount()){
        // 双越界  长度为0的话直接这里走掉了
        return 0;
    }else if (s1>=train1->stationCount()){
        // s1越界，则2和空白匹配一位
        next_i.get(s1-1,s2)=s1;
        next_j.get(s1-1,s2)=s2+1;
        return 0+solve(s1,s2+1,itr1,std::next(itr2),table,next_i,next_j);
    }else if (s2>=train2->stationCount()){
        next_i.get(s1,s2-1)=s1+1;
        next_j.get(s1,s2-1)=s2;
        return 0+solve(s1+1,s2,std::next(itr1),itr2,table,next_i,next_j);
    }

    if (table.get(s1,s2) != -1){
        return table.get(s1,s2);
    }
    int rec1=calSimilarity(*itr1,*itr2)+
            solve(s1+1,s2+1,std::next(itr1),std::next(itr2),table,next_i,next_j);
    int rec2=0+solve(s1,s2+1,itr1,std::next(itr2),table,next_i,next_j);
    int rec3=0+solve(s1+1,s2,std::next(itr1),itr2,table,next_i,next_j);
    int sol=std::max({rec1,rec2,rec3});
    table.get(s1,s2)=sol;
    if(sol==rec1){
        next_i.get(s1,s2)=s1+1;
        next_j.get(s1,s2)=s2+1;
    }else if(sol==rec2){
        next_i.get(s1,s2)=s1;
        next_j.get(s1,s2)=s2+1;
    }else{
        next_i.get(s1,s2)=s1+1;
        next_j.get(s1,s2)=s2;
    }
    return sol;
}

int TrainDifference::genResult(int s1, int s2, std::list<TrainStation>::const_iterator itr1,
                               std::list<TrainStation>::const_iterator itr2,
                               xtl::matrix<int>& next_i, xtl::matrix<int>& next_j)
{
    if (s1==-1 || s2==-1){
        return 0;
    }
    if (s1>=train1->stationCount() && s2>=train2->stationCount()){
        return 0;
    }else if (s1>=train1->stationCount()){
        addStation(std::nullopt,itr2);
        return 1+genResult(s1,s2+1,itr1,std::next(itr2),next_i,next_j);
    }else if(s2>=train2->stationCount()){
        addStation(itr1,std::nullopt);
        return 1+genResult(s1+1,s2,std::next(itr1),itr2,next_i,next_j);
    }
    int nxi=next_i.get(s1,s2);
    int nxj=next_j.get(s1,s2);
    int diff;
    if (nxi!=s1 && nxj!=s2){
        diff=addStation(itr1,itr2);
    }else if(nxi!=s1){
        diff=addStation(itr1,std::nullopt);
    }else{
        diff=addStation(std::nullopt,itr2);
    }
    return diff+genResult(nxi,nxj,nextItr(s1,itr1,nxi),
                          nextItr(s2,itr2,nxj),next_i,next_j);
}



std::list<TrainStation>::const_iterator TrainDifference::nextItr(int s,
                        std::list<TrainStation>::const_iterator itr, int nx)
{
    std::advance(itr,nx-s);
    return itr;
}

int TrainDifference::addStation(std::optional<std::list<TrainStation>::const_iterator> si,
                                std::optional<std::list<TrainStation>::const_iterator> sj)
{
    int diff=1;
    StationDiff::DiffType tp;
    if(!si.has_value()){
        tp=StationDiff::NewAdded;
    }else if(!sj.has_value()){
        tp=StationDiff::Deleted;
    }else{
        tp=StationDiff::stationCompareType(*si.value(),*sj.value());
        if (tp==StationDiff::Unchanged){
            diff=0;
        }
        else if (tp==StationDiff::NewAdded){
            // 两个无关，要分别新增
            stations.emplace_back(StationDiff::Deleted,si,std::nullopt);
            diff=2;
            si=std::nullopt;
        }
    }
    stations.emplace_back(tp,si,sj);
    return diff;
}



