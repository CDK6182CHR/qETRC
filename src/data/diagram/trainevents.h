#pragma once

#include <cstdint>
#include <memory>
#include <deque>
#include <QList>
#include <vector>
#include <QVector>
#include <QString>
#include <QFlags>
#include <variant>
#include <type_traits>
#include <QTime>
#include <optional>

#include <data/common/direction.h>


/**
 * @brief The TrainEventType pyETRC.GraphicsWidget.TrainEventType
 * 列车事件表的类型
 */
enum class TrainEventType:int8_t {
    Meet,   //会车
    OverTaking,  //越行
    Avoid,   //待避
    Arrive,  //到达
    Depart,  //出发
    SettledPass,   //通过 （图定）
    CalculatedPass,    //通过 （推算）
    Origination,    //始发
    Destination,    //终到
    Coincidence,    //重合，或共线
    Undefined
};


class Train;

class RailStation;

class RailInterval;
namespace qeutil {
    QString eventTypeString(TrainEventType t);

    TrainEventType formerEventType(bool isStop);
    TrainEventType latterEventType(bool isStop);
}




/**
 * @brief The StationEvent struct
 * 站内事件的描述，有车站指针就能说明很多问题了
 * 给RailStation指针，因为可能出现推定的情况
 * 站内事件采用时间作比较。如果时间完全一样，再比较type
 */
struct StationEvent {
    TrainEventType type;
    QTime time;
    std::weak_ptr<const RailStation> station;
    std::optional<std::reference_wrapper<const Train>> another;
    QString note;
    StationEvent(TrainEventType type_,
                 const QTime& time_, std::weak_ptr<const RailStation> station_,
                 std::optional<std::reference_wrapper<const Train>> another_,
                 const QString& note_=""):
        type(type_),time(time_),station(station_),another(another_),note(note_){}

    //StationEvent(StationEvent&&)noexcept = default;
    //StationEvent(const StationEvent&) = default;

    bool operator<(const StationEvent& another)const;

    /**
     * @brief ETRC风格的字符串描述 不带换行符
     */
    QString toString()const;
};

/**
 * 2022.03.07
 * 包含事件最基本的信息，但不依赖于TrainLine的版本
 */
struct RailStationEventBase {
    enum Position {
        NoPos = 0b00,   // 都不是：基本上用于处理Avoid类型的间隔
        Pre = 0b01,   // 小里程端
        Post = 0b10,  // 大里程端
        Both = Pre | Post
    };
    Q_DECLARE_FLAGS(Positions, Position);

    TrainEventType type;
    QTime time;
    Positions pos;
    Direction dir;

    RailStationEventBase(TrainEventType type_,
        const QTime& time_, Positions pos_, Direction dir_):
        type(type_),time(time_),pos(pos_),dir(dir_){}


    struct PtrTimeComparator {
        bool operator()(const std::shared_ptr<const RailStationEventBase>& ev1,
            const std::shared_ptr<const RailStationEventBase>& ev2)const {
            return ev1->time.msecsSinceStartOfDay() < ev2->time.msecsSinceStartOfDay();
        }
        bool operator()(const QTime& tm, const std::shared_ptr<const RailStationEventBase>& ev2)const {
            return tm.msecsSinceStartOfDay() < ev2->time.msecsSinceStartOfDay();
        }
        bool operator()(const std::shared_ptr<const RailStationEventBase>& ev1, const QTime& tm)const {
            return ev1->time.msecsSinceStartOfDay() < tm.msecsSinceStartOfDay();
        }
    };

    /**
     * 当前事件是否对应于有标尺的附加时分参与
     */
    bool hasAppend()const;

    QString posString()const;
    static QString posToString(const Positions& pos);

    int secsTo(const RailStationEventBase& rhs)const;

};

namespace qeutil {

    /**
     * 列车运行方向（相对于车站）先经过一侧的绝对位置。
     * 下行为Pre，上行为Post
     */
    RailStationEventBase::Position dirFormerPos(Direction dir);

    /**
     * 列车运行方向（相对于车站）后经过一侧的绝对位置。
     * 下行为Post，上行为Pre
     */
    RailStationEventBase::Position dirLatterPos(Direction dir);
}

class TrainLine;

/**
 * 用于处理车站事件。比车次的事件多一个站前还是站后的标志位。
 * another位置用来放相关车次，固定非null。
 * pos: 1-站前事件（小里程端事件）; 2-站后；3-both
 * 2021.09.06：取消继承。修改API。
 */
struct RailStationEvent: public RailStationEventBase {

    std::weak_ptr<const RailStation> station;
    std::shared_ptr<const TrainLine> line;
    
    QString note;
    
    RailStationEvent(TrainEventType type_,
        const QTime& time_, std::weak_ptr<const RailStation> station_,
        std::shared_ptr<const TrainLine> line_,
        Positions pos_, const QString& note_ = "");


    QString toString()const;

};

//2022.03.06  改为继承 see stationeventaxis.h
//using RailStationEventList = QVector<std::shared_ptr<RailStationEvent>>;

