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

    /**
     * @brief timeCompare  全局函数 考虑周期边界条件下的时间比较
     * 采用能够使得两时刻之间所差时长最短的理解方式来消歧
     * @return tm1 < tm2  tm1是否被认为在tm2之前
     */
    bool timeCompare(const QTime& tm1, const QTime& tm2);
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

class TrainLine;

/**
 * 用于处理车站事件。比车次的事件多一个站前还是站后的标志位。
 * another位置用来放相关车次，固定非null。
 * pos: 1-站前事件（小里程端事件）; 2-站后；3-both
 * 2021.09.06：取消继承。修改API。
 */
struct RailStationEvent {

    enum Position {
        NoPos=0b00,   // 都不是：基本上用于处理Avoid类型的间隔
        Pre = 0b01,   // 小里程端
        Post = 0b10,  // 大里程端
        Both = Pre | Post
    };
    Q_DECLARE_FLAGS(Positions, Position);

    TrainEventType type;
    QTime time;
    std::weak_ptr<const RailStation> station;
    std::shared_ptr<const TrainLine> line;
    Positions pos;
    QString note;
    
    RailStationEvent(TrainEventType type_,
        const QTime& time_, std::weak_ptr<const RailStation> station_,
        std::shared_ptr<const TrainLine> line_,
        Positions pos_, const QString& note_ = "") :
        type(type_),time(time_),station(station_),line(line_),pos(pos_),
        note(note_){}

    QString posString()const;
    QString toString()const;

    static QString posToString(const Positions& pos);

    /**
     * 当前事件是否对应于有标尺的附加时分参与
     */
    bool hasAppend()const;
};

using RailStationEventList = QVector<std::shared_ptr<RailStationEvent>>;

Q_DECLARE_OPERATORS_FOR_FLAGS(RailStationEvent::Positions)


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
        
    DiagnosisIssue(DiagnosisType type_,
        qeutil::DiagnosisLevel level_,
        pos_t pos_, std::shared_ptr<const TrainLine> line_, const QString& description_ ):
        type(type_),level(level_),pos(pos_),line(line_),
        description(description_){}

    QString posString()const;
};

static_assert(std::is_same_v<SnapEvent::pos_t, DiagnosisIssue::pos_t>, "");

using DiagnosisList = QVector<DiagnosisIssue>;

