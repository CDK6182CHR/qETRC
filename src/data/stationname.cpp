#include "stationname.h"
#include <QtCore>

StationName::StationName(const QString &station, const QString &field):
    _station(station),_field(field)
{

}

StationName StationName::fromSingleLiteral(const QString &s)
{
    auto t=s.split("::");
    if(t.isEmpty())
        return StationName();
    else if(t.length()==1){
        return StationName(t.at(0));
    }else if(t.length()==2){
        return StationName(t.at(0),t.at(1));
    }else{
        qDebug()<<"Warning: invalid station name literal: "<<s<<Qt::endl;
        return StationName(t.at(0),t.at(1));
    }
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
