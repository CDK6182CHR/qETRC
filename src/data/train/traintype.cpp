#include "traintype.h"

#include <QDebug>

TrainType::TrainType(const QString &name, const QPen &pen, bool passenger) :
    _name(name), _pen(pen),_passenger(passenger)
{
//    qDebug()<<"TAG1040: trainType contruct: "<<_name<<Qt::endl;
}

//TrainType::TrainType(const TrainType &other):
//    _name(other._name), _pen(other._pen), _passenger(other._passenger)
//{
//    qDebug()<<"TAG1042: trainType copy: "<<_name<<Qt::endl;
//}

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



//TrainType::~TrainType() noexcept
//{
////    qDebug()<<"TAG1041: trainType delete: "<<_name<<Qt::endl;
//}
