#pragma once

#include <cstdint>
#include <QList>
#include <QJsonObject>
#include <list>
#include "trainname.h"
#include "trainstation.h"
#include "data/diagram/trainline.h"
#include "data/diagram/config.h"

enum class TrainPassenger:
        std::int8_t{
    False=0,
    Auto=1,
    True=2
};

class TrainUIConfig{

};

/**
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

    /**
     * 暂定采用std::list来放时刻表。
     * std::list::iterator当作指针来使用；
     * 主要原因是考虑双向迭代的效率
     * 时刻表拥有内部所有结点的所有权
     * 利用std::list的迭代器以及引用不会失效的特性
     * 涉及到删除操作时，要特别小心
     */
    std::list<TrainStation> _timetable;

    std::weak_ptr<Railway> _boundRail;

    /**
     * 运行线数据描述以及对象指针
     * 对数据结构似乎没有特殊要求，暂定QList<shared_ptr>的结构
     */
    QList<std::shared_ptr<TrainLine>> _lines;

    /**
     * Train.autoItems  是否采用自动运行线管理
     */
    bool _autoLines;

    //QtWidgets.QGraphicsViewPathItem pathItem;
    //QtWidgets.QGraphicsViewItem labelItem;

    //todo: 交路，随机访问Map表等

public:
    using StationPtr=std::list<TrainStation>::iterator;
    using ConstStationPtr=std::list<TrainStation>::const_iterator;

    Train(const TrainName& trainName,
          const StationName& starting=StationName::nullName,
          const StationName& terminal=StationName::nullName,
          TrainPassenger passenger=TrainPassenger::Auto);
    explicit Train(const QJsonObject& obj);

    /**
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
    inline bool isNullStation(ConstStationPtr st)const { return st == _timetable.end(); }

    const std::list<TrainStation>& timetable()const{return _timetable;}
    std::list<TrainStation>& timetable(){return _timetable;}

    void appendStation(const StationName& name,
                       const QTime& arrive,
                       const QTime& depart,
                       bool business=true,
                       const QString& track="",
                       const QString& note="");

    void prependStation(const StationName& name,
        const QTime& arrive,
        const QTime& depart,
        bool business = true,
        const QString& track = "",
        const QString& note = "");

    /**
     * Train.stationDict()
     * 查找算法  精确匹配
     */
    StationPtr findFirstStation(const StationName& name);

    QList<StationPtr>
        findAllStations(const StationName& name);

    ConstStationPtr findFirstStation(const StationName& name)const;

    QList<ConstStationPtr>
        findAllStations(const StationName& name)const;

    /**
     * 非严格查找算法
     * 注意与Railway的匹配机制不同：
     * 使用 equalOrBelongsTo(name)
     * 总是优先匹配严格的
     */
    StationPtr findFirstGeneralStation(const StationName& name);

    QList<StationPtr>
        findAllGeneralStations(const StationName& name);


    /**
     * qETRC新增核心函数
     * 将车次绑定到线路
     * 注意：如果已经绑定到同一条线路，不做任何事
     */
    void bindToRailway(std::shared_ptr<Railway> railway);

    /**
     * 适用于线路可能发生变化时，
     * 即使已经绑定到同一条线路，也会撤销再重来
     */
    void updateBoundRailway(std::shared_ptr < Railway> railway);

    void unbindToRailway();

    inline bool isBoundToRailway()const { return !_boundRail.expired(); }

    /**
     * @brief autoLines 自动设置运行线数据
     * 承担了pyETRC中TrainItem::setItem()中划分Item的工作
     * @param railway 绑定到的线路
     * @param config 配置信息
     */
    void autoLines(std::shared_ptr<Railway> railway, const Config& config);

    /**
     * 车次运行线信息表
     * Train.itemInfo()
     */
    auto& lines(){return _lines;}
    auto& lines()const{return _lines;}

    /**
     * 时刻表中前一个（不包含当前）成功绑定到线路的车站。
     * 通过本车次指针向前索引的算法。
     */
    StationPtr prevBoundStation(StationPtr st);

    StationPtr nextBoundStation(StationPtr st);

    ConstStationPtr prevBoundStation(ConstStationPtr st)const;

    ConstStationPtr nextBoundStation(ConstStationPtr st)const;

    /**
     * Train.stationDown()
     * 判定局部上下行性质
     * 注意调用了线性算法来查找车站 （非精确查找）
     *
     * 注意: 参考pyETRC的实现，先向左查再向右查，
     * 即行别优先定义为[到达行别]
     */
    Direction stationDirection(const StationName& station);

    /**
     * 注意：强制要求只有当前车站已经绑定到线路时，
     * 才计算结果。
     * 通过先查找上一站、再查找下一站的算法，由所绑定的两个车站的上下行关系判定
     * 准常数复杂度
     */
    Direction stationDirection(ConstStationPtr station);

    /**
     * Train.translation()  使用给定车次平移
     */
    Train translation(TrainName name, int sec);

    /**
     * Train.delNonLocal()
     * 删除非本线车站 
     */
    void removeNonLocal();

    /**
     * 注意：右值引用，移动语义
     * 调用前，非本线车站已经删除，都已经绑定到同一条线路
     * former: 对方在本线之前
     * 删除原有线性算法：只检查交叉处的几个站是否重合 （站名一致）
     * 最多检查至10个站 （检查的过程没有做优化，平方复杂度，但深度较小，因此关系不大）
     * 
     * 已通过单元测试
     */
    void jointTrain(Train&& train, bool former);

    void show()const;

    inline bool isStartingStation(const StationName& s)const {
        return starting().equalOrBelongsTo(s);
    }
    inline bool isTerminalStation(const StationName& s)const {
        return terminal().equalOrBelongsTo(s);
    }

    /**
     * Train.intervalExchange() 区间换线
     * 注意start,end都包含在内
     */
    void intervalExchange(Train& train2, StationPtr start1, StationPtr end1,
        StationPtr start2, StationPtr end2);

    /**
     * @brief Train.detectPassStation()  推定通过站时刻
     */
    void detectPassStation();

};


