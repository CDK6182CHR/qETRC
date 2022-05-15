#pragma once
#include <QTime>
#include <memory>
#include <data/diagram/traingap.h>
#include "intervalconflictreport.h"

class RailStation;

class CalculationLogAbstract
{
public:
    enum ModifiedField {
        Arrive, Depart
    };  //到开时刻，哪一个被修改

    enum Reason {
        Undefined = 0,
        GapConflict,    //间隔冲突
        IntervalConflict,   // 区间运行线直接冲突
        ForbidConflict,   // 与天窗冲突
        SetStop,   // 预设停车
        Predicted,   // 区间推线的直接结果
        Backoff,  // 排图异常终止，一般是因为排不下去
        BadTermination,
        ForwardFinished,
        Finished,
        NoData,   // 无标尺数据
        Description,
        SubEnter,   // 进入/退出子问题
        SubExit,
    };
protected:
    Reason _reason;
public:
    CalculationLogAbstract(Reason reason);
    auto reason()const { return _reason; }

    virtual QString objectString()const;

    virtual QString toString()const=0;

    virtual ~CalculationLogAbstract()=default;
};


class CalculationLogSimple : public CalculationLogAbstract
{
public:
    using CalculationLogAbstract::CalculationLogAbstract;
    virtual QString toString()const override;
};


/**
 * GreedyPaint过程中关键节点的记录信息
 */
class CalculationLogStation:
    public CalculationLogAbstract
{
protected:
    std::shared_ptr<const RailStation> _station;
    QTime _time;
    ModifiedField _field;

public:
    CalculationLogStation(Reason reason, std::shared_ptr<const RailStation> station,
        const QTime time, ModifiedField field);

    auto station()const{return _station;}
    const auto& time()const{return _time;}
    auto field()const{return _field;}

    virtual QString reasonString()const = 0;

    virtual QString toString()const override;

    QString fieldString()const;
};


class CalculationLogBasic :public CalculationLogStation {
public:
    using CalculationLogStation::CalculationLogStation;

    virtual QString reasonString()const override;
};


class CalculationLogGap: public CalculationLogStation {
    TrainGapTypePair _gapType;
    std::shared_ptr<RailStation> _conflictStation;
    std::shared_ptr<RailStationEvent> _event;
public:
    CalculationLogGap(Reason reason, std::shared_ptr<RailStation> station,
        const QTime time, ModifiedField field, TrainGapTypePair gapType,
        std::shared_ptr<RailStation> conflictStation,
        std::shared_ptr<RailStationEvent> event_);
    virtual QString reasonString()const override;
    virtual QString objectString()const override;
};


class CalculationLogInterval : public CalculationLogStation {
    IntervalConflictReport::ConflictType _type;
    std::shared_ptr<const RailInterval> _railint;
    std::shared_ptr<const TrainLine> _line;
public:
    CalculationLogInterval(Reason reason, std::shared_ptr<const RailStation> station,
        const QTime time, ModifiedField field, IntervalConflictReport::ConflictType type, 
        std::shared_ptr<const RailInterval> railint, std::shared_ptr<const TrainLine> line);
    virtual QString reasonString()const override;
    virtual QString objectString()const override;
};

class Forbid;

class CalculationLogForbid : public CalculationLogStation {
    std::shared_ptr<const RailInterval> _railint;
    std::shared_ptr<Forbid> _forbid;
public:
    CalculationLogForbid(std::shared_ptr<const RailStation> station,
        const QTime& time, ModifiedField field, std::shared_ptr<const RailInterval> railint, std::shared_ptr<Forbid> forbid);

    virtual QString reasonString()const override;
};


class CalculationLogBackoff :public CalculationLogStation
{
    int count;
public:
    CalculationLogBackoff(std::shared_ptr<RailStation> station,
        const QTime time, ModifiedField field, int _count);
    virtual QString reasonString()const override;
};


/**
 * 2022.04.10新增
 * 任意字符串描述。
 */
class CalculationLogDescription : public CalculationLogAbstract
{
    QString _descrip;
public:
    CalculationLogDescription(const QString& description);
    virtual QString toString()const override;
};

class RailInterval;

class CalculationLogSub :public CalculationLogAbstract
{
    const RailInterval* railint;
    QTime time;
    bool stop;
    bool isBack;
public:
    CalculationLogSub(Reason reason, const RailInterval* railint, const QTime& time,
        bool stop,bool isBack):
        CalculationLogAbstract(reason),
        railint(railint),time(time),stop(stop),isBack(isBack){}

    QString reasonString()const;
    virtual QString toString()const override;
};


