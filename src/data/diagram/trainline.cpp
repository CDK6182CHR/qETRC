#include "trainline.h"
#include "data/common/stationname.h"
#include "data/train/train.h"
#include "trainadapter.h"
#include "data/train/traincollection.h"
#include "data/train/train.h"
#include "data/rail/rail.h"
#include "util/utilfunc.h"

#include <QDebug>
#include <cmath>

//I/O部分暂不实现
#if 0
TrainLine::TrainLine(const QJsonObject& obj, Train& train)
{
    fromJson(obj, train);
}

void TrainLine::fromJson(const QJsonObject& obj, Train& train)
{
    StationName startname = StationName::fromSingleLiteral(obj.value("start").toString());
    StationName endname = StationName::fromSingleLiteral(obj.value("end").toString());
    bool started = false;   //强制要求end必须在start之后
    for (auto p = train.timetable().begin(); p != train.nullStation(); ++p) {
        if (!started && p->name == startname) {
            start = p;
            started = true;
        }
        //不知道有没有同一个站终止的极端情况。暂时不用else if
        if (started && p->name == endname) {
            end = p;
        }
    }
    dir = DirFunc::fromIsDown(obj.value("down").toBool());
    startLabel = obj.value("show_start_label").toBool();
    endLabel = obj.value("show_end_label").toBool();
}

QJsonObject TrainLine::toJson() const
{
    return QJsonObject{
        {"show",show},
        {"show_start_label",startLabel},
        {"show_end_label",endLabel},
        {"start",start->name.toSingleLiteral()},
        {"end",end->name.toSingleLiteral()}
    };
}
#endif

TrainLine::TrainLine(TrainAdapter& adapter) :
    _adapter(adapter), _dir(Direction::Undefined), _show(adapter.train()->isShow()),
    _startLabel(true), _endLabel(true)
{
}

void TrainLine::print() const
{
    qDebug() << "TrainLine  labels (" << _startLabel << ", " << _endLabel << "): ";
    qDebug() << train()->trainName().full() << " @ " << _adapter.railway()->name() << Qt::endl;
    for (const auto& p : _stations) {
        qDebug() << *p.trainStation << " -> " << p.railStation.lock()->name << Qt::endl;
    }
}

void TrainLine::setIsShow(bool on)
{
    _show=on;
    train()->checkLinesShow();
}

std::shared_ptr<Train> TrainLine::train()
{
    return _adapter.train();
}

std::shared_ptr<const Train> TrainLine::train() const
{
    return _adapter.train();
}

const StationName &TrainLine::firstStationName() const
{
    return _stations.front().trainStation->name;
}

const StationName &TrainLine::lastStationName() const
{
    return _stations.back().trainStation->name;
}

bool TrainLine::startAtThis() const
{
    return !isNull() && train()->isStartingStation(_stations.front().trainStation->name);
}

bool TrainLine::endAtThis() const
{
    return !isNull() && train()->isTerminalStation(_stations.back().trainStation->name);
}

LineEventList TrainLine::listLineEvents(const TrainCollection& coll) const
{
    LineEventList res;
    res.reserve(static_cast<int>(_stations.size()));
    for (size_t i = 0; i < _stations.size(); i++) {
        res.push_back(StationEventList());
    }

    //车站到开时刻
    listStationEvents(res);

    //与其他列车的互作用
    for (auto t : coll.trains()) {
        if (t == train())
            continue;
        for (auto adp : t->adapters()) {
            if (_adapter.isInSameRailway(*adp)) {
                for (auto line : adp->lines()) {
                    if (line->dir() == dir()) {
                        eventsWithSameDir(res, *line, *t);
                    }
                    else {
                        eventsWithCounter(res, *line, *t);
                    }
                }
            }
        }
    }
    return res;
}

DiagnosisList TrainLine::diagnoseLine(const TrainCollection& coll, bool withIntMeet) const
{
    Q_UNUSED(withIntMeet);
    DiagnosisList res;
    
    diagnoseSelf(res);

    foreach(auto t, coll.trains()) {
        if (t != train()) {
            foreach(auto adp, t->adapters()) {
                if (_adapter.isInSameRailway(*adp)) {
                    foreach(auto line, adp->lines()) {
                        if (line->dir() == dir()) {
                            diagnoWithSameDir(res, *line, *t);
                        }
                        else if (true) {
                            // 2022.09.11: 暂时忽略withIntMeet参数。
                            // 但这可能是个可以优化的点，留着以后看。
                            diagnoWithCounter(res, *line, *t);
                        }
                    }
                }
            }
        }
    }
    return res;
}

int TrainLine::totalSecs() const
{
    if (isNull())
        return 0;
    const QTime& first = startLabel() ? _stations.front().trainStation->arrive :
        _stations.front().trainStation->depart;
    const QTime& last = _stations.back().trainStation->depart;
    int secs = first.secsTo(last);
    if (secs <= 0)
        secs += 24 * 3600;
    return secs;
}

std::pair<int, int> TrainLine::runStaySecs() const
{
    if (isNull())
        return std::make_pair(0, 0);
    else if (_stations.size() == 1) {
        return std::make_pair(0, qeutil::secsTo(_stations.front().trainStation->arrive,
            _stations.front().trainStation->depart));
    }
    int run = 0, stay = 0;
    auto p = _stations.begin();
    if (!isStartingStation(p) && startLabel()) {
        stay += p->trainStation->stopSec();
    }
    auto p0 = p; ++p;
    for (; p != _stations.end(); ++p) {
        //run
        run += qeutil::secsTo(p0->trainStation->depart, p->trainStation->arrive);
        if (!isTerminalStation(p))
            stay += p->trainStation->stopSec();
        p0 = p;
    }
    return std::make_pair(run, stay);
}

double TrainLine::totalMile() const
{
    if (isNull())
        return 0.0;
    return std::abs(_stations.back().railStation.lock()->mile -
        _stations.front().railStation.lock()->mile);
}

void TrainLine::listStationEvents(LineEventList& res) const
{
    int i = 0;
    auto p = _stations.begin();
    //先处理第一站，免得循环里面紧到写判断
    if (startLabel()) {
        if (startAtThis()) {
            //始发
            res[0].emplace(StationEvent(
                TrainEventType::Origination,
                p->trainStation->depart,
                p->railStation,
                std::nullopt
            ));
        }
        else {
            if (p->trainStation->isStopped()) {
                //到达出发
                res[0].emplace(StationEvent(
                    TrainEventType::Arrive,
                    p->trainStation->arrive,
                    p->railStation,
                    std::nullopt
                ));
                res[0].emplace(StationEvent(
                    TrainEventType::Depart,
                    p->trainStation->depart,
                    p->railStation,
                    std::nullopt
                ));
            }
            else {
                res[0].emplace(StationEvent(
                    TrainEventType::SettledPass,
                    p->trainStation->arrive,
                    p->railStation,
                    std::nullopt
                ));
            }
        }
    }
    detectPassStations(res, 0, p);
    auto lastIter = _stations.end(); --lastIter;

    //中间站的判断
    for (++p, ++i; p != lastIter; ++p, ++i) {
        auto ts = p->trainStation;
        auto rs = p->railStation;
        if (ts->isStopped()) {
            //到达出发
            res[i].emplace(StationEvent(
                TrainEventType::Arrive, ts->arrive, rs, std::nullopt
            ));
            res[i].emplace(StationEvent(
                TrainEventType::Depart, ts->depart, rs, std::nullopt
            ));
        }
        else {
            //通过
            res[i].emplace(StationEvent(
                TrainEventType::SettledPass, ts->arrive, rs, std::nullopt
            ));
        }
        detectPassStations(res, i, p);
    }

    //最后一站，判断是不是终到站
    auto rs = p->railStation;
    auto ts = p->trainStation;
    if (endAtThis()) {
        res[i].emplace(StationEvent(
            TrainEventType::Destination, ts->depart, rs, std::nullopt
        ));
    }
    else {
        if (ts->isStopped()) {
            res[i].emplace(StationEvent(
                TrainEventType::Arrive, ts->arrive, rs, std::nullopt
            ));
            res[i].emplace(StationEvent(
                TrainEventType::Depart, ts->depart, rs, std::nullopt
            ));
        }
        else {
            res[i].emplace(StationEvent(
                TrainEventType::SettledPass, ts->depart, rs, std::nullopt
            ));
        }
    }
}

