/*
 * 对车次的封装，原Timetable_new.checi3的主要逻辑
 */
#pragma once

#include <QString>
#include <QJsonArray>
#include <QHash>
#include "data/common/direction.h"

class TrainName
{
    QString _full,_down,_up;
public:
    TrainName(const QString& full="");
    TrainName(const QString& full,const QString& down,const QString& up);

    TrainName(const TrainName&)=default;
    TrainName(TrainName&& )=default;
    TrainName& operator=(const TrainName&)=default;
    TrainName& operator=(TrainName&&)=default;

    void fromJson(const QJsonArray& ar);
    QJsonArray toJson()const;

    inline bool operator==(const TrainName& another)const{
        return _full==another._full && _down==another._down
                && _up == another._up;
    }

    /*
     * 返回字头
     * 好像不需要，暂时不实现
     */
    QString header()const;

    /*
     * 仅设置全车次而不做解析
     */
    inline void setFull(const QString& full){_full=full;}
    inline void setUp(const QString& up){_up=up;}
    inline void setDown(const QString& down){_down=down;}
    inline const QString& full()const{return _full;}
    inline const QString& down()const{return _down;}
    inline const QString& up()const{return _up;}
    inline QString dirName(Direction _dir)const{
        switch (_dir) {
        case Direction::Down:return down();
        case Direction::Up:return up();
        default:return "";
        }
    }
    inline QString dirOrFull(Direction _dir)const{
        decltype(auto) t=dirName(_dir);
        if(t.isEmpty())
            return _full;
        else return t;
    }

    void show()const;

    /**
     * 返回全车次或部分车次是否包含指定串
     */
    bool contains(const QString& s)const;

private:
    /*
     * 由全车次（存储在full中）解析上下行车次
     */
    void parseFullName();

    /*
     * 判断是否是下行，依据单双数
     */
    Direction nameDirection(const QString& s)const;

    /*
     * 注意这个版本不考虑Undefined
     */
    inline QString& dirNameValid(Direction _dir) {
        return _dir == Direction::Down ? _down : _up;
    }
};

inline uint qHash(const TrainName& tn, uint seed)
{
    return qHash(tn.full(),seed) ^ qHash(tn.down(),seed) ^
            qHash(tn.up(),seed);
}


