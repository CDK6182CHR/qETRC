/*
 * TrainStation
 * 对应原pyETRC用于表示车站的dict
 */
#pragma once

#include <memory>
#include <utility>

#include <QTime>
#include <QDebug>

#include "data/common/stationname.h"
#include "data/rail/railstation.h"

/**
 * 注：目前支持在（绑定到相同线路的）不同车次间移动时不出问题。
 * jointTrain()方法依赖这一特性
 */
class TrainStation
{
public:
    StationName name;
    QTime arrive,depart;
    bool business;    //是否办理业务
    QString track;
    QString note;

    static constexpr int msecsOfADay = 24 * 3600 * 1000;
    
    TrainStation(const StationName& name_, const QTime& arrive_,
        const QTime& depart_, bool business_ = true,
        const QString& track_ = "", const QString& note_="");
    explicit TrainStation(const QJsonObject& obj);

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    inline bool isStopped()const {
        return arrive != depart;
    }
    
    int stopSec()const;

    static bool nameEqual(const TrainStation& t1, const TrainStation& t2);

    /**
     * @brief timeInStoppedRange 判定所给时间是否在本站的停车时间范围内。
     * 注意周期边界条件的消歧约定。
     * @param msecs  要判定的时间，以毫秒数表示
     */
    bool timeInStoppedRange(int msecs)const;

    /**
     * 判断两车站停车时间是否存在交集。注意PBC约定
     */
    bool stopRangeIntersected(const TrainStation& another)const;

    /**
     * 停车时间的字符串表示：x分，或者x分x秒
     */
    QString stopString()const;

    bool operator==(const TrainStation& other)const;

    inline bool operator!=(const TrainStation& other)const { return !(*this == other); }

    /**
     * 时间标记。如果不停车，用...替代
     */
    QString timeStringCompressed()const;
};

QDebug operator<<(QDebug debug, const TrainStation& ts);

