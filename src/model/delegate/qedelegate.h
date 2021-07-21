#pragma once
#include <Qt>
#include <optional>
#include <QVariant>
#include "data/rail/railstation.h"

namespace qeutil {
enum {
    DoubleDataRole=Qt::UserRole+100,
    IntervalRole,
};
}

//Q_DECLARE_METATYPE(std::optional<double>);
