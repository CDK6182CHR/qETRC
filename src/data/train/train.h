#pragma once

#include <cstdint>
#include <QList>
#include <QJsonObject>
#include <list>
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

    /*
     * 暂定采用std::list来放时刻表。
     * std::list::iterator当作指针来使用；
     * 主要原因是考虑双向迭代的效率
     * 时刻表拥有内部所有结点的所有权
     * 利用std::list的迭代器以及引用不会失效的特性
     * 涉及到删除操作时，要特别小心
     */
    std::list<TrainStation> _timetable;

    using StationPtr=std::list<TrainStation>::iterator;
    using ConstStationPtr=std::list<TrainStation>::const_iterator;

    std::weak_ptr<Railway> _boundRail;

    //QtWidgets.QGraphicsViewPathItem pathItem;
    //QtWidgets.QGraphicsViewItem labelItem;

    //todo: 交路，随机访问Map表等

public:
    Train(const TrainName& trainName,
          const StationName& starting=StationName::nullName,
          const StationName& terminal=StationName::nullName,
          TrainPassenger passenger=TrainPassenger::Auto);
    explicit Train(const QJsonObject& obj);

    /*
     * 使用std::list时，默认的copy和move都是正确的
     */
    Train(const Train&)=default;
    Train(Train&&)=default;
    Train& operator=(const Train&)=default;
    Train& operator=(Train&&)=default;

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

    inline ConstStationPtr nullStation()const { return _timetable.cend(); }
    inline StationPtr nullStation(){return _timetable.end();}

    const std::list<TrainStation>& timetable()const{return _timetable;}
    std::list<TrainStation>& timetable(){return _timetable;}

    void appendStation(const StationName& name,
                       const QTime& arrive,
                       const QTime& depart,
                       bool business=true,
                       const QString& track="",
                       const QString& note="");

    /*
     * Train.stationDict()
     * 查找算法  精确匹配
     */
    StationPtr findFirstStation(const StationName& name);

    QList<StationPtr>
        findAllStations(const StationName& name);

    /*
     * 非严格查找算法
     * 注意与Railway的匹配机制不同：
     * 使用 equalOrBelongsTo(name)
     * 总是优先匹配严格的
     */
    StationPtr findFirstGeneralStation(const StationName& name);

    QList<StationPtr>
        findAllGeneralStations(const StationName& name);

    /*
     * qETRC新增核心函数
     * 将车次绑定到线路
     */
    void bindToRailway(std::shared_ptr<Railway> railway);

    void unbindToRailway();

    /*
     * Train.stationDown()
     * 判定局部上下行性质
     * 这个实现比较困难，现在不一定能做
     *
     * 注意: 参考pyETRC的实现，先向左查再向右查，
     * 即行别优先定义为[到达行别]
     */
    Direction stationDirection(const StationName& station);

    Direction stationDirection(int index);

    /*
     * Train.translation()  使用给定车次平移
     */
    Train translation(TrainName name, int sec);

    /*
     * 签名未确定
     */
    void jointTrain();

    void show()const;

};