void TrainLine::diagnoseSelf(DiagnosisList& res) const
{
    ConstAdaPtr pr = _stations.begin();
    for (auto p = _stations.begin(); p != _stations.end(); pr = p, ++p) {
        // 上一区间问题  注意这里暂时判定为本站的问题
        if (p != pr) {
            diagnoInterval(res, pr, p);
            int secs = qeutil::secsTo(pr->trainStation->depart, p->trainStation->arrive);
            if (secs > 20 * 3600) {
                res.push_back(DiagnosisIssue(DiagnosisType::StopTooLong2,
                    qeutil::Information, p->railStation.lock(), shared_from_this(),
                    p->trainStation->arrive, p->railStation.lock()->mile,
                    QObject::tr("上一区间运行时长为[%1]，超过20小时，应考虑到开时刻是否填反。")\
                    .arg(qeutil::secsToString(secs))));
            }
            else if (secs > 12 * 3600) {
                res.push_back(DiagnosisIssue(DiagnosisType::StopTooLong1,
                    qeutil::Warning, p->railStation.lock(), shared_from_this(),
                    p->trainStation->arrive, p->railStation.lock()->mile,
                    QObject::tr("上一区间运行时长为[%1]，超过12小时，可能导致事件前后顺序判断出错。")\
                    .arg(qeutil::secsToString(secs))));
            }
        }
        int secs = qeutil::secsTo(p->trainStation->arrive, p->trainStation->depart);
        if (secs > 20 * 3600) {
            res.push_back(DiagnosisIssue(DiagnosisType::StopTooLong2,
                qeutil::Information, p->railStation.lock(), shared_from_this(),
                p->trainStation->depart, p->railStation.lock()->mile,
                QObject::tr("本站停车时长为[%1]，超过20小时，应考虑到开时刻是否填反。")\
                .arg(qeutil::secsToString(secs))));
        }
        else if (secs > 12 * 3600) {
            res.push_back(DiagnosisIssue(DiagnosisType::StopTooLong1,
                qeutil::Warning, p->railStation.lock(), shared_from_this(),
                p->trainStation->depart, p->railStation.lock()->mile,
                QObject::tr("本站停车时长为[%1]，超过12小时，可能导致事件前后顺序判断出错。")\
                .arg(qeutil::secsToString(secs))));
        }
    }
}

void TrainLine::diagnoInterval(DiagnosisList& res, ConstAdaPtr prev, ConstAdaPtr cur) const
{
    auto rprev = prev->railStation.lock(), rcur = cur->railStation.lock();
    auto tprev = prev->trainStation, tcur = cur->trainStation;
    if (rprev->dirAdjacent(dir()) == rcur) {
        // 没有中间站
        diagnoForbid(res, rprev->dirNextInterval(dir()), tprev->depart, tcur->arrive);
    }
    else {
        // 中间站一个个来搞
        int ds = qeutil::secsTo(tprev->depart, tcur->arrive);
        double dy = rcur->y_coeff.value() - rprev->y_coeff.value();
        if (!dy) {
            // 区间里程为0
            res.append(DiagnosisIssue(DiagnosisType::SystemError, qeutil::Error,
                rcur, shared_from_this(), tcur->arrive, rcur->mile,
                QObject::tr("区间[%1-%2]的纵坐标变化为0，"
                    "无法推定中间站时刻。").arg(rprev->name.toSingleLiteral(), rcur->name.toSingleLiteral())));
        }
        else {
            double scale = ds / dy;
            QTime tm_prev = tprev->depart;
            auto p = rprev->dirNextInterval(dir());
            for (;
                p; p = p->nextInterval()) {
                auto ri = p->toStation();
                double dyi = ri->y_coeff.value() - rprev->y_coeff.value();
                int dsi = dyi * scale;
                QTime tm = tprev->depart.addSecs(dsi);

                diagnoForbid(res, p, tm_prev, tm);

                if (p->toStation() == rcur) {
                    break;
                }
                else {
                    tm_prev = tm;
                }
            }
            if (!p) {
                res.append(DiagnosisIssue(DiagnosisType::SystemError, qeutil::Error,
                    rcur, shared_from_this(), 
                    tcur->arrive, rcur->mile, QObject::tr("非预期的区间终止：在匹配站表的"
                        "[%1-%2]区间。可能是运行线行别出现问题。")
                    .arg(rprev->name, rcur->name)));
            }
        }
    }
}

void TrainLine::diagnoForbid(DiagnosisList& res, std::shared_ptr<const RailInterval> railint,
    const QTime& in, const QTime& out) const
{
    const auto& rail = _adapter.railway();
    if (rail->forbids().size() != Forbid::FORBID_COUNT) {
        qDebug() << "TrainLine::diagnoForbid: WARNING: unexpeted forbid count: " <<
            rail->forbids().size() << ", expected " << Forbid::FORBID_COUNT << Qt::endl;
    }
    foreach(auto f, rail->forbids()) {
        auto n = railint->getForbidNode(f);
        if (!n->isNull()) {
            if (qeutil::timeRangeIntersected(n->beginTime, n->endTime,
                in, out)) {
                res.push_back(DiagnosisIssue(DiagnosisType::CollidForbid,
                    qeutil::Error, railint, shared_from_this(),
                    out, railint->toStation()->mile,
                    QObject::tr("与天窗[%1]冲突，"
                        "天窗时间是[%2-%3]，区间运行时间（可能包含推定）是[%4-%5]。")
                    .arg(f->name(), n->beginTime.toString("hh:mm"), n->endTime.toString("hh:mm"),
                        in.toString("hh:mm:ss"), out.toString("hh:mm:ss"))));
            }
        }
    }
}

void TrainLine::detectPassStations(LineEventList& res, int index, ConstAdaPtr itr) const
{
    ConstAdaPtr right = std::next(itr);
    if (right == _stations.end())return;
    auto rs0 = itr->railStation.lock(), rsn = right->railStation.lock();
    auto ts0 = itr->trainStation, tsn = right->trainStation;
    if (rs0->isAdjacentWith(rsn)) {
        //区间没有跨过车站，啥都不用干
        return;
    }
    //以下依赖于纵坐标！
    double y0 = rs0->y_coeff.value(), yn = rsn->y_coeff.value();
    double dy = yn - y0;
    int ds = ts0->depart.secsTo(tsn->arrive);   //区间时间，秒数
    if (ds <= 0)ds += 24 * 3600;
    if (y0 == yn) {
        //这个情况很吊诡，应该不会存在。安全起见，特殊处理
        return;
    }

    for (auto p = rs0->dirNextInterval(dir()); 
        p && p->toStation() != rsn; p = p->nextInterval()) {
        auto rsi = p->toStation();
        double yi = rsi->y_coeff.value();
        double rate = (yi - y0) / dy;
        double dsif = ds * rate;
        int dsi = int(std::round(dsif));
        res[index].emplace(StationEvent(
            TrainEventType::CalculatedPass, ts0->depart.addSecs(dsi),
            rsi, std::nullopt, QObject::tr("推算")
        ));
    }
}

std::list<TrainStation> TrainLine::detectPassStationTimes(ConstAdaPtr itr) const
{
    std::list<TrainStation> res;
    ConstAdaPtr right = std::next(itr); 
    if (right == _stations.end())return res;
    auto rs0 = itr->railStation.lock(), rsn = right->railStation.lock();
    auto ts0 = itr->trainStation, tsn = right->trainStation;
    if (rs0->isAdjacentWith(rsn)) {
        //区间没有跨过车站，啥都不用干
        return res;
    }
    //以下依赖于纵坐标！
    double y0 = rs0->y_coeff.value(), yn = rsn->y_coeff.value();
    double dy = yn - y0;
    int ds = ts0->depart.secsTo(tsn->arrive);   //区间时间，秒数
    if (ds <= 0)ds += 24 * 3600;
    if (y0 == yn) {
        //这个情况很吊诡，应该不会存在。安全起见，特殊处理
        return res;
    }

    for (auto p = rs0->dirNextInterval(dir());
        p && p->toStation() != rsn; p = p->nextInterval()) {
        auto rsi = p->toStation();
        double yi = rsi->y_coeff.value();
        double rate = (yi - y0) / dy;
        double dsif = ds * rate;
        int dsi = int(std::round(dsif));
        const auto& tm = ts0->depart.addSecs(dsi);
        res.emplace_back(rsi->name, tm, tm, false, "", QObject::tr("推定"));
    }
    return res;
}

