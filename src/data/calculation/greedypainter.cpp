#include "greedypainter.h"
#include <data/diagram/diagram.h>
#include <data/train/train.h>
#include <data/rail/ruler.h>
#include <data/rail/rulernode.h>
#include <util/utilfunc.h>
#include <data/rail/forbid.h>
#include <exception>


class BackoffExeed : public std::exception
{

};


GreedyPainter::GreedyPainter(Diagram& diagram) :
	diagram(diagram)
{

}

bool GreedyPainter::paint(const TrainName& trainName)
{
	_railAxis = diagram.stationEventAxisForRail(_railway);
	_train = std::make_shared<Train>(trainName);
	_logs.clear();
	backoffCount = 0;

	// 首站的处理
	QTime tm_arr = _anchorTime, tm_dep = _anchorTime;
	if (auto itr = _settledStops.find(_anchor); itr != _settledStops.end()) {
		if (_anchorAsArrive) {
			tm_dep = _anchorTime.addSecs(itr->second);
		}
		else {
			tm_arr = _anchorTime.addSecs(-itr->second);
		}
	}

	// 此变量表示anchor这个站是否有实际的停车 （不管始发终到的事情）
	bool anchor_stop = (tm_dep != tm_arr);

	bool flag = false, flag2 = false;
	
	// 2022.04.10：这里再套一层循环；最多两次。
	// 用于解决正推时anchor不停车但反推要求停车的情况。
	// loop invariant: 要求anchor站永远在时刻表里面。
	for (int _ = 0; _ < 2; _++) {
		_train->clear();
		_train->appendStation(_anchor->name, tm_arr, tm_dep);

		// 正向推线
		auto railint = _anchor->dirNextInterval(_dir);
		try {
			flag = calForward(railint, _anchorTime,
				(_anchor == _start && _localStarting) || anchor_stop);
		}
		catch (const BackoffExeed&) {
			addLog(std::make_unique<CalculationLogSimple>(CalculationLogAbstract::BadTermination));
		}

		anchor_stop = anchor_stop || _train->timetable().front().isStopped();

		// 反向推线
		auto railint_bak = _anchor->dirPrevInterval(_dir);
		try {
			flag2 = calBackward(railint_bak, tm_arr,
				(_anchor == _end && _localTerminal) || anchor_stop);
		}
		catch (const BackoffExeed&) {
			addLog(std::make_unique<CalculationLogSimple>(CalculationLogAbstract::BadTermination));
		}

		// 只有一种情况会触发循环，即正推不停车且反推失败
		if (!flag2 && !anchor_stop) {
			anchor_stop = true;   // 下一轮强行停车
			addLog(std::make_unique<CalculationLogDescription>(
				QObject::tr("[重新进行正向推线] 反向推线要求锚点站停车，以锚点站停车重排正向运行线")));
		}
		else {
			break;
		}
	}

	if (flag && flag2 && !_train->empty()) {
		if (_localStarting) {
			_train->setStarting(_start->name);
			auto& first = _train->timetable().front();
			if (first.name == _start->name) {
				first.arrive = first.depart;
				first.business = true;
			}
		}
		if (_localTerminal) {
			_train->setTerminal(_end->name);
			auto& last = _train->timetable().back();
			if (last.name == _end->name) {
				last.depart = last.arrive;
				last.business = true;
			}
		}
	}

	return flag;
}

void GreedyPainter::addLog(std::unique_ptr<CalculationLogAbstract> log)
{
	qDebug() << log->toString() << Qt::endl;
	//if (log->toString() == "[史家乡->内江区间运行冲突 右冲突] 将[史家乡]站[出发]时刻设置为[20:39:20] (对象: K9406)") {
	//	qDebug() << "史家乡!";
	//}
	_logs.emplace_back(std::move(log));
}

namespace _greedypaint_detail {

	/**
	 * 使用RAII机制记录递归的进入和退出
	 */
	class _RecurseLogger {
		GreedyPainter* painter;
		const RailInterval* railint;
		const QTime& tm;
		bool stop, isBack;
	public:
		_RecurseLogger(GreedyPainter* painter, const RailInterval* railint,
			const QTime& tm, bool stop, bool isBack) :
			painter(painter), railint(railint), tm(tm), stop(stop), isBack(isBack)
		{
			painter->addLog(std::make_unique<CalculationLogSub>(
				CalculationLogAbstract::SubEnter, railint, tm, stop, isBack
				));
		}

