#pragma once
#include <QTime>
#include <memory>
#include <data/diagram/traingap.h>
#include "intervalconflictreport.h"

class RailStation;


/**
 * GreedyPaint过程中关键节点的记录信息
 */
class CalculationLogAbstract
{
public:

    enum ModifiedField{
        Arrive,Depart
    };  //到开时刻，哪一个被修改

    enum Reason{
        Undefined=0,
        GapConflict,    //间隔冲突
        IntervalConflict,   // 区间运行线直接冲突
        SetStop,   // 预设停车
        Predicted,   // 区间推线的直接结果
        Terminated,  // 排图异常终止，一般是因为排不下去
    };

protected:
    Reason _reason;
    std::shared_ptr<RailStation> _station;
    QTime _time;
    ModifiedField _field;

public:
    CalculationLogAbstract(Reason reason, std::shared_ptr<RailStation> station,
        const QTime time, ModifiedField field);

    auto reason()const{return _reason;}
    auto station()const{return _station;}
    const auto& time()const{return _time;}
    auto field()const{return _field;}

    QString toString()const;

    virtual QString reasonString()const=0;

    virtual QString objectString()const;

    QString fieldString()const;
};


class CalculationLogBasic :public CalculationLogAbstract {
public:
    using CalculationLogAbstract::CalculationLogAbstract;

    virtual QString reasonString()const override;
};


class CalculationLogGap: public CalculationLogAbstract {
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


class CalculationLogInterval : public CalculationLogAbstract {
    IntervalConflictReport::ConflictType _type;
    std::shared_ptr<RailInterval> _railint;
    std::shared_ptr<TrainLine> _line;
public:
    CalculationLogInterval(Reason reason, std::shared_ptr<RailStation> station,
        const QTime time, ModifiedField field, IntervalConflictReport::ConflictType type, 
        std::shared_ptr<RailInterval> railint, std::shared_ptr<TrainLine> line);
    virtual QString reasonString()const override;
    virtual QString objectString()const override;
};