void TrainLine::eventsWithSameDir(LineEventList& res, const TrainLine& another, 
    const Train& antrain) const
{
    //注意：间隔12小时的站内事件判定可能会有问题！可能要先判定交集
    if (std::max(yMin(), another.yMin()) >= std::min(yMax(), another.yMax())) {
        //提前终止条件，y范围根本不相交，不用搞
        return;
    }

    auto pme = _stations.begin(), phe = another._stations.begin();
    ConstAdaPtr mylast = _stations.begin(), hislast = another._stations.begin();   //上一站的迭代器
    int index = 0;   //自己的站序下标，用来插入结果的

    //int ylast = -2, xlast = 0;    //记载上一站比较结果  -2为起始标志
    //double xcond, ycond;

    //后面的站 类似merge过程
    while (pme != _stations.end() && phe != another._stations.end()) {
        int ycond = yComp(pme, phe);
        //if (ylast==-2) {
        //	//和上一次访问在同一个区间，啥都不用干，继续
        //	ylast = ycond;
        //	sameDirStep(ycond, pme, phe, mylast, hislast, index);
        //	continue;
        //}
        //先判定上一个区间有没有发生什么事情。注意此时两个都必定是直线段
        if (pme != mylast && phe != hislast) {
            auto pint = findIntervalIntersectionSameDir(mylast, pme, hislast, phe);
            if (pint.has_value()) {
                addIntervalEvent(res, index - 1, std::get<2>(pint.value()), std::get<1>(pint.value()),
                    *mylast, *pme, std::cref(antrain), std::get<0>(pint.value()),
                    QObject::tr("区间越行??"));
            }
        }

        auto tme = pme->trainStation, the = phe->trainStation;
        auto rme = pme->railStation.lock(), rhe = phe->railStation.lock();
        //判断站内有没有发生什么事情 这个复杂一点
        //麻烦在：可能出现一趟车站内停车，另一趟车通过但这里没有停点的情况
        if (ycond == 0 && (index!=0||startLabel())) {
            //本站时刻表是重合的，那很简单，跟第一站写的那个一样
            if (tme->stopRangeIntersected(*the)) {
                int xlast = xComp(tme->arrive, the->arrive);
                int xcond = xComp(tme->depart, the->depart);
                if (xcond == xlast) {
                    //到开比较情况一致，只有重合需要说明一下
                    if (xcond == 0) {
                        res[index].emplace(StationEvent(
                            TrainEventType::Coincidence, tme->arrive, pme->railStation,
                            std::cref(antrain), QObject::tr("站内共线")
                        ));
                    }
                }
                else if (notStartOrEnd(another, pme, phe)) {
                    //存在始发终到站的情况，不允许踩
                    if (xcond > xlast) {
                        //规定：第一次碰到0时不处理
                        if (xcond > 0) {
                            //比别人发车晚，被踩
                            res[index].emplace(StationEvent(
                                TrainEventType::Avoid, the->depart, pme->railStation,
                                std::cref(antrain)
                            ));
                        }
                    }
                    else {  // xcond < xlast
                        if (xcond < 0) {
                            //比别人发车早，踩了别人
                            res[index].emplace(StationEvent(
                                TrainEventType::OverTaking, tme->depart, pme->railStation,
                                std::cref(antrain)
                            ));
                        }
                    }
                }
            }
            //ylast = ycond;
        }
        else {
            if ((ycond > 0 && dir() == Direction::Down) ||
                (ycond < 0 && dir() == Direction::Up)) {
                //本次列车领先，推定出在前面那个站有无交叉
                if (pme != _stations.begin()) {
                    int passedTime = getPreviousPassedTime(pme, rhe);
                    if (the->timeInStoppedRange(passedTime)&&
                        !another.isStartingOrTerminal(phe)) {
                        //本次列车在上一站踩了它  改成站内事件
                        //res[index - 1].emplace(IntervalEvent(
                        //	TrainEventType::OverTaking,
                        //	QTime::fromMSecsSinceStartOfDay(passedTime),
                        //	*mylast, *pme, std::cref(antrain), rhe->mile,
                        //	QObject::tr("推定")
                        //));
                        res[index - 1].emplace(StationEvent(
                            TrainEventType::OverTaking,
                            QTime::fromMSecsSinceStartOfDay(passedTime),
                            rhe, std::cref(antrain), QObject::tr("推定")
                        ));
                        //qDebug() << ycond;
                        //qDebug() << "推定越行 " << mylast->trainStation->name << ", " <<
                        //	pme->trainStation->name;
                        //qDebug() << "HIS: " << hislast->trainStation->name << ", " <<
                        //	phe->trainStation->name << ", at " << rhe->name;
                    }
                }
            }
            else if (!isStartingOrTerminal(pme)) {
                //本次列车落后，推定对方在本次列车这个站
                if (phe != another._stations.begin() && (index != 0 || startLabel())) {
                    int passedTime = getPreviousPassedTime(phe, rme);
                    if (tme->timeInStoppedRange(passedTime)) {
                        //本次列车在本站被踩
                        res[index].emplace(StationEvent(
                            TrainEventType::Avoid, QTime::fromMSecsSinceStartOfDay(passedTime),
                            rme, std::cref(antrain), QObject::tr("推定")
                        ));
                    }
                }
                else {
                    //qDebug() << "TrainLine::eventsWithSameDir: WARNING: "
                    //	<< "Invalid begin() of iterator encountered at station: "
                    //	<< stationString(*pme) << Qt::endl;
                }
            }
            //ylast = ycond;
        }
        sameDirStep(ycond, pme, phe, mylast, hislast, index);
    }
}

#define SHOW_INTERVALS do{\
qDebug() << "INTERVAL: [" << mylast->trainStation->name << " - "\
<< pme->trainStation->name<<"] vs. [" << hislast->trainStation->name\
<< " - " << phe->trainStation->name << "] @ " << antrain.trainName().full();\
}while(false)

