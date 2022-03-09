#include "calculationlog.h"
#include <QObject>

#include <data/rail/railstation.h>
#include <data/train/train.h>
#include <data/diagram/trainline.h>


CalculationLogAbstract::CalculationLogAbstract(Reason reason, std::shared_ptr<RailStation> station, 
    const QTime time, ModifiedField field):
    _reason(reason),_station(station),_time(time),_field(field)
{
}

QString CalculationLogAbstract::toString() const
{
    // [原因] 将[]站[到/开]时刻设置为[] ([对象])
    QString res=QObject::tr("[%1] 将[%2]站[%3]时刻设置为[%4]").arg(reasonString(),
                                                            _station->name.toSingleLiteral(),
                                                            fieldString(),
                                                            _time.toString("hh:mm:ss"));
    if (QString obj=objectString();!obj.isEmpty()){
        res.append(QObject::tr(" (对象: %1)").arg(obj));
    }
    return res;
}

QString CalculationLogAbstract::objectString() const
{
    return "";
}

QString CalculationLogAbstract::fieldString() const
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
    case CalculationLogAbstract::SetStop: return QObject::tr("设定停车站");
    case CalculationLogAbstract::Predicted: return QObject::tr("区间自动推线");
    case CalculationLogAbstract::Terminated: return QObject::tr("排图异常终止");
    default: return QObject::tr("ERROR: Non-base type");
    }
}

CalculationLogGap::CalculationLogGap(Reason reason, std::shared_ptr<RailStation> station, const QTime time,
    ModifiedField field, TrainGapTypePair gapType, std::shared_ptr<RailStation> conflictStation,
    std::shared_ptr<RailStationEvent> event_) :
    CalculationLogAbstract(reason, station, time, field), _gapType(gapType), _conflictStation(conflictStation),
    _event(event_)
{
}

QString CalculationLogGap::reasonString() const
{
    if (_reason == GapConflict) {
        return QObject::tr("%1站%2间隔冲突").arg(_conflictStation->name.toSingleLiteral(),
            TrainGap::typeToString(_gapType.second, _gapType.first));
    }
    else {
        return QObject::tr("ERROR: Non-gap type");
    }
}

QString CalculationLogGap::objectString() const
{
    return _event->line->train()->trainName().full();
}

CalculationLogInterval::CalculationLogInterval(Reason reason, std::shared_ptr<RailStation> station, 
    const QTime time, ModifiedField field, IntervalConflictReport::ConflictType type,
    std::shared_ptr<RailInterval> railint, std::shared_ptr<TrainLine> line):
    CalculationLogAbstract(reason,station,time,field),_type(type),_railint(railint),_line(line)
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
    return _line->train()->trainName().full();
}
