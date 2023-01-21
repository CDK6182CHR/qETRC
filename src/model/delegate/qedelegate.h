#pragma once
#include <Qt>
#include <optional>
#include <QVariant>
#include <memory>
#include "data/diagram/trainline.h"
#include "data/rail/railstation.h"   // this is for Metatype register of RailStation

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
    PredefTrainFilterPointerRole,
};
}

//Q_DECLARE_METATYPE(std::optional<double>);
Q_DECLARE_METATYPE(std::shared_ptr<TrainLine>);
