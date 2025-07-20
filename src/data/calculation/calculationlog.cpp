#include "calculationlog.h"
#include <QObject>

#include <data/rail/railstation.h>
#include <data/train/train.h>
#include <data/diagram/trainline.h>
#include <data/rail/forbid.h>

CalculationLogAbstract::CalculationLogAbstract(Reason reason):
    _reason(reason)
{
}

QString CalculationLogAbstract::objectString() const
{
    return QString();
}


CalculationLogStation::CalculationLogStation(Reason reason, std::shared_ptr<const RailStation> station, 
    const TrainTime time, ModifiedField field):
    CalculationLogAbstract(reason),_station(station),_time(time),_field(field)
{
}

QString CalculationLogStation::toString() const
{
    // [原因] 将[]站[到/开]时刻设置为[] ([对象])
    QString res=QObject::tr("[%1] 将[%2]站[%3]时刻设置为[%4]").arg(reasonString(),
                                                            _station->name.toSingleLiteral(),
                                                            fieldString(),
                                                            _time.toString(TrainTime::HMS));
    if (QString obj=objectString();!obj.isEmpty()){
        res.append(QObject::tr(" (对象: %1)").arg(obj));
    }
    return res;
}

QString CalculationLogStation::fieldString() const
{
    switch (_field) {
    case Arrive: return QObject::tr("到达");
    case Depart: return QObject::tr("出发");
    default: return "ERROR";
    }
}

QString CalculationLogBasic::reasonString() const
{
    switch (_reason)
    {
    case CalculationLogStation::SetStop: return QObject::tr("设定停车站");
    case CalculationLogStation::Predicted: return QObject::tr("区间自动推线");
    case CalculationLogStation::Finished: return QObject::tr("排图成功");
    case CalculationLogStation::NoData: return QObject::tr("标尺无数据");
    default: return QObject::tr("ERROR: Non-base type");
    }
}

CalculationLogGap::CalculationLogGap(Reason reason, std::shared_ptr<const RailStation> station, const TrainTime& time,
    ModifiedField field, TrainGap::GapTypesV2 gapType, std::shared_ptr<const RailStation> conflictStation,
    std::shared_ptr<RailStationEvent> event_) :
    CalculationLogStation(reason, station, time, field), _gapType(gapType), _conflictStation(conflictStation),
    _event(event_)
{
}

QString CalculationLogGap::reasonString() const
{
    if (_reason == GapConflict) {
        return QObject::tr("%1站%2间隔冲突").arg(_conflictStation->name.toSingleLiteral(),
            TrainGap::typeToString(_gapType));
    }
    else {
        return QObject::tr("ERROR: Non-gap type");
    }
}

QString CalculationLogGap::objectString() const
{
    return _event->line->train()->trainName().full();
}

CalculationLogInterval::CalculationLogInterval(Reason reason, std::shared_ptr<const RailStation> station, 
    const TrainTime& time, ModifiedField field, IntervalConflictReport::ConflictType type,
    std::shared_ptr<const RailInterval> railint, std::shared_ptr<const TrainLine> line):
    CalculationLogStation(reason,station,time,field),_type(type),_railint(railint),_line(line)
{
}

QString CalculationLogInterval::reasonString() const
{
    if (_reason == IntervalConflict) {
        return QObject::tr("%1区间运行冲突 %2").arg(_railint->toString(),
            IntervalConflictReport::typeToString(_type));
    }
    else {
        return QObject::tr("ERROR: Non-interval type");
    }
}

QString CalculationLogInterval::objectString() const
{
    if (_line)
        return _line->train()->trainName().full();
    else return "";
}

CalculationLogForbid::CalculationLogForbid(std::shared_ptr<const RailStation> station, const TrainTime& time, 
    ModifiedField field, std::shared_ptr<const RailInterval> railint, std::shared_ptr<Forbid> forbid):
    CalculationLogStation(ForbidConflict,station,time,field),_railint(railint), _forbid(forbid)
{
}

QString CalculationLogForbid::reasonString() const
{
    if (_reason == ForbidConflict) {
        return QObject::tr("与%1区间天窗%2冲突").arg(_railint->toString(), _forbid->name());
    }
    else {
        return "ERROR: Non-forbid type";
    }
}

CalculationLogBackoff::CalculationLogBackoff(std::shared_ptr<const RailStation> station, 
    const TrainTime time, ModifiedField field, int _count):
    CalculationLogStation(Backoff,station,time,field),count(_count)
{
}

QString CalculationLogBackoff::reasonString() const
{
    if (_reason == Backoff)
        return QObject::tr("无可用线位，推线分支终止  计数：%1").arg(count);
    else
        return "ERROR: Non-backof type";
}

QString CalculationLogSimple::toString() const
{
    switch (_reason)
    {
    case CalculationLogAbstract::BadTermination: return QObject::tr("[回溯次数已达上限] 排图异常终止");
        break;
    case CalculationLogAbstract::ForwardFinished: return QObject::tr("[正向排图成功] 进入反向排图");
    case CalculationLogAbstract::Finished: return QObject::tr("[排图成功]");
        break;
    case CalculationLogAbstract::NoData: return QObject::tr("[标尺无数据] 排图结束");
        break;
    default: return "ERROR: Non-simple type";
        break;
    }
}

CalculationLogDescription::CalculationLogDescription(const QString& description):
    CalculationLogAbstract(Description), _descrip(description)
{
}

QString CalculationLogDescription::toString() const
{
    return _descrip;
}

QString CalculationLogSub::reasonString() const
{
    switch (_reason)
    {
    case CalculationLogAbstract::SubEnter:
        if (isBack) {
            return QObject::tr("反向推线递归子问题");
        }
        else {
            return QObject::tr("正向推线递归子问题");
        }
        break;
    case CalculationLogAbstract::SubExit:
        if (isBack) {
            return QObject::tr("反向推线递归返回");
        }
        else {
            return QObject::tr("正向推线递归返回");
        }
    default:
        return "ERROR: non-sub type";
        break;
    }
}

QString CalculationLogSub::toString() const
{
    return QObject::tr("[%1] 区间=(%2) 起始时刻=%3 stop=%4").arg(reasonString(), railint->toString(),
        time.toString(TrainTime::HMS)).arg(stop);
}
