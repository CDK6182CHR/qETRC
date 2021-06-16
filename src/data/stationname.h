/*
 * QETRC新增类
 * 对站名的封装，主要是为了解决域解析符问题
 */
#ifndef STATIONNAME_H
#define STATIONNAME_H

#include <QString>

class StationName
{
    QString _station, _field;
public:
    explicit StationName(const QString& station="",const QString& field="");

    StationName(const StationName&)=default;
    StationName(StationName&&)=default;
    StationName& operator=(const StationName&)=default;
    StationName& operator=(StationName&&)=default;

    inline const QString& station()const{return _station;}
    inline const QString& field()const{return _field;}
    inline void setStation(const QString& s){_station=s;}
    inline void setField(const QString& s){_field=s;}

    /*
     * 与旧有的Python实现类似，从域解析符::形式解出来
     */
    static StationName fromSingleLiteral(const QString& s);
    QString toSingleLiteral()const;

    /*
     * 把::换成*，显示出来更好看
     */
    QString toDisplayLiteral()const;

    /*
     * 这是基本的实现，仅考虑是否完全一样
     */
    bool operator==(const StationName& name)const;

};

#endif // STATIONNAME_H