void TrainLine::eventsWithCounter(LineEventList& res, const TrainLine& another, const Train& antrain) const
{
    //!!注意counter的xcond意义和同向的不同
    if (std::max(yMin(), another.yMin()) >= std::min(yMax(), another.yMax())) {
        //提前终止条件，y范围根本不相交，不用搞
        return;
    }

    //qDebug() << "event with counter: " << antrain.trainName().full() << Qt::endl;

    auto pme = _stations.begin();
    auto phe = another._stations.rbegin();  //反迭代器
    ConstAdaPtr mylast = _stations.begin();
    auto hislast = another._stations.rbegin();   //上一站的迭代器
    int index = 0;   //自己的站序下标，用来插入结果的

    //后面的站 类似merge过程
    while (pme != _stations.end() && phe != another._stations.rend()) {
        int ycond = yComp(pme, phe);
        //if (ylast==-2) {
        //	//和上一次访问在同一个区间，或者第一次。
        //	//并不总是能直接跳过；还是可能会发生一些事情。
        //	//previous: if (ylast==-2 || ycond == ylast) && ycond != 0
        //	ylast = ycond;
        //	sameDirStep(ycond, pme, phe, mylast, hislast, index);
        //	continue;
        //}

        //先判定上一个区间有没有发生什么事情。注意此时两个都必定是直线段
        if (pme != _stations.begin() && phe != another._stations.rbegin()) {
            auto pint = findIntervalIntersectionCounter(mylast, pme, hislast, phe);
            if (pint.has_value()) {
                addIntervalEvent(res, index - 1, std::get<2>(pint.value()), std::get<1>(pint.value()),
                    *mylast, *pme, std::cref(antrain), std::get<0>(pint.value()));
            }
        }

        auto tme = pme->trainStation, the = phe->trainStation;
        auto rme = pme->railStation.lock(), rhe = phe->railStation.lock();
        //判断站内有没有发生什么事情 这个复杂一点
        //麻烦在：可能出现一趟车站内停车，另一趟车通过但这里没有停点的情况
        if (ycond == 0 && (index != 0 || startLabel())) {
            //本站时刻表是重合的，那很简单，跟第一站写的那个一样
            auto tme = pme->trainStation, the = phe->trainStation;
            int xcond = xComp(tme->arrive, the->arrive);

            if (xcond == 0) {
                //同时到站，直接判定会车
                //会车时刻：this到达的时刻
                res[index].emplace(StationEvent(
                    TrainEventType::Meet, tme->arrive, pme->railStation, std::cref(antrain)
                ));
            }
            else if (xcond < 0) {
                //他来的时候我还没走，发生会车
                if (xComp(the->arrive, tme->depart) <= 0) {
                    //会车时刻：他到达的时刻
                    res[index].emplace(StationEvent(
                        TrainEventType::Meet, the->arrive, pme->railStation, std::cref(antrain)
                    ));
                }
            }
            else {  //xcond > 0
                //我到的时候他还没走
                if (xComp(tme->arrive, the->depart) <= 0) {
                    //会车时刻：我到达的时刻
                    res[index].emplace(StationEvent(
                        TrainEventType::Meet, tme->arrive, pme->railStation, std::cref(antrain)
                    ));
                }
            }
        }
        else {
            if ((ycond > 0 && dir() == Direction::Down) ||
                (ycond < 0 && dir() == Direction::Up)) {
                //本次列车领先，推定出在前面那个站有无交叉
                //注意单向站。判断一下对方那个站是不是本次列车方向也经过的
                if (rhe->isDirectionVia(dir())) {
                    if (pme!=_stations.begin()) {
                        int passedTime = getPreviousPassedTime(pme, rhe);
                        if (the->timeInStoppedRange(passedTime)) {
                            //新增ycond!=ylast条件，保证这个区间内前后关系变了
                            //本次列车在上一站踩了它
                            //res[index - 1].emplace(IntervalEvent(
                            //	TrainEventType::Meet,
                            //	QTime::fromMSecsSinceStartOfDay(passedTime),
                            //	*mylast, *pme, std::cref(antrain), rhe->mile,
                            //	QObject::tr("推定")
                            //));
                            res[index - 1].emplace(StationEvent(
                                TrainEventType::Meet,
                                QTime::fromMSecsSinceStartOfDay(passedTime),
                                rhe, std::cref(antrain), QObject::tr("推定")
                            ));
                        }
                    }
                    else {
                        //这是不正常的
                        //qDebug() << "TrainLine::eventsWithCounter: WARNING: "
                        //	<< "Invalid begin() of iterator encountered at station: "
                        //	<< stationString(*pme) << " with " 
                        //	<< another.stationString(*phe) << Qt::endl;
                    }
                }
                else { //对方这个站本次列车不通过 没啥意义
                    //但似乎也不用特殊处理？
                }
            }
            else {
                //本次列车落后，推定对方在本次列车这个站
                if (rme->isDirectionVia(another.dir())&& (index != 0 || startLabel())) {
                    if (phe != another._stations.rbegin()) {
                        int passedTime = getPreviousPassedTime(phe, rme);
                        if (tme->timeInStoppedRange(passedTime)) {
                            //本次列车在本站被踩
                            res[index].emplace(StationEvent(
                                TrainEventType::Meet, QTime::fromMSecsSinceStartOfDay(passedTime),
                                rme, std::cref(antrain), QObject::tr("推定")
                            ));
                        }
                    }
                    else {
                        //qDebug() << "TrainLine::eventsWithCounter: WARNING: "
                        //	<< "Invalid rbegin() of iterator encountered at station: "
                        //	<< another.stationString(*phe) << " with " <<
                        //	stationString(*pme) << Qt::endl;
                    }
                }
                
            }
        }
        sameDirStep(ycond, pme, phe, mylast, hislast, index);
    }
}

void TrainLine::diagnoWithSameDir(DiagnosisList& res, const TrainLine& another, const Train& antrain) const
{
    if (std::max(yMin(), another.yMin()) >= std::min(yMax(), another.yMax())) {
        //提前终止条件，y范围根本不相交，不用搞
        return;
    }

    auto pme = _stations.begin(), phe = another._stations.begin();
    ConstAdaPtr mylast = _stations.begin(), hislast = another._stations.begin();   //上一站的迭代器
    int index = 0;   //自己的站序下标，用来插入结果的

    while (pme != _stations.end() && phe != another._stations.end()) {
        int ycond = yComp(pme, phe);
        //先判定上一个区间有没有发生什么事情。注意此时两个都必定是直线段
        if (pme != mylast && phe != hislast) {
            auto pint = findIntervalIntersectionSameDir(mylast, pme, hislast, phe);
            if (pint.has_value()) {
                //区间越行  
                auto pos = compressSnapInterval(mylast, pme, std::get<0>(*pint));
                res.push_back(DiagnosisIssue(DiagnosisType::IntervalOverTaking,
                    qeutil::Error, pos, shared_from_this(), 
                    std::get<1>(*pint), std::get<0>(*pint),
                    QObject::tr("在里程[%1]，时刻"
                        "[%2]与车次[%3]发生[%4]").arg(std::get<0>(*pint)).arg(
                            std::get<1>(*pint).toString("hh:mm:ss"),
                            antrain.trainName().full(),
                            qeutil::eventTypeString(std::get<2>(*pint)))));
            }
        }

        //auto tme = pme->trainStation, the = phe->trainStation;
        //auto rme = pme->railStation.lock(), rhe = phe->railStation.lock();
        //判断站内有没有发生什么事情 这个复杂一点
        //麻烦在：可能出现一趟车站内停车，另一趟车通过但这里没有停点的情况
        if (ycond == 0 && (index != 0 || startLabel())) {
            // 同站事件（A类） 暂时无需处理
        }
        else {
            // 对方站内事件（C类） 暂时无需处理
        }
        sameDirStep(ycond, pme, phe, mylast, hislast, index);
    }
}

void TrainLine::diagnoWithCounter(DiagnosisList& res, const TrainLine& another, const Train& antrain) const
{
    //!!注意counter的xcond意义和同向的不同
    if (std::max(yMin(), another.yMin()) >= std::min(yMax(), another.yMax())) {
        //提前终止条件，y范围根本不相交，不用搞
        return;
    }


    auto pme = _stations.begin();
    auto phe = another._stations.rbegin();  //反迭代器
    ConstAdaPtr mylast = _stations.begin();
    auto hislast = another._stations.rbegin();   //上一站的迭代器
    int index = 0;   //自己的站序下标，用来插入结果的

    //后面的站 类似merge过程
    while (pme != _stations.end() && phe != another._stations.rend()) {
        int ycond = yComp(pme, phe);

        //先判定上一个区间有没有发生什么事情。注意此时两个都必定是直线段
        if (pme != _stations.begin() && phe != another._stations.rbegin()) {
            auto pint = findIntervalIntersectionCounter(mylast, pme, hislast, phe);
            // 区间会车
            if (pint.has_value()) {
                auto pos = compressSnapInterval(mylast, pme, std::get<0>(*pint));
                if (pos.index() == 1) {
                    // holds interval
                    auto railint = std::get<1>(pos);
                    if (railint->isSingleRail()) {
                        res.push_back(DiagnosisIssue(DiagnosisType::IntervalMeet, qeutil::Warning,
                            pos, shared_from_this(), std::get<1>(*pint), std::get<0>(*pint),
                            QObject::tr("在单线区间[%5] 里程标[%1]，时刻[%2]与"
                                "车次[%3]发生[%4]事件").arg(std::get<0>(*pint))
                            .arg(std::get<1>(*pint).toString("hh:mm:ss"), antrain.trainName().full(),
                                qeutil::eventTypeString(std::get<2>(*pint)),railint->toString())
                        ));
                    }
                }
            }
        }

        //auto tme = pme->trainStation, the = phe->trainStation;
        //auto rme = pme->railStation.lock(), rhe = phe->railStation.lock();
        //判断站内有没有发生什么事情 这个复杂一点
        //麻烦在：可能出现一趟车站内停车，另一趟车通过但这里没有停点的情况
        if (ycond == 0 && (index != 0 || startLabel())) {
            // A类事件 暂时无需处理
        }
        else {
            // C类事件 暂时无需处理
        }
        sameDirStep(ycond, pme, phe, mylast, hislast, index);
    }
}

double TrainLine::yMin()const
{
    if (dir() == Direction::Down) {
        return firstRailStation()->y_coeff.value();
    }
    else {
        return lastRailStation()->y_coeff.value();
    }
}

