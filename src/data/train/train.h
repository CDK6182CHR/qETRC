#pragma once

#include <cstdint>
#include <QList>
#include <QJsonObject>
#include "trainname.h"
#include "trainstation.h"

enum class TrainPassenger:
        std::int8_t{
    False=0,
    Auto=1,
    True=2
};

class TrainUIConfig{

};

/*
 * pyETRC.train.Train
 */
class Train
{
    TrainName _trainName;
    StationName _starting,_terminal;
    QString _type;
    TrainUIConfig _ui;
    TrainPassenger _passenger;
    bool _show;

    QList<std::shared_ptr<TrainStation>> _timetable;

    std::weak_ptr<Railway> _boundRail;

    //QtWidgets.QGraphicsViewPathItem pathItem;
    //QtWidgets.QGraphicsViewItem labelItem;

    //todo: 交路，随机访问Map表等

public:
    Train(const TrainName& trainName,
          const StationName& starting=StationName::nullName,
          const StationName& terminal=StationName::nullName,
          TrainPassenger passenger=TrainPassenger::Auto);
    Train(const QJsonObject& obj);

    /*
     * 需要时再好好考虑实现方法
     */
    Train(const Train&)=delete;
    Train(Train&&)=default;
    Train& operator=(const Train&)=delete;
    Train& operator=(Train&&)=delete;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    inline const TrainName& trainName()const{return _trainName;}
    inline const StationName& starting()const{return _starting;}
    inline const StationName& terminal()const{return _terminal;}
    inline const QString& type()const{return _type;}
    inline TrainPassenger passenger()const{return _passenger;}
    inline bool isShow()const{return _show;}

    inline void setTrainName(const TrainName& n){_trainName=n;}
    inline void setStarting(const StationName& s){_starting=s;}
    inline void setTerminal(const StationName& s){_terminal=s;}
    inline void setType(const QString& t){_type=t;}
    inline void setPassenger(TrainPassenger t){_passenger=t;}
    inline void setIsShow(bool s){_show=s;}

    void appendStation(const StationName& name,
                       const QTime& arrive,
                       const QTime& depart,
                       bool business=true,
                       const QString& track="",
                       const QString& note="");

    /*
     * Train.stationDict()
     * 查找算法
     */
    std::shared_ptr<TrainStation> stationByName(const StationName& name);

    /*
     * 判定局部上下行性质
     * 这个实现比较困难，现在不一定能做
     */
    Direction stationDirection(const StationName& station);

    Direction stationDirection(int index);

};