		~_RecurseLogger() {
			painter->addLog(std::make_unique<CalculationLogSub>(
				CalculationLogAbstract::SubExit, railint, tm, stop, isBack
				));
		}
	};

}

bool GreedyPainter::calForward(std::shared_ptr<const RailInterval> railint, const QTime& _tm, bool stop)
{
	if (!railint || railint->fromStation() == _end) {
		addLog(std::make_unique<CalculationLogSimple>(
			CalculationLogAbstract::ForwardFinished
			));
		return true;
	}

	_greedypaint_detail::_RecurseLogger _logger(this, railint.get(), _tm, stop, false);

	auto node = railint->getRulerNode(_ruler);
	if (node->isNull()) {
		addLog(std::make_unique<CalculationLogSimple>(CalculationLogAbstract::NoData));
		return true;
	}

	auto st_from = node->railInterval().fromStation();
	auto st_to = node->railInterval().toStation();
	const auto& ax_from = _railAxis.at(st_from);
	const auto& ax_to = _railAxis.at(st_to);

	auto itr = _settledStops.find(st_to);
	bool next_stop = ((itr != _settledStops.end()) || (st_to == _end && _localTerminal));

	// 注意以后的出发时间以这里面的为准！
	RailStationEventBase ev_start(qeutil::latterEventType(stop), _tm, qeutil::dirLatterPos(_dir)
		, _dir);

	if (stop) {
		if (auto itr = _settledStops.find(st_from); itr != _settledStops.end()) {
			ev_start.time = ev_start.time.addSecs(itr->second);
			addLog(std::make_unique<CalculationLogBasic>(
				CalculationLogAbstract::SetStop, st_from, ev_start.time,
				CalculationLogAbstract::Depart
				));
			_train->timetable().back().business = true;
		}
	}

	int tot_delay = qeutil::secsTo(_tm, ev_start.time);

	// 这个Flag标记to站是否要尝试停车。
	// 在第一次通过to站发现停车间隔冲突时，需尝试是否要在后站停车；
	// 此时必须重来一遍循环，设置此flag为true，表示下一轮不再尝试通过。
	bool to_try_stop = false;

	while (true) {
		qDebug() << railint->toString() << "  delay: " << tot_delay << ", " << tot_delay / 3600. << Qt::endl;
		if (tot_delay >= 24 * 3600) {
			// 没有可排的线位
			if (st_from != _anchor)
				_train->timetable().pop_back();
			backoffCount++;
			addLog(std::make_unique<CalculationLogBackoff>(
				st_from, ev_start.time, CalculationLogAbstract::Depart, backoffCount
				));
			if (backoffCount > _maxBackoffTimes) {
				throw BackoffExeed();
			}
			return false;
		}

		// 出发时刻检测
		auto ev_conf = ax_from.conflictEvent(ev_start, _constraints);
		if (ev_conf) {
			// 出发时刻冲突
			if (!stop && st_from != _anchor) {
				_train->timetable().pop_back();
				qDebug() << "回溯 " << st_from->name.toSingleLiteral() << Qt::endl;
				return false;
			}
			else {
				// 调整出发时刻为使得满足条件
				if (qeutil::timeCompare(ev_start.time, ev_conf->time)) {
					// 右冲突事件
					tot_delay += qeutil::secsTo(ev_start.time, ev_conf->time);
					ev_start.time = ev_conf->time;
					addLog(std::make_unique<CalculationLogGap>(CalculationLogAbstract::GapConflict,
						st_from, ev_start.time, CalculationLogAbstract::Depart,
						*TrainGap::gapTypeBetween(ev_start, *ev_conf, _constraints.isSingleLine()),
						st_from, ev_conf
						));
				}
				else {
					// 左冲突事件，将时刻弄到与当前不冲突的地方
					auto type = TrainGap::gapTypeBetween(*ev_conf, ev_start, _constraints.isSingleLine());
					qDebug() << "左冲突：" << ev_conf->toString() << Qt::endl;
					int gap_min = _constraints.at(*type);
					auto trial_tm = ev_conf->time.addSecs(gap_min);
					tot_delay += qeutil::secsTo(ev_start.time, trial_tm);
					ev_start.time = trial_tm;
					addLog(std::make_unique<CalculationLogGap>(
						CalculationLogAbstract::GapConflict, st_from, ev_start.time,
						CalculationLogAbstract::Depart, *type, st_from, ev_conf
						));
				}
				// 到这里只能说解决了当前冲突，并不一定符合出发条件，还要进一步循环！
				to_try_stop = false;   // 出发时刻改变后优先尝试通过
				continue;
			}

		}

		// 检测区间冲突
		int int_secs = node->interval;
		//tot_delay这个判据用来解决anchor站被迫停车时的附加
		if (stop || tot_delay) 
			int_secs += node->start;
		if (next_stop || to_try_stop) {
			// 本轮循环中后站尝试停车，因此带附加时分
			int_secs += node->stop;
		}

		// 区间天窗冲突
		auto tm_to = ev_start.time.addSecs(int_secs);
		bool forbid_conf = false;
		for (auto forbid : _usedForbids) {
			auto fbdnode = railint->getForbidNode(forbid);
			if (!fbdnode->isNull()) {
				if (qeutil::timeRangeIntersectedExcl(fbdnode->beginTime, fbdnode->endTime, 
					ev_start.time, tm_to)) {
					// 发生天窗冲突
					if (!stop && st_from != _anchor) {
						_train->timetable().pop_back();
						return false;
					}
					else {
						tot_delay += qeutil::secsTo(ev_start.time, fbdnode->endTime);
						ev_start.time = fbdnode->endTime;
						addLog(std::make_unique<CalculationLogForbid>(st_from, ev_start.time,
							CalculationLogAbstract::Depart, railint, forbid));
						to_try_stop = false;
						//continue;
						//2022.06.01：这里必须continue外层循环
						forbid_conf = true;
						break;
					}
				}
			}
		}
		if (forbid_conf)
			continue;

		// 区间运行冲突

		auto rep = _railAxis.intervalConflicted(st_from, st_to, _dir, ev_start.time, int_secs, _constraints.isSingleLine(), false);
		if (rep.type != IntervalConflictReport::NoConflict) {
			// 存在冲突
			if (!stop && st_from != _anchor) {
				_train->timetable().pop_back();
				return false;
			}
			else if (rep.type == IntervalConflictReport::LeftConflict) {
				// 左冲突
				auto tm_trial = rep.conflictEvent->time.addSecs(-int_secs);
				tot_delay += qeutil::secsTo(ev_start.time, tm_trial);
				ev_start.time = tm_trial;
			}
			else if (rep.type == IntervalConflictReport::RightConflict) {
				// 右冲突
				tot_delay += qeutil::secsTo(ev_start.time, rep.conflictEvent->time);
				ev_start.time = rep.conflictEvent->time;
			}
			else {
				// 只剩下共线冲突了 没有更好的办法，只有延迟一个小量跳过去
				tot_delay += 1;
				ev_start.time = ev_start.time.addSecs(1);
			}

			addLog(std::make_unique<CalculationLogInterval>(
				CalculationLogAbstract::IntervalConflict,
				st_from, ev_start.time, CalculationLogAbstract::Depart,
				rep.type, railint, rep.conflictEvent ? rep.conflictEvent->line : nullptr
				));
			to_try_stop = false;
			continue;
		}
		// 到现在为止，前站时刻可以暂时确定了
		_train->timetable().back().depart = ev_start.time;

		// 后站 首先检测是否能通过

		tm_to = ev_start.time.addSecs(int_secs);
		RailStationEventBase ev_stop(TrainEventType::SettledPass, tm_to, qeutil::dirFormerPos(_dir), _dir);
		auto to_conf = ax_to.conflictEvent(ev_stop, _constraints);
		if (!to_try_stop && !next_stop && !to_conf) {
			addLog(std::make_unique<CalculationLogBasic>(
				CalculationLogAbstract::Predicted, st_to, tm_to,
				CalculationLogAbstract::Arrive
				));
			_train->appendStation(st_to->name, tm_to, tm_to, false);
			bool flag = calForward(railint->nextInterval(), tm_to, false);
			if (flag) return true;
		}

		// 如果走到这里，说明通过的尝试失败
		// 如果本轮循环是尝试通过的，则下一轮不用再试通过。
		if (!to_try_stop) {
			to_try_stop = true;
			continue;
		}

		// 下面：后站需要停车的情况。注意根据递归基本约定，
		// 此时后站尚未入栈
		//int_secs += node->stop; // 2022.03.19：现在不用再加，因为前面加过了
		tm_to = ev_start.time.addSecs(int_secs);
		ev_stop.time = tm_to;
		ev_stop.type = TrainEventType::Arrive;
		to_conf = ax_to.conflictEvent(ev_stop, _constraints);

		// 只要尝试过一次原时刻停车了，不论结果如何，下一轮都优先考虑通过
		to_try_stop = false;

		if (!to_conf) {
			// 可以停车
			addLog(std::make_unique<CalculationLogBasic>(
				CalculationLogAbstract::Predicted, st_to, tm_to,
				CalculationLogAbstract::Arrive
				));
			_train->appendStation(st_to->name, tm_to, tm_to, false);
			bool flag = calForward(railint->nextInterval(), tm_to, true);
			if (flag) {
				return true;
			}
			else {
				if (st_from != _anchor)
				_train->timetable().pop_back();
				return false;
			}
		}
		else {
			if (!stop && st_from != _anchor) {
				// 回溯
				if (st_from != _anchor)
				_train->timetable().pop_back();
				return false;
			}

			int_secs -= node->stop;
			TrainGapTypePair type;
			if (qeutil::timeCompare(tm_to, to_conf->time)) {
				// 右冲突事件，设置出发时间使得到达时间为冲突时刻的时间
				auto trial_tm = to_conf->time.addSecs(-int_secs);
				tot_delay += qeutil::secsTo(ev_start.time, trial_tm);
				ev_start.time = trial_tm;
				type = *TrainGap::gapTypeBetween(ev_stop, *to_conf, _constraints.isSingleLine());
			}
			else {
				// 左冲突事件，将时刻弄到与当前不冲突的地方
				type = *TrainGap::gapTypeBetween(*to_conf, ev_stop, _constraints.isSingleLine());
				int gap_min = _constraints.at(type);

				QTime trial_arr = to_conf->time.addSecs(gap_min);
				QTime trial_dep = trial_arr.addSecs(-int_secs);
				tot_delay += qeutil::secsTo(ev_start.time, trial_dep);
				ev_start.time = trial_dep;

			}
			addLog(std::make_unique<CalculationLogGap>(
				CalculationLogAbstract::GapConflict, st_from, ev_start.time,
				CalculationLogAbstract::Depart, type, st_to, to_conf
				));
		}

	}
}