double TrainLine::yMax() const
{
    if (dir() == Direction::Down) {
        return lastRailStation()->y_coeff.value();
    }
    else {
        return firstRailStation()->y_coeff.value();
    }
}


int TrainLine::xComp(const QTime& tm1, const QTime& tm2) const
{
    int x1 = tm1.msecsSinceStartOfDay(), x2 = tm2.msecsSinceStartOfDay();
    int res = 0;
    if (x1 < x2)
        res = -1;
    else if (x1 > x2)
        res = +1;
    //PBC条件：选择两者之间间隔较小的结果
    if (std::abs(x1 - x2) > 24 * 3600 * 1000 / 2) {
        return -res;
    }
    return res;
}

int TrainLine::getPreviousPassedTime(ConstAdaPtr st, std::shared_ptr<RailStation> target) const
{
    static constexpr int msecsOfADay = 24 * 3600 * 1000;
    auto prev = std::prev(st);
    double y0 = prev->railStation.lock()->y_coeff.value();
    double yn = st->railStation.lock()->y_coeff.value();
    double yi = target->y_coeff.value();

    int x0 = prev->trainStation->depart.msecsSinceStartOfDay();
    int xn = st->trainStation->arrive.msecsSinceStartOfDay();

    if (xn < x0) xn += msecsOfADay;
    return int(std::round((yi - y0) / (yn - y0) * (xn - x0) + x0)) % msecsOfADay;
}

int TrainLine::getPreviousPassedTime(std::deque<AdapterStation>::const_reverse_iterator st,
    std::shared_ptr<RailStation> target) const
{
    static constexpr int msecsOfADay = 24 * 3600 * 1000;
    auto prev = std::prev(st);
    double y0 = prev->railStation.lock()->y_coeff.value();
    double yn = st->railStation.lock()->y_coeff.value();
    double yi = target->y_coeff.value();

    int x0 = prev->trainStation->arrive.msecsSinceStartOfDay();
    int xn = st->trainStation->depart.msecsSinceStartOfDay();

    if (x0 < xn) x0 += msecsOfADay;    //反迭代器，应该保证 x0 >= xn !
    return int(std::round((yi - y0) / (yn - y0) * (xn - x0) + x0)) % msecsOfADay;
}


std::optional<std::tuple<double, QTime, TrainEventType>>
    TrainLine::findIntervalIntersectionSameDir(ConstAdaPtr mylast, ConstAdaPtr mythis, 
        ConstAdaPtr hislast, ConstAdaPtr histhis) const
{
    auto rm1 = mylast->railStation.lock(), rm2 = mythis->railStation.lock();
    auto rh1 = hislast->railStation.lock(), rh2 = histhis->railStation.lock();

    //y值：是直接确定的
    double ym1 = rm1->y_coeff.value(), ym2 = rm2->y_coeff.value();
    double yh1 = rh1->y_coeff.value(), yh2 = rh2->y_coeff.value();

    //x值，按照msecs表示
    double xm1 = mylast->trainStation->depart.msecsSinceStartOfDay(),
        xm2 = mythis->trainStation->arrive.msecsSinceStartOfDay();
    double xh1 = hislast->trainStation->depart.msecsSinceStartOfDay(),
        xh2 = histhis->trainStation->arrive.msecsSinceStartOfDay();

    static constexpr int msecsOfADay = 24 * 3600 * 1000;

    //x的表示：如果后一个时刻小于前一个时刻，加上一天。
    if (xm2 < xm1)xm2 += msecsOfADay;
    if (xh2 < xh1)xh2 += msecsOfADay;

    //剪枝：首先判定x坐标不交叉条件，直接再见
    if (std::max(xm1, xh1) > std::min(xm2, xh2)) {
        return std::nullopt;
    }

    //直接采用直线的一般方程
    // A*x + B*y + C = 0
    double a1 = 0, b1 = 0, c1 = 0, a2 = 0, b2 = 0, c2 = 0;
    //本次列车区间运行线解析式
    if (xm1 != xm2) {
        //用点斜式： k*x - y + (y0-k*x0) = 0
        double k = (ym2 - ym1) / (xm2 - xm1);   //斜率
        a1 = k; b1 = -1; c1 = ym1 - k * xm1;
    }
    else {
        //垂直于x轴，表达式为x=xm1
        a1 = 1; b1 = 0; c1 = xm1;
    }
    
    //对方列车区间运行线解析式
    if (xh1 != xh2) {
        double k = (yh2 - yh1) / (xh2 - xh1);
        a2 = k; b2 = -1; c2 = yh1 - k * xh1;
    }
    else {
        a2 = 1; b2 = 0, c2 = xh1;
    }

    //平行条件  A1*B2 - A2*B1 = 0
    if (a1 * b2 == a2 * b1) {
        //平行条件
        if (b1 * c2 == b2 * c1) {
            //重合条件 -- 标记为区间共线运行，时刻为起点时刻中较为靠后的
            int tm_msec;
            double mile;
            if (xm1 <= xh1) {
                //把当前运行线的区间起点作为标记起点
                tm_msec = xm1;
                mile = rm1->mile;
            }
            else {
                tm_msec = xh1;
                mile = rh1->mile;
            }
            return std::make_tuple(mile, QTime::fromMSecsSinceStartOfDay(tm_msec % msecsOfADay),
                TrainEventType::Coincidence);
        }
        else {
            return std::nullopt;
        }
    }

    double xinter, yinter;   //交点坐标
    //不平行，两条直线总可以找到交点  
    if (a1 == 0) {
        //很特殊的情况，方程1直接没了
        yinter = -c1 / b1;
        xinter = (-c2 - b2 * yinter) / a2;
    }
    else {
        //Gauss消元法：r2 = r1 * A2/A1
        double b2p = b2 - a2 * b1 / a1;   //不可能等于0，否则矩阵的秩就不对头了
        yinter = (-c2 + a2 * c1 / a1) / b2p;
        xinter = (-c1 - b1 * yinter) / a1;
    }

    //2021.07.06 由于12小时问题, x判定可能并不安全，因此还是采用y判定
    double yhmin = yh1, yhmax = yh2;
    if (yhmin > yhmax)std::swap(yhmin, yhmax);
    double ymmin = ym1, ymmax = ym2;
    if (ymmin > ymmax)std::swap(ymmin, ymmax);

    // 判定交点是否在合理范围内
    // 2021.09.27 删除等号——正好相等的情况应处理为站内事件。
    if (ymmin < yinter && yinter < ymmax &&
        yhmin < yinter && yinter < yhmax) {
        //合法交点  在本次列车的运行线上算出里程
        double mile;
        if (b1 == 0)mile = rm1->mile;   //斜率无穷大，没得算
        else {
            mile = (rm2->mile - rm1->mile) * (yinter - ym1) / (ym2 - ym1) + rm1->mile;
        }
        QTime&& tm = QTime::fromMSecsSinceStartOfDay(int(round(xinter)) % msecsOfADay);
        if (b1 == 0 || std::abs((ym2 - ym1) / (xm2 - xm1)) >
            std::abs((yh2 - yh1) / (xh2 - xh1))) {
            //本次列车的斜率绝对值比对方来的大，是越行
            return std::make_tuple(mile, tm, TrainEventType::OverTaking);
        }
        else {
            //本次列车被踩
            return std::make_tuple(mile, tm, TrainEventType::Avoid);
        }
    }
    else 
        return std::nullopt;
}

