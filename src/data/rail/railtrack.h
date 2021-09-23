#pragma once



#include <memory>
#include <QTime>
#include <set>
#include <list>
#include <optional>
#include <QColor>
#include "data/common/direction.h"
#include "data/common/stationname.h"

class TrainStation;
class TrainLine;
/**
 * @brief 占用股道的元素，包括停车、通过等。
 */
struct TrackItem{
    enum StayType{
        Stop=0,
        Pass=1,
        Link=2,
        Departure,
        Destination
    };
    QString title;
    StationName stationName;
    QTime beginTime,endTime;
    StayType type;
    /**
     * @brief line
     * 对于Link类型，保存区间的line
     */
    std::shared_ptr<TrainLine> line;
    Direction dir;
    QString specTrack;    // 指定股道（图定股道） 不包含推定的

    using train_st_t=std::optional<TrainStation*>;
    /**
     * 记录时刻表中的车站。2仅在LinkItem中启用。
     * 由于有效性可能出现问题，所以尽量不要调用；目前仅在保存股道信息时使用。
     */
    train_st_t trainStation1,trainStation2;

    TrackItem(const QString& title, const StationName& stationName, const QTime& beginTime,
              const QTime& endTime, StayType type, std::shared_ptr<TrainLine> line,
              QString specTrack,const train_st_t& st1,
              const train_st_t& st2=std::nullopt);

    QString typeString()const;
    QString toString()const;

    bool isStopped()const;

    /**
     * 占用股道的时间。主要是考虑通过列车需延拓至1分钟。
     */
    std::pair<QTime, QTime> occpiedRange()const;

};

/**
 * 股道的一段占用情况。和TrackItem无异，主要是跨日的多一层处理。
 * fromLeft: 不认item的起始时刻，而是以0点起始；toRight: 以2359为终止。
 */
struct TrackOccupy {
    static const QTime LEFT, RIGHT;
    std::shared_ptr<TrackItem> item;
    bool fromLeft, toRight;
    TrackOccupy(const std::shared_ptr<TrackItem> item, bool fromLeft = false,
        bool toRight = false):
        item(item),fromLeft(fromLeft),toRight(toRight){}
    const QTime& fromTime()const;
    const QTime& toTime()const;
    std::pair<QTime,QTime> occupiedRange()const;

    // 注意：按照终止时间排序！！
    struct Comparator {
        using is_transparent = std::true_type;
        bool operator()(const TrackOccupy& occ1, const TrackOccupy& occ2)const {
            return occ1.toTime() < occ2.toTime();
        }
        bool operator()(const TrackOccupy& occ, const QTime& other)const {
            return occ.toTime() < other;
        }
        bool operator()(const QTime& tm, const TrackOccupy& occ)const {
            return tm < occ.toTime();
        }
    };
};


/**
 * 一条股道的描述；包含插入以及判断占用等操作。
 */
class Track : public std::set<TrackOccupy, TrackOccupy::Comparator>
{
    QString _name;
    using _Base=std::set<TrackOccupy, TrackOccupy::Comparator>;
public:
    using _Base::set;  //构造函数
    Track(const QString& name): _name(name){}

    const QString& name()const{return _name;}
    void setName(const QString& name){_name=name;}

    /**
     * 返回指定时间范围内，存在占用的第一个迭代器。
     * 注意，这里不考虑间隔时间，即是：间隔时间应该已经被考虑在tm1, tm2中。
     * 如果没有，返回一个end()。
     */
    const_iterator occupiedInRange(const QTime& tm1, const QTime& tm2)const;

    /**
     * 返回指定时段是否占用。
     */
    bool isOccupied(const QTime& tm1, const QTime& tm2)const;

    /**
     * 考虑单线冲突检测时需要用到；
     * seealso `occupiedInRange`
     * 但这里直接考虑到了股道占用间隔的要求。
     * 由于跨日要分情况比较复杂，这里直接用暴力算法
     */
    const_iterator conflictItem(const std::shared_ptr<TrackItem>& item,
                                int sameSplitSecs,
                                int oppsiteSplitSecs)const;

    /**
     * 判定指定车次停靠范围能否放进当前股道。需考虑间隔。
     * 注意同向间隔和对向间隔是不一样的。
     */
    bool isIdleFor(const std::shared_ptr<TrackItem>& item,
                   int sameSplitSecs,
                   int oppsiteSplitSecs)const;

    /**
     * 已知双线模式下的判空操作.. 不需要考虑间隔类型，更简单
     */
    bool isIdleForDouble(const std::shared_ptr<TrackItem>& item,
                   int sameSplitMinu)const;

    void addItem(const std::shared_ptr<TrackItem>& item);
};
