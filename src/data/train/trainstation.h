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

/*
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
};

QDebug operator<<(QDebug debug, const TrainStation& ts);