std::optional<std::tuple<double, QTime, TrainEventType>> 
    TrainLine::findIntervalIntersectionCounter(ConstAdaPtr mylast, ConstAdaPtr mythis, 
        std::deque<AdapterStation>::const_reverse_iterator hislast, 
        std::deque<AdapterStation>::const_reverse_iterator histhis) const
{
    auto rm1 = mylast->railStation.lock(), rm2 = mythis->railStation.lock();
    auto rh1 = hislast->railStation.lock(), rh2 = histhis->railStation.lock();

    //y值：是直接确定的
    double ym1 = rm1->y_coeff.value(), ym2 = rm2->y_coeff.value();
    double yh1 = rh1->y_coeff.value(), yh2 = rh2->y_coeff.value();

    //x值，按照msecs表示
    double xm1 = mylast->trainStation->depart.msecsSinceStartOfDay(),
        xm2 = mythis->trainStation->arrive.msecsSinceStartOfDay();
    //注意：对向车用的是反迭代器！
    double xh1 = hislast->trainStation->arrive.msecsSinceStartOfDay(),
        xh2 = histhis->trainStation->depart.msecsSinceStartOfDay();

    static constexpr int msecsOfADay = 24 * 3600 * 1000;

    //x的表示：如果后一个时刻小于前一个时刻，加上一天。
    if (xm2 < xm1) {
        xm2 += msecsOfADay;
    }
        
    if (xh1 < xh2) {
        xh1 += msecsOfADay;
    }

    //以下有：xh1 >= xh2
        

    //剪枝：首先判定x坐标不交叉条件，直接再见
    //再次注意xh1和xh2的大小关系！
    if (std::max(xm1, xh2) > std::min(xm2, xh1)) {
        return std::nullopt;
    }

    //直接采用直线的一般方程
    // A*x + B*y + C = 0
    double a1 = 0, b1 = 0, c1 = 0, a2 = 0, b2 = 0, c2 = 0;
    //本次列车区间运行线解析式
    if (xm1 != xm2) {
        //用点斜式： k*x - y + (y0-k*x0) = 0
        double k = (ym2 - ym1) / (xm2 - xm1);   //斜率
        a1 = k; b1 = -1; c1 = ym1 - k * xm1;
    }
    else {
        //垂直于x轴，表达式为x=xm1
        a1 = 1; b1 = 0; c1 = xm1;
    }

    //对方列车区间运行线解析式
    if (xh1 != xh2) {
        double k = (yh2 - yh1) / (xh2 - xh1);
        a2 = k; b2 = -1; c2 = yh1 - k * xh1;
    }
    else {
        a2 = 1; b2 = 0, c2 = xh1;
    }

    //qDebug() << "Equations: " << a1 << ", " << b1 << ", " << c1 << "; "
    //	<< a2 << ", " << b2 << ", " << c2 << Qt::endl;

    //平行条件  A1*B2 - A2*B1 = 0  按道理在对向中不会发生
    if (a1 * b2 == a2 * b1) {
        //平行条件
        if (b1 * c2 == b2 * c1) {
            //重合条件 -- 标记为区间共线运行，时刻为起点时刻中较为靠后的
            int tm_msec;
            double mile;
            if (xm1 <= xh1) {
                //把当前运行线的区间起点作为标记起点
                tm_msec = xm1;
                mile = rm1->mile;
            }
            else {
                tm_msec = xh1;
                mile = rh1->mile;
            }
            return std::make_tuple(mile, QTime::fromMSecsSinceStartOfDay(tm_msec % msecsOfADay),
                TrainEventType::Coincidence);
        }
        else {
            return std::nullopt;
        }
    }

    double xinter, yinter;   //交点坐标
    //不平行，两条直线总可以找到交点  
    if (a1 == 0) {
        //很特殊的情况，方程1直接没了
        yinter = -c1 / b1;
        xinter = (-c2 - b2 * yinter) / a2;
    }
    else {
        //Gauss消元法：r2 = r1 * A2/A1
        double b2p = b2 - a2 * b1 / a1;   //不可能等于0，否则矩阵的秩就不对头了
        yinter = (-c2 + a2 * c1 / a1) / b2p;
        xinter = (-c1 - b1 * yinter) / a1;
    }

    double yhmin = yh1, yhmax = yh2;
    if (yhmin > yhmax)std::swap(yhmin, yhmax);
    double ymmin = ym1, ymmax = ym2;
    if (ymmin > ymmax)std::swap(ymmin, ymmax);

    // 判定交点是否在合理范围内。用x判定，因为x的大小关系是明确的
    // 2021.09.27删除等号；正好相等的情况应在站内处理。
    if (yhmin < yinter && yinter < yhmax &&
        ymmin < yinter && yinter < ymmax) {
        //合法交点  在本次列车的运行线上算出里程
        double mile;
        if (b1 == 0)mile = rm1->mile;   //斜率无穷大，没得算
        else {
            mile = (rm2->mile - rm1->mile) * (yinter - ym1) / (ym2 - ym1) + rm1->mile;
        }
        QTime&& tm = QTime::fromMSecsSinceStartOfDay(int(round(xinter)) % msecsOfADay);
        return std::make_tuple(mile, tm, TrainEventType::Meet);
    }
    else
        return std::nullopt;
}

void TrainLine::addIntervalEvent(LineEventList& res, int index, TrainEventType type,
    const QTime& time, const AdapterStation& former, 
    const AdapterStation& latter, 
    std::reference_wrapper<const Train> another, double mile, const QString& note) const
{
    auto rfor = former.railStation.lock(), rlat = latter.railStation.lock();
    if (rfor->dirAdjacent(dir()) != rlat) {
        //非相邻站，收缩区间
        //现在假定缺站情况不严重，线性地往中间搜索
        auto r1 = rfor->dirAdjacent(dir());   //临时变量
        while (r1 && mileBeforeEq(r1, mile)) {
            rfor = r1; r1 = r1->dirAdjacent(dir());
        }
        auto invdir = DirFunc::reverse(dir());
        r1 = rlat->dirAdjacent(invdir);
        while (r1 && mileAfterEq(r1, mile)) {
            rlat = r1; r1 = r1->dirAdjacent(invdir);
        }
        //现在：rfor, rlat是收缩后的区间界限
    }
    decltype(rfor) rin{};   //如果站内事件，用这个
    if (rfor->mile == mile)
        rin = rfor;
    else if (rlat->mile == mile)
        rin = rlat;
    if (rin) {
        //判定为站内事件
        res[index].emplace(StationEvent(type, time, rin, another, note));
    }
    else {
        res[index].emplace(IntervalEvent(type, time, rfor, rlat, another, mile, note));
    }
}

SnapEvent::pos_t 
    TrainLine::compressSnapInterval(ConstAdaPtr former, ConstAdaPtr latter, 
    double mile) const
{
    auto rfor = former->railStation.lock(), rlat = latter->railStation.lock();
    
    if (rfor->dirAdjacent(dir()) != rlat) {
        //非相邻站，收缩区间
        //现在假定缺站情况不严重，线性地往中间搜索
        auto r1 = rfor->dirAdjacent(dir());   //临时变量
        while (r1 && mileBeforeEq(r1, mile)) {
            rfor = r1; r1 = r1->dirAdjacent(dir());
        }
        auto invdir = DirFunc::reverse(dir());
        r1 = rlat->dirAdjacent(invdir);
        while (r1 && mileAfterEq(r1, mile)) {
            rlat = r1; r1 = r1->dirAdjacent(invdir);
        }
        //现在：rfor, rlat是收缩后的区间界限
    }
    decltype(rfor) rin{};   //如果站内事件，用这个
    if (rfor->mile == mile)
        rin = rfor;
    else if (rlat->mile == mile)
        rin = rlat;
    if (rin) {
        //站内
        return rin;
    }
    else {
        //区间事件，返回这个区间
        return rfor->dirNextInterval(dir());
    }
}

double TrainLine::snapEventMile(ConstAdaPtr former, ConstAdaPtr latter, const QTime& time) const
{
    auto rfor = former->railStation.lock(), rlat = latter->railStation.lock();

    //先算出里程
    const QTime& t0 = former->trainStation->depart, tn = latter->trainStation->arrive;
    double mile0 = rfor->mile, milen = rlat->mile;
    int ds = qeutil::secsTo(t0, tn);
    return mile0 +
        static_cast<double>(qeutil::secsTo(t0, time)) / ds * (milen - mile0);
}

bool TrainLine::mileBeforeEq(std::shared_ptr<const RailStation> st, double mile) const
{
    return dir() == Direction::Down ?
        st->mile <= mile :
        st->mile >= mile;
}

bool TrainLine::mileAfterEq(std::shared_ptr<const RailStation> st, double mile) const
{
    return dir() == Direction::Down ?
        st->mile >= mile :
        st->mile <= mile;
}

QString TrainLine::stationString(const AdapterStation& st) const
{
    return st.trainStation->name.toSingleLiteral() + " [" +
        train()->trainName().full() + " @ " + _adapter.railway()->name() + "]";
}

bool TrainLine::notStartOrEnd(const TrainLine& another, ConstAdaPtr pme, ConstAdaPtr phe)const
{
    return !isStartingStation(pme) && !isTerminalStation(pme) &&
        !another.isStartingStation(phe) && !another.isTerminalStation(phe);
}

