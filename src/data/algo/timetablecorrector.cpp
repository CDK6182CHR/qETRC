#include "timetablecorrector.h"
#include <vector>
#include <data/train/train.h>
#include <util/utilfunc.h>
#include <stdexcept>

bool TimetableCorrector::autoCorrect(std::shared_ptr<Train> train)
{
    int i;
    for(i=0;i<120;i++){
        bool flag = correctCycle(train);
        if(!flag)
            break;
    }
    return i>0;
}

bool TimetableCorrector::autoCorrectSafe(std::shared_ptr<Train> train)
{
    int i;
    for (i = 0; i < 120; i++) {
        bool flag = false;
        try {
            flag = correctCycle(train);
        }
        catch (const std::exception& e) {
            qDebug() << "TimetableCorrector::autoCorrectSafe: exception: " << e.what() << Qt::endl;
            break;
        }
        
        if (!flag)
            break;
    }
    return i > 0;
}

bool TimetableCorrector::correctCycle(std::shared_ptr<Train> train)
{
    if (train->empty()){
        return false;
    }
    bool changed=false;
    bool unsolved=false;
    std::vector<TrainStation> timelist(train->timetable().begin(),
                                       train->timetable().end());
    int i=-1;
    while (i<static_cast<int>(train->timetable().size())-2){
        i+=1;
        const auto& dep = timelist.at(i).depart;
        const auto& arr = timelist.at(i+1).arrive;
        int sec=qeutil::secsTo(dep,arr);
        if (unsolved){
            /*
            # 2019.03.18新增逻辑。上一区间存在未解决的问题，很有可能是本站是始发站造成的。
            # 若本站是始发站，直接带着后面若干个站弄到第一个去。
            # 但也可能不是始发站，而是本线的第一个站。故取消始发站的限制。
            # 改为判定要移动的最后一个站和当前的第一个是否吻合。使用宽松判据（not self.error_interval）。
            # if stationEqual(self.sfz, timelist[i][0]):
            */
            int move_end=i;
            for (int t=i+1;t<(int)timelist.size();t++){
                const auto& last_dep = timelist[t-1].depart;
                const auto& this_arr = timelist[t].arrive;
                int sec1 = qeutil::secsTo(last_dep, this_arr);
                if (!neighbourInterval(sec1)){
                    break;
                }else{
                    move_end=t;
                }
            }
            const auto& origin_first = timelist.at(0).arrive;
            const auto& before_first = timelist.at(move_end).depart;
            int sec2=qeutil::secsTo(before_first, origin_first);
            if (train->starting().generalEqual(timelist[i].name) ||
                    neighbourInterval(sec2)){
                qDebug()<<"整体搬迁到首部 "<<train->trainName().full()<<" "<<
                          timelist.at(i).name.toSingleLiteral() <<
                          timelist.at(move_end).name.toSingleLiteral();
                // timelist = timelist[i:move_end+1]+timelist[:i]+timelist[move_end+1:]
                std::vector<TrainStation> newlist(timelist.begin()+i,
                                                  timelist.begin()+(move_end+1));
                // add [:i]
                {
                    auto itr=timelist.begin();
                    for(int k=0;k<i;k++,++itr){
                        newlist.emplace_back(std::move(*itr));
                    }
                }
                // add [move_end+1:]
                {
                    auto itr=timelist.begin()+(move_end+1);
                    for(;itr!=timelist.end();++itr){
                        newlist.emplace_back(std::move(*itr));
                    }
                }

                timelist = std::move(newlist);
                changed=true;
            }
            else{
                /*
                # 2019.11.28添加逻辑：另一种可能情况，由单个站的异常下沉有限个区间引起。
                # 尝试将当前站上浮，检查是否存在合理的位置。
                # 前面已经检查过放到首部不成立，所以只检查到第二个位置。
                */
                bool found=false;
                int t=i;
                while (t>1){
                    t-=1;
                    int int_try_before = qeutil::secsTo(timelist.at(t-1).depart,
                                                        timelist.at(t).arrive);
                    int int_try_after = qeutil::secsTo(timelist.at(t).depart,
                                                       timelist.at(t+1).arrive);
                    if (!errorInterval(int_try_before) && !errorInterval(int_try_after)){
                        qDebug()<<"结点上浮 "<<train->trainName().full()<<" "<<
                                  timelist.at(i).name.toSingleLiteral()<<" "<<
                                  timelist.at(t).name.toSingleLiteral();
                        found=true;
                        changed=true;
                        {
                            auto st=std::move(timelist.at(i));
                            timelist.erase(timelist.begin()+i);
                            timelist.insert(timelist.begin()+t, std::move(st));
                            break;
                        }
                    }
                }
                if(found){
                    unsolved=false;
                }else{
                    qDebug()<<"未能解决上一区间遗留问题"<<timelist.at(i).name.toSingleLiteral();
                }
            }
            unsolved=false;
            i=move_end;
            continue;
        }
        if (errorInterval(sec)){
            // 当前站与后一站的时间间隔超出合理阈值，将当前站往下沉
            int interval_before;
            if (i>0){
                interval_before = qeutil::secsTo(timelist.at(i-1).depart, timelist.at(i).arrive);
            }else{
                interval_before = -1;
            }
            int j=i;
            bool find=false;
            while (j<(int)timelist.size()-1){
                // # j是考虑放置现在的第i个元素的位置的前一个元素
                j++;
                int int_try_next;
                if (j<(int)timelist.size()-1){
                    int_try_next=qeutil::secsTo(dep, timelist.at(j+1).arrive);
                }else{
                    int_try_next=1;
                }
                if (!errorInterval(int_try_next)){
                    /*
                    # 下一区间时间合法，再考虑前一区间时间是否小于原来的前一区间时间。
                    # 这部分待定，可能考虑删去。试一下再说。
                    */
                    int int_try_before=qeutil::secsTo(timelist.at(j).depart,
                                                      timelist.at(i).arrive);
                    if (int_try_before<interval_before || interval_before == -1){
                        qDebug()<<"detect error change "<<
                                  train->trainName().full()<<" "<<
                                  timelist.at(i).name.toSingleLiteral()<<" "<<
                                  timelist.at(j).name.toSingleLiteral();
                        find=true;
                        break;
                    }else{
                        qDebug()<<"上一区间数据不合法 "<<timelist.at(i).name.toSingleLiteral()<<
                                  " "<<timelist.at(j).name.toSingleLiteral();
                    }
                }
            }
            if (find){
                changed=true;
                TrainStation st=std::move(timelist.at(i));
                timelist.erase(timelist.begin()+i);
                timelist.insert(timelist.begin()+j,std::move(st));
            }else{
                qDebug()<<"问题遗留到下一区间处理 "<<timelist.at(i).name.toSingleLiteral();
                unsolved=true;
            }
        }else{
            unsolved=false;
        }
    }
    // now restore timetable
    if (changed){
        std::list<TrainStation> newlist(timelist.begin(),timelist.end());
        train->timetable()=std::move(newlist);
    }
    return changed;
}

bool TimetableCorrector::neighbourInterval(int secs)
{
    constexpr const int ALLOW=30*60;
    return secs<=ALLOW;
}

bool TimetableCorrector::errorInterval(int secs)
{
    constexpr const int ALLOW=12*3660;
    return secs>ALLOW;
}
