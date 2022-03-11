#include "traingap.h"
#include "util/utilfunc.h"
#include "trainline.h"
#include <QObject>
#include <initializer_list>

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

// 旧版本 依据事件的数据做判断
#if 0
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
#endif

QString TrainGap::typeString() const
{
    return typeToString(type, position());
}

QString TrainGap::typeToString(const GapTypes& type, const RailStationEvent::Positions& pos)
{
    // 特殊规则
    if (type == Avoid) {
        return QObject::tr("待避");
    }
    QString text;
    // 方向描述
    if (bool(type & LeftDown) != bool(type & RightDown)) {
        //text = QObject::tr("对向");
        if (type & LeftDown) {
            text = QObject::tr("下上");
        }
        else {
            text = QObject::tr("上下");
        }
    }
    else if (type & LeftDown) {
        text = QObject::tr("下行");
    }
    else {
        text = QObject::tr("上行");
    }


    if (!(type & LeftAppend) && !(type & RightAppend)) {
        // 两事件都是站前站后贯通，只能是通通
        return text + QObject::tr("通通");
    }


    // 左车次描述
    if (type & LeftAppend) {
        if (isTypeLeftFormer(type, pos))
            text.append(QObject::tr("到"));
        else
            text.append(QObject::tr("发"));
    }
    else {
        text.append(QObject::tr("通"));
    }
    //右车次描述
    if (type & RightAppend) {
        if (isTypeRightFormer(type, pos))
            text.append(QObject::tr("到"));
        else
            text.append(QObject::tr("发"));
    }
    else {
        text.append(QObject::tr("通"));
    }
    return text;
}

QString TrainGap::posTypeToString(const GapTypes& type, const RailStationEvent::Positions& pos)
{
    QString text = RailStationEvent::posToString(pos);
    if (!text.isEmpty())text.append('\n');
    text.append(typeToString(type, pos));
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

bool TrainGap::isTypeLeftFormer(const GapTypes& type, const RailStationEvent::Positions& pos)
{
    return  ((type & LeftDown) && (pos & RailStationEvent::Pre)) ||
        ( !(type & LeftDown) && (pos & RailStationEvent::Post));
}

bool TrainGap::isTypeRightFormer(const GapTypes& type, const RailStationEvent::Positions& pos)
{
    return  (type & RightDown && (pos & RailStationEvent::Pre)) ||
        (!(type & RightDown) && (pos & RailStationEvent::Post));
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

std::optional<std::pair<RailStationEvent::Positions, TrainGap::GapTypes>> 
    TrainGap::gapTypeBetween(const RailStationEventBase& left, 
        const RailStationEventBase& right, bool singleLine)
{
    //先排除无关的事件，然后直接套用构造函数那一套就行了
    auto pos = left.pos & right.pos;
    if (!singleLine && left.dir != right.dir) {
        // 双线反向两车次不构成间隔
        return std::nullopt;
    }
    else if (pos == RailStationEvent::NoPos) {
        // 空间不相干事件不构成间隔
        return std::nullopt;
    }

    GapTypes type = NoAppend;
    if (left.hasAppend())
        type |= LeftAppend;
    if (right.hasAppend())
        type |= RightAppend;
    if (left.dir == Direction::Down)
        type |= LeftDown;
    if (right.dir == Direction::Down)
        type |= RightDown;
    return std::make_pair(pos, type);
}

typename TrainGap::GapTypes 
    TrainGap::generateType(Direction left_dir, Direction right_dir, bool left_append, bool right_append)
{
    GapTypes res = NoAppend;
    if (left_dir == Direction::Down)
        res |= LeftDown;
    if (right_dir == Direction::Down)
        res |= RightDown;
    if (left_append)
        res |= LeftAppend;
    if (right_append) {
        res |= RightAppend;
    }
    return res;
}

typename TrainGap::GapTypes 
TrainGap::genDirGapType(Direction left_dir, Direction right_dir, GapTypes base)
{
    if (left_dir == Direction::Down)
        base |= LeftDown;
    if (right_dir == Direction::Down)
        base |= RightDown;
    return base;
}

std::vector<std::pair<RailStationEvent::Positions, TrainGap::GapTypes>> 
    TrainGap::allPossibleGaps(bool singleLine)
{
    std::vector<GapTypes> baseTypes;
    auto appendTypes = { NoAppend,LeftAppend,RightAppend,BothAppend };
    if (singleLine) {
        for (auto tp : appendTypes) {
            for (auto a : { Direction::Down,Direction::Up }) {
                for (auto b : { Direction::Down,Direction::Up }) {
                    baseTypes.push_back(genDirGapType(a, b, tp));
                }
            }
        }
    }
    else {
        for (auto tp : appendTypes) {
            for (auto a : { Direction::Down,Direction::Up }) {
                baseTypes.push_back(genDirGapType(a, a, tp));
            }
        }
    }

    std::vector<std::pair<RailStationEvent::Positions, TrainGap::GapTypes>> res;
    for (auto t : { RailStationEventBase::Pre,RailStationEventBase::Post }) {
        for (auto s : baseTypes) {
            res.emplace_back(t, s);
        }
    }
    return res;
}