bool TrainLine::isStartingStation(ConstAdaPtr st) const
{
    return startAtThis() && st == _stations.begin();
}

bool TrainLine::isStartingStation(const AdapterStation* st) const
{
    return startAtThis() && st == &*_stations.begin();
}

bool TrainLine::isTerminalStation(ConstAdaPtr st) const
{
    ConstAdaPtr last = _stations.end(); --last;
    return endAtThis() && st == last;
}

bool TrainLine::isTerminalStation(const AdapterStation* st) const
{
    return endAtThis() && st == &(_stations.back());
}

bool TrainLine::hasStartAppend(ConstAdaPtr st) const
{
    return isStartingStation(st) || st->trainStation->isStopped();
}

bool TrainLine::hasStopAppend(ConstAdaPtr st) const
{
    return isTerminalStation(st) || st->trainStation->isStopped();
}

QString TrainLine::appStringShort(ConstAdaPtr prev, ConstAdaPtr cur) const
{
    QString res;
    if (prev->trainStation->isStopped()) {
        res += QObject::tr("起");
    }
    else if( isStartingStation(prev)) {
        res += QObject::tr("始");
    }
    if (cur->trainStation->isStopped()) {
        res += QObject::tr("停");
    }
    else if (isTerminalStation(cur)) {
        res += QObject::tr("终");
    }
    return res;
}

TrainLine::IntervalAttachType 
    TrainLine::getIntervalAttachType(ConstAdaPtr prev, ConstAdaPtr cur) const
{
    auto res = AttachNone;
    if (prev->trainStation->isStopped() || isStartingStation(prev))
        res = static_cast<IntervalAttachType>(res | AttachStart);
    if (cur->trainStation->isStopped() || isTerminalStation(cur))
        res = static_cast<IntervalAttachType>(res | AttachStop);
    return res;
}

QString TrainLine::attachTypeString(IntervalAttachType type)
{
    QString res;
    if (type & AttachStart)
        res += QObject::tr("起");
    if (type & AttachStop)
        res += QObject::tr("停");
    return res;
}

QString TrainLine::attachTypeStringFull(IntervalAttachType type)
{
    QString res;
    if (type & AttachStart)
        res += QObject::tr("起");
    else res += QObject::tr("通");
    if (type & AttachStop)
        res += QObject::tr("停");
    else res += QObject::tr("通");
    return res;
}

std::shared_ptr<const Railway> TrainLine::railway() const
{
    return _adapter.railway();
}

std::shared_ptr<Railway> TrainLine::railway()
{
    return _adapter.railway();
}

bool TrainLine::autoBusiness()
{
    bool flag = false;
    auto t = train();
    bool passen = t->getIsPassenger();
    for (auto p = _stations.begin(); p != _stations.end(); ++p) {
        bool should_busi = (isStartingOrTerminal(p) || p->trainStation->isStopped())
            && ((passen && p->railStation.lock()->passenger) ||
                (!passen && p->railStation.lock()->freight));
        if (should_busi != p->trainStation->business) {
            p->trainStation->business = should_busi;
            flag = true;
        }
    }
    return flag;
}

double TrainLine::previousBoundIntervalMile(ConstAdaPtr st) const
{
    double res=0.;
    auto prev_ada=std::prev(st);
    auto railst=prev_ada->railStation.lock();
    auto railst_end=st->railStation.lock();

    bool foundEnd=false;

    for (auto interv=railst->dirNextInterval(dir()); interv;
         interv=interv->nextInterval()){
        res += interv->mile();
        if (interv->toStation() == railst_end){
            foundEnd=true;
            break;
        }
    }

    if (!foundEnd){
        qDebug()<<"TrainLine::prevousBoundIntervalMile: Invalid data: "
                  "the inverval loop passes the end. " << railst->name.toSingleLiteral() << Qt::endl;
        res=-1;
    }
    return res;
}

RailStationEvent::Position TrainLine::passStationPos(ConstAdaPtr st) const
{
    if (st == _stations.begin()) {
        //第一站
        return dir() == Direction::Down ? RailStationEvent::Post : RailStationEvent::Pre;
    }
    else if (st == std::prev(_stations.end())) {
        return dir() == Direction::Down ? RailStationEvent::Pre : RailStationEvent::Post;
    }
    else return RailStationEvent::Both;
}

std::deque<AdapterStation>::const_iterator TrainLine::stationFromYCoeff(double y) const
{
    if (dir() == Direction::Down)
        return std::lower_bound(_stations.begin(), _stations.end(), y);
    else {
        return std::upper_bound(_stations.rbegin(), _stations.rend(), y).base();
    }
}

const AdapterStation* TrainLine::stationFromRail(std::shared_ptr<RailStation> rail) const
{
    // 2021.09.22：不能直接lower_bound，要考虑上下行
    auto p = stationFromYCoeff(rail->y_coeff.value());
    if (p!=_stations.end()&& p->railStation.lock() == rail) {
        return &(*p);
    }
    else return nullptr; 
}

const AdapterStation* TrainLine::stationByTrainLinear(Train::ConstStationPtr st) const
{
    for (const auto& p : _stations) {
        if (p.trainStation == st) {
            return &p;
        }
    }
    return nullptr;
}

RailStationEventList
    TrainLine::stationEventFromRail(std::shared_ptr<const RailStation> rail) const
{
    if (isNull())return {};
    auto last = std::prev(_stations.end());
    auto p = stationFromYCoeff(rail->y_coeff.value());   //运行方向区间后站
    if (p == _stations.end())
        return {};
    else if (p->railStation.lock() == rail) {
        // 2021.09.09新增规则：运行线首站到达、末站出发不算进来
        bool localFirst = (p == _stations.begin());
        bool localLast = (p == last);
        // 2022.03.12修改规则：多段运行线交接点的，出发算后段、到达算前段
        // 实际上和普通运行线没区别了
        //if (startLabel() || !localFirst) {
        if (true || !localFirst) {
            auto ts = p->trainStation;
            if (ts->isStopped()) {
                //只要有停车，一律按到达出发处理
                RailStationEventList res;
                if (!localFirst) {
                    res.push_back(std::make_shared< RailStationEvent>(TrainEventType::Arrive, ts->arrive,
                        p->railStation, shared_from_this(),
                        dir() == Direction::Down ? RailStationEvent::Pre : RailStationEvent::Post,
                        ts->note));
                }
                if (!localLast) {
                    res.push_back(std::make_shared<RailStationEvent>(TrainEventType::Depart, ts->depart,
                        p->railStation, shared_from_this(),
                        dir() == Direction::Down ? RailStationEvent::Post : RailStationEvent::Pre,
                        ts->note));
                }
                return res;
            }
            else if (isStartingStation(p)) {
                //始发事件
                return { std::make_shared<RailStationEvent>(TrainEventType::Origination,
                    ts->depart,p->railStation,shared_from_this(),
                    dir() == Direction::Down ? RailStationEvent::Post : RailStationEvent::Pre,ts->note) };
            }
            else if (isTerminalStation(p)) {
                return { std::make_shared<RailStationEvent>(TrainEventType::Destination,
                    ts->arrive,p->railStation,shared_from_this(),
                    dir() == Direction::Down ? RailStationEvent::Pre : RailStationEvent::Post,ts->note) };
            }
            else {
                //通过
                return { std::make_shared<RailStationEvent>(TrainEventType::SettledPass,
                    ts->arrive,p->railStation,shared_from_this(),
                    passStationPos(p),ts->note) };
            }
        }

    }
    else if (p == _stations.begin())
        return {};
    else {
        //需要推定通过站时刻
        auto p0 = std::prev(p);
        double y0 = p0->yCoeff(), yn = p->yCoeff(), yi = rail->y_coeff.value();
        double dsif = (qeutil::secsTo(p0->trainStation->depart,
            p->trainStation->arrive)) * (yi - y0) / (yn - y0);
        if (!std::isnan(dsif) && !std::isinf(dsif)) {
            int dsi = int(std::round(dsif));
            return { std::make_shared<RailStationEvent>(TrainEventType::CalculatedPass,
                p0->trainStation->depart.addSecs(dsi), rail, shared_from_this(),
                RailStationEvent::Both, QObject::tr("推算")) };
        }
    }
    return {};
}