Q_DECLARE_OPERATORS_FOR_FLAGS(RailStationEventBase::Positions)


struct AdapterStation;

/**
 * @brief The IntervalEvent struct
 * 区间事件描述  也采用时刻来排序
 */
struct IntervalEvent{
    TrainEventType type;
    QTime time;
    std::shared_ptr<const RailStation> former, latter;
    std::reference_wrapper<const Train> another;
    double mile;
    QString note;
    IntervalEvent(TrainEventType type_, const QTime& time_,
        std::shared_ptr<const RailStation> former_,
        std::shared_ptr<const RailStation> latter_,
        std::reference_wrapper<const Train> another_,
        double mile_, const QString& note_ = ""
    ) :
        type(type_), time(time_), former(former_), latter(latter_),
        another(another_), mile(mile_), note(note_) 
    {
    }

    IntervalEvent(IntervalEvent&&)noexcept = default;
    IntervalEvent(const IntervalEvent&) = default;
    IntervalEvent& operator=(const IntervalEvent&)=default;
    IntervalEvent& operator=(IntervalEvent&&)=default;

    bool operator<(const IntervalEvent& another)const;

    /**
     * @brief ETRC风格的字符串描述 不带换行符
     */
    QString toString()const;
};



class TrainItem;
class TrainAdapter;


/**
 * @brief 车站及其后区间的事件表抽象
 * 逻辑上类似QPair<QList<StationEvent>, QList<IntervalEvent>>
 * 封装出来，主要是为了做二分查找的插入操作
 */
class StationEventList{
public:
    QList<StationEvent> stEvents;
    QList<IntervalEvent> itEvents;

    StationEventList() = default;
    void emplace(StationEvent&& event);
    void emplace(IntervalEvent&& event);
};


using LineEventList=QList<StationEventList>;


/**
 * Adapter的事件描述由TrainLine的简单加和。
 */
using AdapterEventList = LineEventList;

/**
 * 列车事件表是Adapter事件表的简单组合。
 */
using TrainEventList=std::deque<QPair<std::shared_ptr<TrainAdapter>,AdapterEventList>>;

class TrainLine;

/**
 * 列车在指定时刻运行状态的描述，包括：所在站或者所在区间，里程标
 */
struct SnapEvent {
    std::shared_ptr<const TrainLine> line;
    double mile;
    using pos_t = std::variant<std::shared_ptr<const RailStation>, 
        std::shared_ptr<const RailInterval>>;
    pos_t pos;
    bool isStopped;
    QString note;
    SnapEvent(std::shared_ptr<const TrainLine> line_,
        double mile_,
        pos_t pos_,
        bool isStopped_,
        const QString& note_=""):
        line(line_),mile(mile_),pos(pos_),isStopped(isStopped_), note(note_){}

    inline bool isStationEvent()const {
        return std::holds_alternative<std::shared_ptr<const RailStation>>(pos);
    }

    inline bool operator<(const SnapEvent& another)const {
        return mile < another.mile;
    }
};

using SnapEventList = QVector<SnapEvent>;


enum class DiagnosisType {
    StopTooLong1,   // 停时超过12小时
    StopTooLong2,   // 停时超过20小时
    IntervalTooLong1,   //区间超过12小时
    IntervalTooLong2,   //区间超过20小时
    IntervalMeet,       //区间会车（可选项）
    IntervalOverTaking, //区间越行
    CollidForbid,       //与天窗冲突
    SystemError,        //内部数据问题
};

namespace qeutil {
    enum DiagnosisLevel {
        Information = 1,
        Warning = 2,
        Error = 3
    };

    QString diagnoLevelString(DiagnosisLevel level);
    QString diagnoTypeString(DiagnosisType type);
}


struct DiagnosisIssue {
    DiagnosisType type;
    qeutil::DiagnosisLevel level;

    using pos_t = std::variant<std::shared_ptr<const RailStation>,
        std::shared_ptr<const RailInterval>>;
    pos_t pos;
    std::shared_ptr<const TrainLine> line;
    QString description;

    // 2022.09.11: add mile and time info
    QTime time;
    double mile;
        
    DiagnosisIssue(DiagnosisType type_,
        qeutil::DiagnosisLevel level_,
        pos_t pos_, std::shared_ptr<const TrainLine> line_, 
        QTime time_, double mile_,
        const QString& description_ ):
        type(type_),level(level_),pos(pos_),line(line_),time(time_),mile(mile_),
        description(description_){}

    QString posString()const;

    /**
     * 2021.10.23  判定当前的Issure是否属于指定的车站范围，
     * 即mile范围有所交集；分为Station和Interval两种情况。
     * 用于筛选Issue；输入应保证属于同一条线路（但无所谓）；保证start, end非空（不做检查）
     */
    bool inRange(std::shared_ptr<RailStation> start, std::shared_ptr<RailStation> end)const;
};

static_assert(std::is_same_v<SnapEvent::pos_t, DiagnosisIssue::pos_t>, "");

using DiagnosisList = QVector<DiagnosisIssue>;

