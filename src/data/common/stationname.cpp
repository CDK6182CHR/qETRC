#include "stationname.h"
#include <QtCore>

const StationName& StationName::nullName=StationName::fromSingleLiteral("");

StationName::StationName(const QString &station, const QString &field):
    _station(station),_field(field)
{

}

StationName::StationName(const QString& singleLiteral):
    _station(),_field()
{
    auto t = singleLiteral.split("::");
    if (t.isEmpty());
    else if (t.length() == 1) {
        _station = singleLiteral;
    }
    else if (t.length() == 2) {
        _station = t.at(0);
        _field = t.at(1);
    }
    else {
        qDebug() << "Warning: invalid station name literal: " << singleLiteral << Qt::endl;
        _station = t.at(0);
        _field = t.at(1);
    }
}

StationName StationName::fromSingleLiteral(const QString &s)
{
    return StationName(s);
}

QString StationName::toSingleLiteral() const
{
    if(_field.isEmpty()){
        return _station;
    }else{
        return _station+"::"+_field;
    }
}

QString StationName::toDisplayLiteral() const
{
    if(_field.isEmpty()){
        return _station;
    }else{
        return _station+"*"+_field;
    }
}

bool StationName::operator==(const StationName &name) const
{
    return _station==name._station && _field==name._field;
}

bool StationName::operator<(const StationName& name) const
{
    if (_station == name._station)
        return _field < name._field;
    return _station < name._station;
}

bool StationName::operator>(const StationName& name) const
{
    if (_station == name._station)
        return _field > name._field;
    return _station > name._station;
}