std::optional<QTime> TrainLine::sectionTime(double y) const
{
    auto p = stationFromYCoeff(y);
    if (p == _stations.end())
        return std::nullopt;
    else if (p == _stations.begin()) {
        //首站的特殊处理
        if (p->railStation.lock()->y_coeff.value() == y)
            return dir() == Direction::Down ?
            p->trainStation->arrive :
            p->trainStation->depart;
    }
    else {
        //正常的区间情况 根据y值计算  此时p是运行方向区间后站
        auto q = std::prev(p);
        double y0 = q->railStation.lock()->y_coeff.value();
        double yn = p->railStation.lock()->y_coeff.value();
        int dsn = q->trainStation->depart.secsTo(p->trainStation->arrive);
        int dsi = std::round((y - y0) / (yn - y0) * dsn);
        return q->trainStation->depart.addSecs(dsi);
    }
    return std::nullopt;
}

SnapEventList TrainLine::getSnapEvents(const QTime& time) const
{
    SnapEventList res;
    auto pr = _stations.begin();  //上一站迭代器
    for (auto p = _stations.begin(); p != _stations.end(); ++p) {
        //先判断前一区间  注意区间不包括端点
        if (pr != p) {
            // pr.depart < tm < p.arrive
            if (qeutil::timeCompare(pr->trainStation->depart, time) &&
                qeutil::timeCompare(time, p->trainStation->arrive)) {
                //注意这俩不一定是相邻的...
                double mile = snapEventMile(pr, p, time);
                auto pos = compressSnapInterval(pr, p, mile);
                res.append(SnapEvent(shared_from_this(), mile, pos, false,
                    std::holds_alternative<std::shared_ptr<const RailStation>>(pos) ?
                    QObject::tr("推算") : ""));
            }
        }
        if (p->trainStation->timeInStoppedRange(time.msecsSinceStartOfDay())) {
            //恰好在站内
            res.append(SnapEvent(shared_from_this(), p->railStation.lock()->mile,
                p->railStation.lock(), p->trainStation->isStopped()));
        }
        pr = p;
    }
    return res;
}

static int round_secs(int secs, int prec)
{
    int rem = secs % prec;
    if (rem >= prec / 2)
        return secs - rem + prec;
    else return secs - rem;
}

void TrainLine::timetaleInterpolation(std::shared_ptr<const Ruler> ruler, bool toBegin,
    bool toEnd, int precision)
{
    if (isNull())return;

    // todo 时刻修约还没写

    // 先进行内插操作
    // 先写一个直接实现的版本，不考虑抽象
    auto pr = _stations.begin(), p = std::next(_stations.begin());
    for (; p != _stations.end(); pr = p, ++p) {
        auto rpr = pr->railStation.lock(), rp = p->railStation.lock();
        if (rpr->dirAdjacent(dir()) != rp) {
            // 现在需要进行插值操作
            int secs = qeutil::secsTo(pr->trainStation->depart, p->trainStation->arrive);
            int std_pass = ruler->totalInterval(rpr, rp, dir());
            if (std_pass <= 0) {
                // 标尺缺数据，此区间不推定
                continue;
            }
            // 现在区间标尺一定有效
            QTime startTime = pr->trainStation->depart;
            int std_start = rpr->dirNextInterval(dir())->getRulerNode(*ruler)->start;
            int std_stop = rp->dirPrevInterval(dir())->getRulerNode(*ruler)->stop;
            int real_pass = secs;
            if (isStartingStation(pr) || pr->trainStation->isStopped()) {
                real_pass -= std_start;
                startTime = startTime.addSecs(std_start);
            }
            if (isTerminalStation(p) || p->trainStation->isStopped())
                real_pass -= std_stop;
            if (real_pass <= 0) {
                // 数据异常，此区间不推定
                continue;
            }
            double rate = double(real_pass) / std_pass;
            int accum_std_secs = 0;    // 当前站距离推定起始站的累加标尺通通时分

            // 下面可以安全地进行划分了
            // 现在的一切操作，必须注意迭代器p的有效性！！
            auto itr = pr;   // 此迭代器永远有效，并且总是在它之后插入
            decltype (rpr) to_deter;   // 下一个要计算的站
            while ((to_deter = itr->railStation.lock()->dirAdjacent(dir())) != rp) {
                accum_std_secs += to_deter->dirPrevInterval(dir())
                    ->getRulerNode(*ruler)->interval;
                QTime tm = startTime.addSecs(int(round_secs(accum_std_secs * rate,precision)));
                // 插入时刻表车站
                auto train_itr = train()->timetable().emplace(std::next(itr->trainStation),
                    to_deter->name, tm, tm, false, "", QObject::tr("推定"));
                auto line_itr = _stations.emplace(std::next(itr), train_itr, to_deter);
                itr = line_itr;
            }
            p = std::next(itr);
        }
    }
    if (toBegin && !isStartingStation(_stations.begin())) {
        // 向前做外插操作，注意需要截止于（可能出现的）始发站
        auto to_deter=_stations.begin()
            ->railStation.lock()->dirPrevAdjacent(dir());
        QTime refTime = _stations.begin()->trainStation->arrive;
        int acc_std_secs = 0;   // 保存正数，方便修约
        while (to_deter) {
            auto node = to_deter->dirNextInterval(dir())->getRulerNode(*ruler);
            if (node->isNull())
                break;
            acc_std_secs += node->interval;
            if (hasStopAppend(_stations.begin()))
                acc_std_secs += node->stop;
            bool is_starting = train()->isStartingStation(to_deter->name);
            if (is_starting)
                acc_std_secs += node->start;
            QTime tm = refTime.addSecs(-round_secs(acc_std_secs, precision));
            auto train_itr=train()->timetable().emplace(_stations.begin()->trainStation,
                to_deter->name, tm, tm, is_starting, "", QObject::tr("推定"));
            _stations.emplace_front(train_itr, to_deter);
            if (is_starting)break;
            else to_deter = to_deter->dirPrevAdjacent(dir());
        }
    }
    if (auto last = std::prev(_stations.end());toEnd && !isTerminalStation(last)) {
        // 向后做外插
        // last迭代器要始终保持有效
        auto to_deter = last->railStation.lock()->dirAdjacent(dir());
        QTime refTime = last->trainStation->depart;
        int acc_std_secs = 0;
        while (to_deter) {
            auto node = to_deter->dirPrevInterval(dir())->getRulerNode(*ruler);
            if (node->isNull())break;
            acc_std_secs += node->interval;
            if (hasStartAppend(last))
                acc_std_secs += node->start;
            bool is_terminal = train()->isTerminalStation(to_deter->name);
            if (is_terminal) {
                acc_std_secs += node->stop;
            }
            QTime tm = refTime.addSecs(round_secs(acc_std_secs, precision));
            auto train_itr = train()->timetable().emplace(
                std::next(last->trainStation), to_deter->name,
                tm, tm, is_terminal, "", QObject::tr("推定"));
            _stations.emplace_back(train_itr, to_deter);
            last = std::prev(_stations.end());
            if (is_terminal)break;
            else to_deter = to_deter->dirAdjacent(dir());
        }
    }
}

int TrainLine::timetableInterpolationSimple()
{
    int count;
    for (auto itr = _stations.begin(); itr != _stations.end(); ++itr) {
        auto stations = detectPassStationTimes(itr);
        if (stations.empty())continue;
        // 现在std::next(itr)肯定存在
        auto itr_next = std::next(itr);
        auto tcur = itr->trainStation, tnext = itr_next->trainStation;
        // now, do a manual two-way merge
        auto p0 = std::next(tcur);
        while (p0 != tnext && !stations.empty()) {
            if (qeutil::timeCompare(stations.front().depart, p0->arrive)) {
                train()->timetable().splice(p0, stations, stations.begin(), std::next(stations.begin()));
            }
        }
        if (!stations.empty()) {
            train()->timetable().splice(tnext, std::move(stations));
        }
        count += stations.size();
    }
    return count;
}


bool AdapterStation::operator<(double y) const
{
    return railStation.lock()->y_coeff.value() < y;
}

double AdapterStation::yCoeff() const
{
    return railStation.lock()->y_coeff.value();
}

bool operator<(double y, const AdapterStation& adp)
{
    return y < adp.railStation.lock()->y_coeff.value();
}
