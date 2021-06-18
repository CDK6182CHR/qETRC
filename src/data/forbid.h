/*
 * 天窗相关的都放这里
 * 处理成和标尺差不多的形式
 * 框架代码可能会比较像
 */

#ifndef FORBID_H
#define FORBID_H

#include <QString>
#include <QTime>


class RailInterval;
class Forbid;

class ForbidNode{
    Forbid& _forbid;
    RailInterval& _railint;

public:
    QTime startTime,endTime;
    ForbidNode(Forbid& forbid,RailInterval& railint);
};

class Railway;

class Forbid
{
    Railway& _railway;
    int _index;
    bool _downShow,_upShow;

    /*
     * 原则上只允许Railway::addEmptyForbid调用
     */
    Forbid(Railway& railway);
public:

};

#endif // FORBID_H
