/*
 * 原pyETRC项目的LineStation类
 * 原pyETRC中直接用dict做的，这里都采用struct的模式简单实现
 */
#ifndef RAILSTATION_H
#define RAILSTATION_H
#include <optional>
#include <QString>
#include <QObject>
#include <QJsonObject>

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
    QString tracks;
    RailStation(const StationName& name_,
                double mile_,
                int level_=4,
                std::optional<double> counter_=std::nullopt
                );
    RailStation(const QJsonObject& obj);

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    QString counterStr()const;
};

#endif // RAILSTATION_H
