﻿/*
 * 对车次的封装，原Timetable_new.checi3的主要逻辑
 */
#pragma once

#include <QString>
#include <QJsonArray>
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
    inline QString dirName(Direction dir)const{
        switch (dir) {
        case Direction::Down:return down();
        case Direction::Up:return up();
        default:return "";
        }
    }
    inline const QString& dirOrFull(Direction dir)const{
        decltype(auto) t=dirName(dir);
        if(t.isEmpty())
            return _full;
        else return t;
    }

    void show()const;

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
    inline QString& dirNameValid(Direction dir) {
        return dir == Direction::Down ? _down : _up;
    }
};

