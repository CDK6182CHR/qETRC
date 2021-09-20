#pragma once
#include <memory>
#include <QVector>
#include <QVariant>

class RailStation;
class TrainLine;
/**
 * @brief The TrainStationBounding struct
 * 车次时刻表中车站绑定到对象的描述，事实上是Rail部分的描述
 * 车站绑定信息的描述，其实就是TrainLine指针+RailStationBouding
 * 目前仅用于TimetableQuickWidget/Model
 * 本来该用AdapterStation; 但其安全性无法保证，因此改用weak_ptr<RailStation>
 */
struct TrainStationBounding
{
    std::weak_ptr<TrainLine> line;
    std::weak_ptr<RailStation> railStation;
    TrainStationBounding(std::weak_ptr<TrainLine> line,
        std::weak_ptr<RailStation> railst):
        line(line),railStation(railst){}
};

using TrainStationBoundingList=QVector<TrainStationBounding>;
Q_DECLARE_METATYPE(TrainStationBoundingList)
