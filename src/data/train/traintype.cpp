#include "traintype.h"

#include <QDebug>

TrainType::TrainType(const QString& name, const QPen& pen, bool passenger):
	_name(name), _line_style(pen, LineMarkerStyle::no_marker()), _passenger(passenger)
{
}

TrainType::TrainType(const QString &name, const TrainLineStyle &line_style, bool passenger) :
    _name(name), _line_style(line_style),_passenger(passenger)
{
}

//TrainType::TrainType(const TrainType &other):
//    _name(other._name), _pen(other._pen), _passenger(other._passenger)
//{
//    qDebug()<<"TAG1042: trainType copy: "<<_name<<Qt::endl;
//}

bool TrainType::operator==(const TrainType &other) const
{
    return _name==other._name && _line_style==other._line_style&&_passenger==other._passenger;
}

bool TrainType::operator!=(const TrainType &other) const
{
    return ! operator==(other);
}

void TrainType::swap(TrainType& other)
{
    std::swap(_name, other._name);
    std::swap(_line_style, other._line_style);
    std::swap(_passenger, other._passenger);
}



//TrainType::~TrainType() noexcept
//{
////    qDebug()<<"TAG1041: trainType delete: "<<_name<<Qt::endl;
//}
