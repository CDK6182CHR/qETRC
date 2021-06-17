/*
 * 原pyETRC项目的LineStation类
 * 原pyETRC中直接用dict做的，这里都采用struct的模式简单实现
 */
#ifndef RAILSTATION_H
#define RAILSTATION_H
#include <optional>
#include <QtCore>

#include "stationname.h"

enum class PassedDirection {
    NoVia=0b00,
    DownVia=0b01,
    UpVia=0b10,
    BothVia=0b11
};

class RailStation
{
public:
    StationName name;
    double mile;
    int level;
    std::optional<double> counter;
    double y_value;
    PassedDirection direction;
    bool show;
    bool passenger,freight;
    QList<QString> tracks;
    RailStation(const StationName& name_,
                double mile_,
                int level_=4,
                std::optional<double> counter_=std::nullopt,
                PassedDirection direction_=PassedDirection::BothVia
                );
    RailStation(const QJsonObject& obj);

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    QString counterStr()const;

    inline bool isDownVia()const {
        return direction == PassedDirection::DownVia ||
            direction == PassedDirection::BothVia;
    }

    inline bool isUpVia()const {
        return direction == PassedDirection::UpVia ||
            direction == PassedDirection::BothVia;
    }

    inline bool isDirectionVia(bool down)const {
        return down ? isDownVia() : isUpVia();
    }
};

#endif // RAILSTATION_H
