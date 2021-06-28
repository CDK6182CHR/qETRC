/**
 * 天窗相关的都放这里
 * 初期和pyETRC行为一致：总是有且仅有两套天窗
 */

#include <QTime>
#include <QJsonObject>
#include "railintervaldata.hpp"

class Forbid;

class QGraphicsRectItem;

class ForbidNode:
        public RailIntervalNode<ForbidNode,Forbid>
{
public:
    QTime beginTime, endTime;
    ForbidNode(Forbid& forbid,RailInterval& railint);

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /**
     * 天窗时长，以秒为单位
     * 这是为了方便API。原则上，天窗时长总是为整数分钟。
     */
    int durationSec()const;

    int durationMin()const;

    inline bool isNull()const { 
        return beginTime.isNull() || endTime.isNull() || beginTime == endTime; 
    }
};


class Forbid:
        public RailIntervalData<ForbidNode,Forbid>
{
    friend class Railway;
    bool downShow,upShow;

    Forbid(Railway& railway,bool different,int index);
    Forbid(const Forbid&) = delete;
    Forbid(Forbid&&) = delete;

    QList<QGraphicsRectItem*> _downItems, _upItems;

public:

    inline bool isDownShow()const{return downShow;}
    inline bool isUpShow()const{return upShow;}
    inline bool isDirShow(Direction dir)const { return dir == Direction::Down ? downShow : upShow; }

    QJsonObject toJson()const;

    void _show()const;

    auto& downItems() { return _downItems; }
    auto& upItems() { return _upItems; }
    auto& dirItems(Direction dir) { return dir == Direction::Down ? _downItems : _upItems; }

    inline void addItem(Direction dir, QGraphicsRectItem* item) {
        dirItems(dir).append(item);
    }
};
