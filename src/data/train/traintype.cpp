#include "traintype.h"

bool TrainType::operator==(const TrainType &other) const
{
    return _name==other._name && _pen==other._pen&&_passenger==other._passenger;
}

bool TrainType::operator!=(const TrainType &other) const
{
    return ! operator==(other);
}

void TrainType::swap(TrainType &other)
{
    std::swap(_name,other._name);
    std::swap(_pen,other._pen);
    std::swap(_passenger,other._passenger);
}
