#include "train.h"



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
        _timetable.emplace_back(*p);
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
