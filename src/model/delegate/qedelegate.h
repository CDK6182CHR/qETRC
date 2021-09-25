#pragma once
#include <Qt>
#include <optional>
#include <QVariant>
#include <memory>
#include "data/rail/railstation.h"
#include "data/diagram/trainline.h"

namespace qeutil {
enum {
    DoubleDataRole=Qt::UserRole+100,
    IntervalRole,
    TrainLineRole,
    RailStationRole,
    RulerNodeRole,
    TrainRole,
    BoolDataRole,
    TrainTypeRole,
    RoutingRole,
    TrainStationRole,
    TimeDataRole,
    TrainGapRole,
    RailwayRole,
    GraphVertexRole,
    GraphEdgeRole,
};
}

//Q_DECLARE_METATYPE(std::optional<double>);
Q_DECLARE_METATYPE(std::shared_ptr<TrainLine>);
