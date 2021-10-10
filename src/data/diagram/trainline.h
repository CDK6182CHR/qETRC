#pragma once

#include <memory>
#include <deque>
#include <optional>
#include <tuple>
#include <cstdint>
#include <QPair>
#include <QList>

#include "trainevents.h"
#include "data/common/direction.h"

class Ruler;
class Railway;
class StationName;
class TrainStation;
class Train;


/**
 * @brief The AdapterNode struct
 * 一个铺画的车站
 */
struct AdapterStation{
    std::list<TrainStation>::iterator trainStation;
    std::weak_ptr<RailStation> railStation;
    AdapterStation(std::list<TrainStation>::iterator trainStation_,
        std::weak_ptr<RailStation> railStation_):
        trainStation(trainStation_),railStation(railStation_){}
    bool operator<(double y)const;
    double yCoeff()const;
};

bool operator<(double y, const AdapterStation& adp);



class TrainCollection;


/**
 * @brief TrainLine  车次运行线的数据描述  按照class组织
 * 由TrainAdapter管理
 * 扩展原Train中Item的描述类
 * 2021.06.22  增加绑定序列类
 * 原则上只允许TrainAdapter修改；外面的类只读。
 */
class TrainLine:
    public std::enable_shared_from_this<TrainLine>
{
    friend class TrainAdapter;
    TrainAdapter& _adapter;

    /**
     * 采用std::deque实现，保证指针和引用的安全性，但注意迭代器仍然是不安全的！
     */
    std::deque<AdapterStation> _stations;
    Direction _dir;
    bool _show;
    bool _startLabel, _endLabel;
    
    /**
     * 迭代器使用注意：全程必须保证_stations不变
     * 即仅能在const下使用
     * 原则上，迭代器不能外传
     */
    using ConstAdaPtr = std::deque<AdapterStation>::const_iterator;

public:

    enum IntervalAttachType {
        AttachNone = 0b00,
        AttachStart = 0b01,
        AttachStop = 0b10,
        AttachBoth = 0b11
    };

    /**
     * 构造空的运行线，默认显示Item和双标签
     */
    TrainLine(TrainAdapter& adapter);
    TrainLine(const TrainLine&) = default;
    TrainLine(TrainLine&&) = default;

    inline void addStation(std::list<TrainStation>::iterator trainStation,
        std::weak_ptr<RailStation> rs) {
        _stations.emplace_back(trainStation, rs);
    }

    inline void pop_back() {
        _stations.pop_back();
    }

    inline auto count()const { return _stations.size(); }

    void print()const;

    inline bool show()const { return _show; }
    void setIsShow(bool on);

    inline Direction dir()const { return _dir; }
    inline bool startLabel()const { return _startLabel; }
    inline bool endLabel()const { return _endLabel; }

    /**
     * 注意：原则上不允许在中间修改
     */
    auto& stations() { return _stations; }
    const auto& stations()const { return _stations; }
    auto& adapter() { return _adapter; }
    std::shared_ptr<Train> train();
    std::shared_ptr<const Train> train()const;

    const StationName& firstStationName()const;

    const StationName& lastStationName()const;


    inline std::shared_ptr<RailStation>
        firstRailStation(){
        return _stations.front().railStation.lock();
    }

    inline std::shared_ptr<const RailStation>
        firstRailStation()const {
        return _stations.front().railStation.lock();
    }

    inline auto firstTrainStation() {
        return _stations.front().trainStation;
    }

    inline std::shared_ptr<RailStation>
        lastRailStation(){
        return _stations.back().railStation.lock();
    }

    inline std::shared_ptr<const RailStation>
        lastRailStation()const {
        return _stations.back().railStation.lock();
    }

    inline bool isNull()const { return _stations.empty(); }

    bool startAtThis()const;

    bool endAtThis()const;

    /**
     * @brief listLineEvents 列出本运行线事件表
     * 遍历其他所有运行线获取数据。
     */
    LineEventList listLineEvents(const TrainCollection& coll)const;

    /**
     * 列车运行情况诊断，判断可能存在的问题。
     * 采用和`listLineEvents`类似的框架。
     */
    DiagnosisList diagnoseLine(const TrainCollection& coll, bool withIntMeet)const;

    inline const AdapterStation* lastStation()const {
        return _stations.empty() ? nullptr : &(_stations.back());
    }
    inline const AdapterStation* firstStation()const {
        return _stations.empty() ? nullptr : &(_stations.front());
    }

    /**
     * 本运行线的总时长。
     * 假定总时长小于24小时。
     * 存在折返的列车，不计首站停时。
     */
    int totalSecs()const;

    /**
     * 运行时间和停车时间
     * 其中停车时间不计算始发终到；折返的车，不计算首站。
     * 返回：运行时间，停车时间。逐个区间扫描。
     */
    std::pair<int, int> runStaySecs()const;

    /**
     * 本运行线总里程，终点减起点的绝对值
     */
    double totalMile()const;


    /**
     * 由指定y，通过二分查找，返回相应的车站。
     * 返回车站采用：运行方向的区间后站。
     * 返回车站采用std::lower_bound: iterator to he first element that not less than `y`.
     * 对上行采用反迭代器查
     * ！！注意迭代器有效性！！
     * 2021.10.10  注意入参是coeff而不是绝对坐标
     */
    std::deque<AdapterStation>::const_iterator
        stationFromYCoeff(double y)const;

    /**
     * 注意  不标注起点标签的线，首站不返回。
     * 2021.08.01  改为二分查找
     */
    const AdapterStation* stationFromRail(std::shared_ptr<RailStation> rail)const;

    /**
     * 标尺排图中，初始化选择起始站使用。
     * 线性查找。
     */
    const AdapterStation* stationByTrainLinear(std::list<TrainStation>::const_iterator st)const;

    /**
     * 基于事件的实现版本  二分查找
     * 包含推定。到达、出发要拆开，因此返回QList
     */
    RailStationEventList
           stationEventFromRail(std::shared_ptr<const RailStation> rail)const;

    /**
     * 计算通过指定纵坐标处的时刻；如果运行线不经过该点，返回空
     * 如果指定纵坐标恰好是某一车站，则以【左区间】为准，
     * 即下行列车到达时刻、上行列车出发时刻。
     */
    std::optional<QTime> sectionTime(double y)const;

    /**
     * 返回指定时刻列车的运行状态。
     */
    SnapEventList getSnapEvents(const QTime& time)const;

    /**
     * 所给站是否是始发站。seealso `hasStartAppend`
     */
    bool isStartingStation(ConstAdaPtr st)const;
    bool isStartingStation(const AdapterStation* st)const;

    bool isTerminalStation(ConstAdaPtr st)const;
    bool isTerminalStation(const AdapterStation* st)const;

    /**
     * 所给站站后区间是否包含起步附加，即所给站为始发站，或停车
     */
    bool hasStartAppend(ConstAdaPtr st)const;

    /**
     * 所给站站前区间是否包含停车附加，即所给站为终到站，或停车
     */
    bool hasStopAppend(ConstAdaPtr st)const;

    /**
     * 所给区间的附加情况，例如起，始停之类的；不包含“通”。
     */
    QString appStringShort(ConstAdaPtr prev, ConstAdaPtr cur)const;

    /**
     * 所给区间附加情况，始发视作起步。
     */
    IntervalAttachType getIntervalAttachType(ConstAdaPtr prev, ConstAdaPtr cur)const;

    /**
     * 附加类型转字符串；不考虑始发终到，不包括“通”
     */
    static QString attachTypeString(IntervalAttachType type);

    /**
     * 附加类型转字符串；不考虑始发终到，包括“通”
     */
    static QString attachTypeStringFull(IntervalAttachType type);

    std::shared_ptr<const Railway> railway()const;
    std::shared_ptr<Railway> railway();

    /**
     * 自动设置营业站信息
     * 客运列车在办客站停车/始发/终到视为营业  其他情况类推
     * 当前这个对象应该是复制的Train对象下面的，并且已经特别绑定到线路
     */
    bool autoBusiness();

private:

    /**
     * @brief listStationEvents
     * 列出每个站的到开时刻，这个很简单
     */
    void listStationEvents(LineEventList& res)const;

    /**
     * @brief detectPassStations  推定区间通过站时刻
     * 注意通过站时刻放在左边那个站的StationEvent里面
     * @param index  区间左端点下标，即插入位置
     * @param itr 区间左端点 迭代器
     */
    void detectPassStations(LineEventList& res, int index, ConstAdaPtr itr)const;

    /**
     * @brief eventsWithSameDir  同向列车事件表：越行、待避、共线
     * @param another  已知同向的另一段列车运行线
     */
    void eventsWithSameDir(LineEventList& res, const TrainLine& another, const Train& antrain)const;

    /**
     * @brief eventsWithCounter 反向列车事件表：主要是交会
     * 反向列车用反向迭代器，这样保证y的方向是单向变化的
     */
    void eventsWithCounter(LineEventList& res, const TrainLine& another, const Train& antrain)const;

    void diagnoWithSameDir(DiagnosisList& res, const TrainLine& another, const Train& antrain)const;

    /**
     * @brief eventsWithCounter 反向列车事件表：主要是交会
     * 反向列车用反向迭代器，这样保证y的方向是单向变化的
     */
    void diagnoWithCounter(DiagnosisList& res, const TrainLine& another, const Train& antrain)const;

    /**
     * @brief 当前运行线的y最小值，等于首站或者末站（取决于行别）的y值
     * 前提条件是已经铺画过（y数值非0），否则会出错
     */
    double yMin()const;

    double yMax()const;

    /**
     * @brief yComp 两个站的y坐标比较
     * 如果y(st1)<y(st2)，返回-1.
     * 采用模版，是为了对付反向迭代器
     */
    template <typename ForwardIter1, typename ForwardIter2>
    inline int yComp(ForwardIter1 st1, ForwardIter2 st2)const {
        double y1 = st1->railStation.lock()->y_coeff.value(),
            y2 = st2->railStation.lock()->y_coeff.value();
        if (y1 == y2)
            return 0;
        else if (y1 < y2)
            return -1;
        return +1;
    }

    /**
     * @brief xComp 两个时刻，也即横坐标的比较
     * 如果tm1在tm2之前，返回-1.
     * 注意周期边界条件PBC下的歧义性。总是采取使得tm1, tm2距离绝对值最小的理解来消除歧义
     */
    int xComp(const QTime& tm1, const QTime& tm2)const;

    /**
     * @brief sameDirStep 同向的条件下，依据y坐标比较情况和行别，判定谁该进一步。
     * @param ycond: 0, +1, -1 三种情况
     * @param pme: this的迭代器  注意是引用，这里会更改！！
     * @param phe: 对方的迭代器 引用！
     * 保证传入的不是end()
     */
    template <typename ForwardIter1,typename ForwardIter2>
    inline void sameDirStep(int ycond, ForwardIter1& pme, ForwardIter2& phe,
        ForwardIter1& mylast, ForwardIter2& hislast,int& index)const {
        if (ycond == 0) {
            //同一站，共进一步
            mylast = pme;
            ++pme;
            ++index;
            hislast = phe;
            ++phe;
        }
        else if (dir() == Direction::Down && ycond < 0 ||
            dir() == Direction::Up && ycond>0) {
            //下行时，我的y较小，即比较落后，因此进一步
            mylast = pme;
            ++index;
            ++pme;
        }
        else {
            hislast = phe;
            ++phe;
        }
    }

    /**
     * @brief getPreviousPassedTime 推定前一站通过的时刻。
     * 用在同向事件表生成时，判定是否有区间越行情况。按照y值计算
     * @param st 跨越区间后的一站。这一站到其前序的区间，包含目标站。保证不是begin()
     * @param target 要推定时刻的目标车站。
     * @return 目标站的时刻，以【毫秒数】表示！！
     */
    //template <typename BidirIter>
    int getPreviousPassedTime(ConstAdaPtr st, std::shared_ptr<RailStation> target)const;
    

    int getPreviousPassedTime(std::deque<AdapterStation>::const_reverse_iterator st,
        std::shared_ptr<RailStation> target)const; 

    /**
     * @brief findIntervalIntersectionSameDIr 判断同向区间交点。
     * 解析几何方法判定交点是否存在，用斜率判断是越行还是待避。
     * @return 如果存在合法交点，返回交点的里程、时刻以及类型；否则返回std::nullopt
     */
    std::optional<std::tuple<double, QTime, TrainEventType>>
        findIntervalIntersectionSameDir(ConstAdaPtr mylast, ConstAdaPtr mythis,
            ConstAdaPtr hislast, ConstAdaPtr histhis)const;

    /**
     * @brief findIntervalIntersectionCounter 判断对向区间交点。
     * 基本照抄同向的；但是只需要判定是否有交叉。
     * 代码重复比较多。但为了避免在.h文件里搞一大堆代码，似乎只能这样
     */
    std::optional<std::tuple<double, QTime, TrainEventType>>
        findIntervalIntersectionCounter(ConstAdaPtr mylast, ConstAdaPtr mythis,
            std::deque<AdapterStation>::const_reverse_iterator hislast,
            std::deque<AdapterStation>::const_reverse_iterator histhis)const;

    /**
     * 封装添加区间事件的操作，相比简单的添加，主要是增加了区间的判断。
     * 对于列车时刻表区间非本线的小区间的情况，对线路做二分搜索确定具体区间。
     * 当然，这样搜索出来，可能会得到站内事件。仅有恰好等于y坐标时（几乎不可能），才判为站内事件。
     * index: 要插入的目标位置
     * 注意搜索按照里程搜。这样可以保证被搜索序列递增 （用户设置线路数据时，添加强制检查）
     */
    void addIntervalEvent(LineEventList& res, int index, TrainEventType type, const QTime& time,
        const AdapterStation& former, const AdapterStation& latter,
        std::reference_wrapper<const Train> another, double mile,
        const QString& note = "")const;

    /**
     * SnapEvent使用 将former->latter这个区间压缩到实际的线路区间，线性探查。
     * 首先要通过时间定为里程标。
     * 
     */
    SnapEvent::pos_t compressSnapInterval(ConstAdaPtr former, ConstAdaPtr latter,
        double mile)const;

    double snapEventMile(ConstAdaPtr former, ConstAdaPtr latter, const QTime& time)const;

    /**
     * 判定st所示站是否在mile所示里程的运行方向前方或相等，即下行时里程比它小、上行时里程比它大
     * 简单地比较里程
     */
    bool mileBeforeEq(std::shared_ptr<const RailStation> st, double mile)const;

    bool mileAfterEq(std::shared_ptr<const RailStation> st, double m2)const;
    
    QString stationString(const AdapterStation& st)const;

    /**
     * @brief 判断两站中是否存在始发终到站，用于同向交互事件。
     * 如果存在始发终到，则不允许发生越行。
     */
    bool notStartOrEnd(const TrainLine& another, ConstAdaPtr pme, ConstAdaPtr phe)const;

    inline bool isStartingOrTerminal(ConstAdaPtr st)const {
        return isStartingStation(st) || isTerminalStation(st);
    }

    /**
     * 判断所给的图定通过事件是站前，站后还是both
     * 根据是否是首站、末站，以及行别
     */
    RailStationEvent::Position passStationPos(ConstAdaPtr st)const;

    /**
     * 单车次问题诊断，即自己时刻表直接能看出的问题
     * 停时过长以及天窗冲突
     */
    void diagnoseSelf(DiagnosisList& res)const;

    /**
     * 在各个小区间依次判定  注意需要推定各个中间站的通过时刻
     */
    void diagnoInterval(DiagnosisList& res, ConstAdaPtr prev, ConstAdaPtr cur)const;

    void diagnoForbid(DiagnosisList& res, std::shared_ptr<const RailInterval> railint,
        const QTime& in, const QTime& out)const;

    /**
     * 时刻插值的具体实现，由Adapter转发过来。
     * toBegin, toEnd转换为关于本次列车（本运行线）的数据。由Adapter负责保证本运行线
     * 是第一或最后运行线；但如果当前时刻的首站、末站为始发终到站，则不进行外插，
     * 由本函数进行这个判断。
     * 暂定新推定的时刻直接进行绑定。
     */
    void timetaleInterpolation(std::shared_ptr<const Ruler> ruler, bool toBegin,
        bool toEnd, int precision);

};


//以前的版本...
/*
 * 暂定不维护Train指针或引用。
 * 这里Train&只用来读取车站信息，构建weak_ptr
 * 注意这里需要遍历一遍时刻表获得指针，因此较慢
 */
//    TrainLine(const QJsonObject& obj, Train& train);

//    void fromJson(const QJsonObject& obj, Train& train);
//    QJsonObject toJson()const;


