#include "trainadapterstation.h"

#include "data/rail/railstation.h"

bool AdapterStation::operator==(const AdapterStation& other) const
{
    return trainStation == other.trainStation && railStation.lock() == other.railStation.lock();
}

bool AdapterStation::operator<(double y) const
{
    return railStation.lock()->y_coeff.value() < y;
}

double AdapterStation::yCoeff() const
{
    return railStation.lock()->y_coeff.value();
}

bool operator<(double y, const AdapterStation& adp)
{
    return y < adp.railStation.lock()->y_coeff.value();
}

