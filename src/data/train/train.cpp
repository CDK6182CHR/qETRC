#include "train.h"
#include <QDebug>

#include "data/rail/rail.h"


Train::Train(const TrainName &trainName,
             const StationName &starting,
             const StationName &terminal,
             TrainPassenger passenger):
    _trainName(trainName),_starting(starting),_terminal(terminal),
    _passenger(passenger),_show(true)
{

}

Train::Train(const QJsonObject &obj)
{
    fromJson(obj);
}

void Train::fromJson(const QJsonObject &obj)
{
    const QJsonArray& archeci=obj.value("checi").toArray();
    _trainName.fromJson(archeci);

    _type=obj.value("type").toString();
    _starting=StationName::fromSingleLiteral( obj.value("sfz").toString());
    _terminal=StationName::fromSingleLiteral(obj.value("zdz").toString());
    _show=obj.value("shown").toBool();
    _passenger=static_cast<TrainPassenger>(obj.value("passenger").toInt());
    const QJsonArray& artable=obj.value("timetable").toArray();
    //todo: UI, circuit, item
    for (auto p=artable.cbegin();p!=artable.cend();++p){
        _timetable.emplace_back(p->toObject());
    }
}

QJsonObject Train::toJson() const
{
    QJsonArray ar;
    for(const auto& p:_timetable){
        ar.append(p.toJson());
    }
    return QJsonObject{
        {"checi",_trainName.toJson()},
        {"type",_type},
        {"sfz",_starting.toSingleLiteral()},
        {"zdz",_terminal.toSingleLiteral()},
        {"shown",_show},
        {"passenger",static_cast<int>(_passenger)},
        {"timetable",ar}
    };
}

void Train::appendStation(const StationName &name,
                          const QTime &arrive, const QTime &depart,
                          bool business, const QString &track, const QString &note)
{
    _timetable.emplace_back(name, arrive, depart, business, track, note);
}

typename Train::StationPtr
    Train::findFirstStation(const StationName& name)
{
    auto p = _timetable.begin();
    for (; p != _timetable.end(); ++p) {
        if (p->name == name)
            break;
    }
    return p;
}

QList<typename Train::StationPtr> Train::findAllStations(const StationName& name)
{
    QList<StationPtr> res;
    for (auto p = _timetable.begin(); p != _timetable.end(); ++p) {
        if (p->name == name) {
            res.append(p);
        }
    }
    return res;
}

Train::StationPtr Train::findFirstGeneralStation(const StationName &name)
{
    auto p=findFirstStation(name);
    if(p!=nullStation())
        return p;
    for(p=_timetable.begin();p!=_timetable.end();++p){
        if (p->name.equalOrBelongsTo(name))
            break;
    }
    return p;
}

QList<Train::StationPtr> Train::findAllGeneralStations(const StationName &name)
{
    QList<StationPtr> res;
    for (auto p = _timetable.begin(); p != _timetable.end(); ++p) {
        if (p->name.equalOrBelongsTo(name)) {
            res.append(p);
        }
    }
    return res;
}

void Train::bindToRailway(std::shared_ptr<Railway> railway)
{
    unbindToRailway();
    _boundRail=railway;
    auto last=nullStation();   //上一个成功绑定的车站
    std::shared_ptr<RailStation> raillast;
    auto p=_timetable.begin();
    Direction locDir=Direction::Undefined;
    int cnt=0;   //绑定计数器
    for(;p!=nullStation();++p){
        //在Railway中找车站是常数复杂度
        auto railst=railway->stationByGeneralName(p->name);
        if(railst){
            //非空指针表示搜索成功。现在考虑是否绑定
            //目前唯一阻止绑定的事由是经由方向不对
            //原则上，这是不大可能发生的事情；因此只有显示知道行别不对时，才拒绝绑定
            p->bindToRailStation(railst);
            if (locDir==Direction::Undefined ||
                    railst->isDirectionVia(locDir)){
                //成功绑定到车站
                cnt++;
                p->bindToRailStation(railst);
                //现在：计算当前区间上下行情况
                if (raillast){
                    locDir=railway->gapDirection(raillast,railst);
                }
                if (cnt==2){
                    //第二个站时，还是检查一下第一个站的绑定是否合适
                    if(!raillast->isDirectionVia(locDir)){
                        //发现方向不对，撤销绑定
                        //相当于当前是第一个
                        cnt--;
                        last->unbindToRailStation();
                    }
                }
                raillast=railst;
                last=p;
            }
        }
    }
}

void Train::unbindToRailway()
{
    _boundRail.reset();
    for(auto& p:_timetable){
        p.unbindToRailStation();
    }
}

Train Train::translation(TrainName name, int sec)
{
    Train train(*this);   //copy construct
    train.setTrainName(name);
    for(auto& p:train._timetable){
        p.arrive=p.arrive.addSecs(sec);
        p.depart=p.depart.addSecs(sec);
    }
    return train;
}

void Train::show() const
{
    _trainName.show();
    for(const auto& p:_timetable){
        qDebug()<<p;
    }
}
