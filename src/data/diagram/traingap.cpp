#include "traingap.h"
#include "util/utilfunc.h"
#include "trainline.h"
#include <QObject>
#include <initializer_list>

TrainGap::TrainGap(std::shared_ptr<const RailStationEvent> left_, 
    std::shared_ptr<const RailStationEvent> right_, int period_hours_) :
    left(left_), right(right_), type(NoAppend), num(0), period_hours(period_hours_)
{
    if (left->hasAppend())
        type |= LeftAppend;
    if (right->hasAppend())
        type |= RightAppend;
    if (left->line->dir() == Direction::Down)
        type |= LeftDown;
    if (right->line->dir() == Direction::Down)
        type |= RightDown;
    type |= posToGapPosLeft(left->pos);
    type |= posToGapPosRight(right->pos);
}

RailStationEvent::Positions TrainGap::position() const
{
    return left->pos & right->pos;
}

int TrainGap::secs() const
{
    // 2025.07.21: special: if left and right are the same event, this means they are splitted by a whole period
    if (left == right)
		return period_hours * 3600;
    return qeutil::secsTo(left->time, right->time, period_hours);
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
    return typeToString(type);
}

TrainGap::GapTypesV2 TrainGap::posToGapPosLeft(RailStationEvent::Positions pos)
{
    return (TrainGap::GapTypesV2) (pos << 8);
}

TrainGap::GapTypesV2 TrainGap::posToGapPosRight(RailStationEvent::Positions pos)
{
    return TrainGap::GapTypesV2(pos << 10);
}

RailStationEvent::Positions TrainGap::gapTypeToPosLeft(GapTypesV2 type)
{
    return RailStationEvent::Positions(0b11 & (type >> 8));
}

RailStationEvent::Positions TrainGap::gapTypeToPosRight(GapTypesV2 type)
{
    return RailStationEvent::Positions(0b11 & (type >> 10));
}

typename TrainGap::GapTypesV2 TrainGap::setLeftPos(GapTypesV2 type, RailStationEvent::Positions pos)
{
    type &= ~LeftPositionMask;
    type |= posToGapPosLeft(pos);
    return type;
}

typename TrainGap::GapTypesV2 TrainGap::setRightPos(GapTypesV2 type, RailStationEvent::Positions pos)
{
    type &= ~RightPositionMask;
    type |= posToGapPosRight(pos);
    return type;
}

bool TrainGap::isSamePositionType(GapTypesV2 type)
{
    return gapTypeToPosLeft(type) & gapTypeToPosRight(type);
}

QString TrainGap::typeToString(const GapTypesV2& type)
{
    if (type == Avoid)
        return QObject::tr("待避");
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
        if (isTypeLeftFormer(type))
            text.append(QObject::tr("到"));
        else
            text.append(QObject::tr("发"));
    }
    else {
        text.append(QObject::tr("通"));
    }
    //右车次描述
    if (type & RightAppend) {
        if (isTypeRightFormer(type))
            text.append(QObject::tr("到"));
        else
            text.append(QObject::tr("发"));
    }
    else {
        text.append(QObject::tr("通"));
    }
    return text;
}

QString TrainGap::prefixPosToString(const GapTypesV2& type)
{
    auto pos = (gapTypeToPosLeft(type) & gapTypeToPosRight(type));
    switch (pos) {
    case RailStationEvent::Pre:return QObject::tr("站前");
    case RailStationEvent::Post:return QObject::tr("站后");
    case RailStationEvent::Both:return QObject::tr("前后");
    case RailStationEvent::NoPos:return QObject::tr("对侧");
    default:return "";
    }
}

#if 0
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
#endif

QString TrainGap::posTypeToString(const GapTypesV2& type)
{
    // special cases
    switch (type) {
    case Avoid: return QObject::tr("待避");
    }

    QString text = prefixPosToString(type);
    if (!text.isEmpty())text.append('\n');
    text.append(typeToString(type));
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

bool TrainGap::isTypeLeftFormer(const GapTypesV2& type)
{
    return  ((type & LeftDown) && (type & LeftPre)) ||
        ( !(type & LeftDown) && (type & LeftPost));
}

bool TrainGap::isTypeRightFormer(const GapTypesV2& type)
{
    return  ((type & RightDown) && (type & RightPre)) ||
        (!(type & RightDown) && (type & RightPost));
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

std::optional<TrainGap::GapTypesV2> 
    TrainGap::gapTypeBetween(const RailStationEventBase& left, 
        const RailStationEventBase& right, bool singleLine)
{
    //先排除无关的事件，然后直接套用构造函数那一套就行了
    auto pos = left.pos & right.pos;
    if (!singleLine && left.dir != right.dir) {
        // 双线反向两车次不构成间隔
        return std::nullopt;
    }
    //2023.06.15: for gap type V2, no this condition
    //else if (pos == RailStationEvent::NoPos) {
    //    // 空间不相干事件不构成间隔
    //    return std::nullopt;
    //}

    GapTypesV2 type = NoAppend;
    if (left.hasAppend())
        type |= LeftAppend;
    if (right.hasAppend())
        type |= RightAppend;
    if (left.dir == Direction::Down)
        type |= LeftDown;
    if (right.dir == Direction::Down)
        type |= RightDown;
    type |= posToGapPosLeft(left.pos);
    type |= posToGapPosRight(right.pos);
    return type;
}

typename TrainGap::GapTypesV2 
    TrainGap::generateType(Direction left_dir, Direction right_dir, bool left_append, bool right_append)
{
    // MIND: not modified for V2!!!
    GapTypesV2 res = NoAppend;
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

typename TrainGap::GapTypesV2
TrainGap::genDirGapType(Direction left_dir, Direction right_dir, GapTypesV2 base)
{
    if (left_dir == Direction::Down)
        base |= LeftDown;
    if (right_dir == Direction::Down)
        base |= RightDown;
    return base;
}

std::vector<TrainGap::GapTypesV2> 
    TrainGap::allPossibleGaps(bool singleLine)
{
    std::vector<GapTypesV2> baseTypes;
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

    // 2023.06.15  API V2
    // 产生所有可能的间隔类型时，并不考虑Pre|Post这种，通过类型。
    // 但分析/匹配的时候，需要特别注意。
    std::vector<TrainGap::GapTypesV2> res;
    for (auto a : { TrainGap::LeftPre, TrainGap::LeftPost }) {
        for (auto b : { TrainGap::RightPre,TrainGap::RightPost }) {
            for (auto s : baseTypes) {
                res.emplace_back(a | b | s);
            }
        }
    }
    return res;
}
