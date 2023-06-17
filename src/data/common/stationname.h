
#pragma once


#include <QString>
#include <QHash>
#include <QDebug>
#include <stdint.h>

/**
 * QETRC新增类
 * 对站名的封装，主要是为了解决域解析符问题
 */
class StationName
{
    QString _station, _field;

    /**
     * 2021.08.15：这个双参数的构造函数似乎没用过
     */
    explicit StationName(const QString& station, const QString& field);
public:
    StationName() = default;

    /**
     * 2021.08.15
     * 原有的fromSingleLiteral，重写为构造函数，方便进行隐式转换
     */
    StationName(const QString& singleLiteral);
    static const StationName& nullName;

    StationName(const StationName&)=default;
    StationName(StationName&&)=default;
    StationName& operator=(const StationName&)=default;
    StationName& operator=(StationName&&)=default;

    inline const QString& station()const{return _station;}
    inline const QString& field()const{return _field;}
    inline void setStation(const QString& s){_station=s;}
    inline void setField(const QString& s){_field=s;}

    /**
     * 与旧有的Python实现类似，从域解析符::形式解出来
     */
    static StationName fromSingleLiteral(const QString& s);
    QString toSingleLiteral()const;

    /**
     * 把::换成*，显示出来更好看
     */
    QString toDisplayLiteral()const;

    /**
     * 这是基本的实现，仅考虑是否完全一样
     */
    bool operator==(const StationName& name)const;

    inline bool operator!=(const StationName& name)const {
        return !operator==(name);
    }

    bool operator<(const StationName& name)const;

    bool operator>(const StationName& name)const;

    /**
     * 是否为仅有站名没有场名的类型
     */
    inline bool isBare()const{
        return _field.isEmpty();
    }

    inline bool empty()const {
        return _station.isEmpty() && _field.isEmpty();
    }

    inline operator bool()const {
        return !empty();
    }

    inline bool equalOrContains(const StationName& another)const{
        return (station()==another.station()) &&
                (field()==another.field() || isBare());
    }

    inline bool equalOrBelongsTo(const StationName& another)const{
        return (station()==another.station()) &&
                (field()==another.field() || another.isBare());
    }

    inline bool isSingleName()const { return _field.isEmpty(); }

    /**
     * 相等，或者其中有一个有场名，另一个没有
     */
    inline bool generalEqual(const StationName& another)const {
        return (station() == another.station()) &&
            (field() == another.field() || another.isBare() || isBare());
    }

};

#if QT_VERSION_MAJOR >= 6
inline size_t qHash(const StationName& sn, size_t seed)
{
    return qHash(sn.station(),seed) ^ qHash(sn.field(),seed);
}
#else 
inline uint qHash(const StationName& sn, uint seed)
{
    return qHash(sn.station(), seed) ^ qHash(sn.field(), seed);
}
#endif

inline QDebug operator<<(QDebug debug, const StationName& s){
    QDebugStateSaver saver(debug);
    debug.nospace() << s.toSingleLiteral();
    return debug;
}


