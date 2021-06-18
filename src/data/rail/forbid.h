/*
 * 天窗相关的都放这里
 * 初期和pyETRC行为一致：总是有且仅有两套天窗
 */

#include <QTime>
#include <QJsonObject>
#include "railintervaldata.hpp"

class Forbid;

class ForbidNode:
        public RailIntervalNode<ForbidNode,Forbid>
{
public:
    QTime beginTime, endTime;
    ForbidNode(Forbid& forbid,RailInterval& railint);

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /*
     * 天窗时长，以秒为单位
     * 这是为了方便API。原则上，天窗时长总是为整数分钟。
     */
    int durationSec()const;

    int durationMin()const;
};


class Forbid:
        public RailIntervalData<ForbidNode,Forbid>
{
    friend class Railway;
    bool downShow,upShow;

    Forbid(Railway& railway,bool different,int index);
public:

    inline bool isDownShow()const{return downShow;}
    inline bool isUpShow()const{return upShow;}

    QJsonObject toJson()const;

    void show()const;
};
