/**
 * 天窗相关的都放这里
 * 初期和pyETRC行为一致：总是有且仅有两套天窗
 */
#pragma once
#include <QTime>
#include <QJsonObject>
#include "railintervaldata.hpp"

class Forbid;


class ForbidNode:
        public RailIntervalNode<ForbidNode,Forbid>
{
public:
    QTime beginTime, endTime;
    ForbidNode(Forbid& forbid, RailInterval& railint, const QTime& beginTime_ = QTime(),
        const QTime& endTime_ = QTime());

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /**
     * 注意只复制时间！
     */
    ForbidNode& operator=(const ForbidNode& other);

    /**
     * 天窗时长，以秒为单位
     * 这是为了方便API。原则上，天窗时长总是为整数分钟。
     */
    int durationSec()const;

    int durationMin()const;

    inline bool isNull()const { 
        return beginTime.isNull() || endTime.isNull() || beginTime == endTime; 
    }

    void swap(ForbidNode& other);
};


class Forbid:
        public RailIntervalData<ForbidNode,Forbid>
{
    friend class Railway;
    bool downShow,upShow;

    Forbid(std::weak_ptr<Railway> railway,bool different,int index);
    Forbid(const Forbid&) = delete;
    Forbid(Forbid&&) = delete;

public:
    static constexpr int FORBID_COUNT = 2;
    inline bool isDownShow()const{return downShow;}
    inline bool isUpShow()const{return upShow;}
    inline bool isDirShow(Direction dir)const { return dir == Direction::Down ? downShow : upShow; }

    /**
     * 反转指定方向的是否显示设定
     */
    void toggleDirShow(Direction dir);

    QJsonObject toJson()const;

    void _show()const;

    std::shared_ptr<Railway> clone()const;

    /**
     * @brief name
     * 和一般的`name()`概念不同，是直接根据Index确定的。
     */
    QString name()const;

    void swap(Forbid& other);
};