bool GreedyPainter::calBackward(std::shared_ptr<const RailInterval> railint, const QTime& _tm, bool stop)
{
	// 反向推线，代码基本从正向复制。
	// 注意现在的from/to按照推线的观点看，与railint的from/to恰好相反。
	// 暂时还有个坑：如果正推时锚点站没停车但反推是要求锚点站停车，如何处理。

	if (!railint || railint->toStation() == _start) {
		addLog(std::make_unique<CalculationLogSimple>(
			CalculationLogAbstract::Finished
			));
		return true;
	}

	_greedypaint_detail::_RecurseLogger _logger(this, railint.get(), _tm, stop, true);

	auto node = railint->getRulerNode(_ruler);
	if (node->isNull()) {
		addLog(std::make_unique<CalculationLogSimple>(CalculationLogAbstract::NoData));
		return true;
	}

	auto st_from = node->railInterval().toStation();
	auto st_to = node->railInterval().fromStation();
	const auto& ax_from = _railAxis.at(st_from);
	const auto& ax_to = _railAxis.at(st_to);

	auto itr = _settledStops.find(st_to);
	bool next_stop = ((itr != _settledStops.end()) || (st_to == _start && _localStarting));

	// 注意以后的出发时间以这里面的为准！
	RailStationEventBase ev_arrive(qeutil::formerEventType(stop), _tm, qeutil::dirFormerPos(_dir)
		, _dir);

	if (stop) {
		if (auto itr = _settledStops.find(st_from); itr != _settledStops.end()) {
			ev_arrive.time = ev_arrive.time.addSecs(-itr->second);
			addLog(std::make_unique<CalculationLogBasic>(
				CalculationLogAbstract::SetStop, st_from, ev_arrive.time,
				CalculationLogAbstract::Arrive
				));
			_train->timetable().back().business = true;
		}
	}

	int tot_delay = qeutil::secsTo(ev_arrive.time, _tm);

	// 这个Flag标记to站是否要尝试停车。
	// 在第一次通过to站发现停车间隔冲突时，需尝试是否要在后站停车；
	// 此时必须重来一遍循环，设置此flag为true，表示下一轮不再尝试通过。
	bool to_try_stop = false;

	while (true) {
		qDebug() << railint->toString() << "  delay: " << tot_delay << ", " << tot_delay / 3600. << Qt::endl;
		if (tot_delay >= 24 * 3600) {
			// 没有可排的线位
			if (st_from != _anchor)
				_train->timetable().pop_front();
			backoffCount++;
			addLog(std::make_unique<CalculationLogBackoff>(
				st_from, ev_arrive.time, CalculationLogAbstract::Depart, backoffCount
				));
			if (backoffCount > _maxBackoffTimes) {
				throw BackoffExeed();
			}
			return false;
		}

		// 到达时刻（反向出发）检测
		auto ev_conf = ax_from.conflictEvent(ev_arrive, _constraints);
		if (ev_conf) {
			// 反向出发时刻冲突
			// 注意：anchor站也回溯
			if (!stop) {
				if (st_from!=_anchor)
					_train->timetable().pop_front();
				qDebug() << "回溯 " << st_from->name.toSingleLiteral() << Qt::endl;
				return false;
			}
			else {
				// 调整出发时刻为使得满足条件
				if (qeutil::timeCompare(ev_conf->time, ev_arrive.time)) {
					// 反向左冲突，原来右冲突
					tot_delay += qeutil::secsTo(ev_conf->time, ev_arrive.time);
					ev_arrive.time = ev_conf->time;
					addLog(std::make_unique<CalculationLogGap>(CalculationLogAbstract::GapConflict,
						st_from, ev_arrive.time, CalculationLogAbstract::Arrive,
						*TrainGap::gapTypeBetween(*ev_conf, ev_arrive, _constraints.isSingleLine()),
						st_from, ev_conf
						));
				}
				else {
					// 反向右冲突事件，将时刻弄到与当前不冲突的地方
					auto type = TrainGap::gapTypeBetween(ev_arrive, *ev_conf, _constraints.isSingleLine());
					int gap_min = _constraints.at(*type);
					auto trial_tm = ev_conf->time.addSecs(-gap_min);
					tot_delay += qeutil::secsTo(trial_tm, ev_arrive.time);
					ev_arrive.time = trial_tm;
					addLog(std::make_unique<CalculationLogGap>(
						CalculationLogAbstract::GapConflict, st_from, ev_arrive.time,
						CalculationLogAbstract::Arrive, *type, st_from, ev_conf
						));
				}
				// 到这里只能说解决了当前冲突，并不一定符合出发条件，还要进一步循环！
				to_try_stop = false;   // 出发时刻改变后优先尝试通过
				continue;
			}

		}

		// 检测区间冲突
		int int_secs = node->interval;
		//tot_delay这个判据用来解决anchor站被迫停车时的附加
		if (stop || tot_delay)
			int_secs += node->stop;
		if (next_stop || to_try_stop) {
			// 本轮循环中后站尝试停车，因此带附加时分
			int_secs += node->start;
		}

		// 区间天窗冲突
		auto tm_dep = ev_arrive.time.addSecs(-int_secs);
		for (auto forbid : _usedForbids) {
			auto fbdnode = railint->getForbidNode(forbid);
			if (!fbdnode->isNull()) {
				if (qeutil::timeRangeIntersectedExcl(fbdnode->beginTime, fbdnode->endTime,
					tm_dep, ev_arrive.time)) {
					// 发生天窗冲突
					if (!stop) {
						if(st_from!=_anchor)
						_train->timetable().pop_front();
						return false;
					}
					else {
						tot_delay += qeutil::secsTo(fbdnode->beginTime, ev_arrive.time);
						ev_arrive.time = fbdnode->beginTime;
						addLog(std::make_unique<CalculationLogForbid>(st_from, ev_arrive.time,
							CalculationLogAbstract::Arrive,  railint, forbid));
						to_try_stop = false;
						continue;
					}
				}
			}
		}

		// 区间运行冲突  注意区间的判定按照正向运行的逻辑传参

		auto rep = _railAxis.intervalConflicted(st_to, st_from, _dir, tm_dep, int_secs, 
			_constraints.isSingleLine(), true);
		if (rep.type != IntervalConflictReport::NoConflict) {
			// 存在冲突
			if (!stop) {
				if(st_from != _anchor)
				_train->timetable().pop_front();
				return false;
			}
			else if (rep.type == IntervalConflictReport::LeftConflict) {
				// 左冲突
				auto tm_trial = rep.conflictEvent->time.addSecs(int_secs);
				tot_delay += qeutil::secsTo(tm_trial, ev_arrive.time);
				ev_arrive.time = tm_trial;
			}
			else if (rep.type == IntervalConflictReport::RightConflict) {
				// 右冲突
				tot_delay += qeutil::secsTo(rep.conflictEvent->time, ev_arrive.time);
				ev_arrive.time = rep.conflictEvent->time;
			}
			else {
				// 只剩下共线冲突了 没有更好的办法，只有延迟一个小量跳过去
				tot_delay -= 1;
				ev_arrive.time = ev_arrive.time.addSecs(-1);
			}

			addLog(std::make_unique<CalculationLogInterval>(
				CalculationLogAbstract::IntervalConflict,
				st_from, ev_arrive.time, CalculationLogAbstract::Arrive,
				rep.type, railint, rep.conflictEvent ? rep.conflictEvent->line : nullptr
				));
			to_try_stop = false;
			continue;
		}
		// 到现在为止，前站时刻可以暂时确定了
		_train->timetable().front().arrive = ev_arrive.time;

		// 后站 首先检测是否能通过

		tm_dep = ev_arrive.time.addSecs(-int_secs);
		RailStationEventBase ev_depart(TrainEventType::SettledPass, tm_dep, qeutil::dirLatterPos(_dir), _dir);
		auto to_conf = ax_to.conflictEvent(ev_depart, _constraints);
		if (!to_try_stop && !next_stop && !to_conf) {
			addLog(std::make_unique<CalculationLogBasic>(
				CalculationLogAbstract::Predicted, st_to, tm_dep,
				CalculationLogAbstract::Depart
				));
			_train->prependStation(st_to->name, tm_dep, tm_dep, false);
			bool flag = calBackward(railint->prevInterval(), tm_dep, false);
			if (flag) return true;
		}

		// 如果走到这里，说明通过的尝试失败
		// 如果本轮循环是尝试通过的，则下一轮不用再试通过。
		if (!to_try_stop) {
			to_try_stop = true;
			continue;
		}

		// 下面：后站需要停车的情况。注意根据递归基本约定，
		// 此时后站尚未入栈
		//int_secs += node->stop; // 2022.03.19：现在不用再加，因为前面加过了
		tm_dep = ev_arrive.time.addSecs(-int_secs);
		ev_depart.time = tm_dep;
		ev_depart.type = TrainEventType::Depart;
		to_conf = ax_to.conflictEvent(ev_depart, _constraints);

		// 只要尝试过一次原时刻停车了，不论结果如何，下一轮都优先考虑通过
		to_try_stop = false;

		if (!to_conf) {
			// 可以停车
			addLog(std::make_unique<CalculationLogBasic>(
				CalculationLogAbstract::Predicted, st_to, tm_dep,
				CalculationLogAbstract::Depart
				));
			_train->prependStation(st_to->name, tm_dep, tm_dep, false);
			bool flag = calBackward(railint->prevInterval(), tm_dep, true);
			if (flag) {
				return true;
			}
			else {
				if(st_from != _anchor)
				_train->timetable().pop_front();
				return false;
			}
		}
		else {
			if (!stop && st_from != _anchor) {
				// 回溯
				if (st_from != _anchor)
				_train->timetable().pop_front();
				return false;
			}

			int_secs -= node->start;
			TrainGapTypePair type;
			if (qeutil::timeCompare(to_conf->time, tm_dep)) {
				// 反向左冲突事件，设置出发时间使得到达时间为冲突时刻的时间
				auto tm_trial = to_conf->time.addSecs(int_secs);
				tot_delay += qeutil::secsTo(tm_trial, ev_arrive.time);
				ev_arrive.time = tm_trial;
				type = *TrainGap::gapTypeBetween(*to_conf, ev_depart, _constraints.isSingleLine());
			}
			else {
				// 反向右冲突事件，将时刻弄到与当前不冲突的地方
				type = *TrainGap::gapTypeBetween(ev_depart, *to_conf, _constraints.isSingleLine());
				int gap_min = _constraints.at(type);
				QTime trial_dep = to_conf->time.addSecs(-gap_min);
				QTime trial_arr = trial_dep.addSecs(int_secs);
				tot_delay += qeutil::secsTo(trial_arr, ev_arrive.time);
				ev_arrive.time = trial_arr;
			}
			addLog(std::make_unique<CalculationLogGap>(
				CalculationLogAbstract::GapConflict, st_from, ev_arrive.time,
				CalculationLogAbstract::Arrive, type, st_to, to_conf
				));
		}

	}
}
