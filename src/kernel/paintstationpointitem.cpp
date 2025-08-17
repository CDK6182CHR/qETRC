#include "paintstationpointitem.h"
#include "data/common/qesystem.h"

#define RADIUS SystemJson::get().station_mark_radius

PaintStationPointItem::PaintStationPointItem(StationPoint point, AdapterStation *station,
                                             double x, double y, QGraphicsItem *parent):
    QGraphicsRectItem(x-RADIUS, y-RADIUS, 2*RADIUS, 2*RADIUS, parent),
    _point(point), _station(station)
{

}

#undef RADIUS
