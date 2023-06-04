#pragma once

#include <QGraphicsRectItem>
#include "data/train/stationpoint.h"


struct AdapterStation;
/**
 * @brief The PaintStationPointItem class
 * 2023.06.04  铺画点
 * 运行线上突出显示的一个点，用正方形图元实现。参见ETRC。
 */
class PaintStationPointItem : public QGraphicsRectItem
{
    StationPoint _point;
    AdapterStation*const _station;
public:
    enum { Type = UserType + 2 };
    PaintStationPointItem(StationPoint point, AdapterStation* station,
                          double x, double y, QGraphicsItem* parent);

    auto point()const {return _point;}
    auto* station()const {return _station;}
    inline int type()const override { return Type; }
};

