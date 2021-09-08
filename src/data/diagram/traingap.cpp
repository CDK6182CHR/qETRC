#include "traingap.h"
#include "util/utilfunc.h"
#include "trainline.h"
#include <QObject>

TrainGap::TrainGap(std::shared_ptr<const RailStationEvent> left_, 
    std::shared_ptr<const RailStationEvent> right_) :
    left(left_), right(right_), type(NoAppend), num(0)
{
    if (left->hasAppend())
        type |= LeftAppend;
    if (right->hasAppend())
        type |= RightAppend;
    if (left->line->dir() == Direction::Down)
        type |= LeftDown;
    if (right->line->dir() == Direction::Down)
        type |= RightDown;
}

RailStationEvent::Positions TrainGap::position() const
{
    return left->pos & right->pos;
}

int TrainGap::secs() const
{
    return qeutil::secsTo(left->time, right->time);
}

Direction TrainGap::leftDir() const
{
    return left->line->dir();
}

Direction TrainGap::rightDir() const
{
    return right->line->dir();
}

QString TrainGap::typeString() const
{
    // 特殊规则
    if (type == Avoid) {
        return QObject::tr("待避");
    }
    QString text;
    // 方向描述
    if (bool(type & LeftDown) != bool(type & RightDown)) {
        text = QObject::tr("对向");
    }
    else if (type & LeftDown) {
        text = QObject::tr("下行");
    }
    else {
        text = QObject::tr("上行");
    }


    if (position() == RailStationEvent::Both) {
        // 两事件都是站前站后贯通，只能是通通
        return text + QObject::tr("通通");
    }

    
    // 左车次描述
    if (type & LeftAppend) {
        if (isLeftFormer())
            text.append(QObject::tr("到"));
        else
            text.append(QObject::tr("发"));
    }
    else {
        text.append(QObject::tr("通"));
    }
    //右车次描述
    if (type & RightAppend) {
        if (isRightFormer())
            text.append(QObject::tr("到"));
        else
            text.append(QObject::tr("发"));
    }
    else {
        text.append(QObject::tr("通"));
    }
    return text;
}

bool TrainGap::isLeftFormer() const
{
    return  (leftDir() == Direction::Down && (left->pos & RailStationEvent::Pre)) ||
        (leftDir() == Direction::Up && (left->pos & RailStationEvent::Post));
}

bool TrainGap::isRightFormer() const
{
    return  (rightDir() == Direction::Down && (right->pos & RailStationEvent::Pre)) ||
        (rightDir() == Direction::Up && (right->pos & RailStationEvent::Post));
}

bool TrainGap::operator<(const TrainGap& gap) const
{
    return secs() < gap.secs();
}

bool TrainGap::ltSecs(const std::shared_ptr<TrainGap>& gap1, 
    const std::shared_ptr<TrainGap>& gap2)
{
    return gap1->secs() < gap2->secs();
}
