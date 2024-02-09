#pragma once
#include <memory>
#include "gapconstraints.h"
#include "railwaystationeventaxis.h"
#include "calculationlog.h"

namespace _greedypaint_detail {
	class _RecurseLogger;
}

class Diagram;
class TrainName;
class Railway;
class Ruler;
class RulerNode;
class Forbid;
class TrainFilterSelectorCore;

/**
 * @brief The GreedyPainter class
 * 贪心算法全自动铺画运行线。
 * 本类包含铺画约束条件（间隔约束条件，必停站，锚点站，起讫点）和铺图核心算法。
 * 本类只管生成铺画区间的运行线，不去管整合的问题。
 */
class GreedyPainter
{
	Diagram& diagram;
	const TrainFilterSelectorCore& filter;
	std::shared_ptr<Railway> _railway;
	std::shared_ptr<Ruler> _ruler;
	std::shared_ptr<const RailStation> _anchor, _start, _end;
	bool _localStarting, _localTerminal, _anchorAsArrive;
	Direction _dir;
	QTime _anchorTime;
	std::map<std::shared_ptr<const RailStation>, int> _settledStops;
	std::set<const RailStation*> _fixedStations;

	friend class _greedypaint_detail::_RecurseLogger;

	/**
	 * @brief _train
	 * 这是铺画数据的目标；这里新建。
	 */
	std::shared_ptr<Train> _train;
	GapConstraints _constraints;
	RailwayStationEventAxis _railAxis;

	std::vector<std::unique_ptr<CalculationLogAbstract>> _logs;
	std::vector<std::shared_ptr<Forbid>> _usedForbids;

	int _maxBackoffTimes;
	int backoffCount = 0;

public:
	GreedyPainter(Diagram& diagram, const TrainFilterSelectorCore& filter);
	auto railway() { return _railway; }
	auto ruler() { return _ruler; }
	auto anchor() { return _anchor; }
	auto start() { return _start; }
	auto end() { return _end; }
	auto dir() { return _dir; }
	bool localStarting() { return _localStarting; }
	bool localTerminal() { return _localTerminal; }
	int maxBackoffTimes() { return _maxBackoffTimes; }
	void setRailway(std::shared_ptr<Railway> railway) { _railway = railway; }
	void setRuler(std::shared_ptr<Ruler> ruler) { _ruler = ruler; }
	void setAnchor(std::shared_ptr<const RailStation> anchor) { _anchor = anchor; }
	void setStart(std::shared_ptr<const RailStation> value) { _start = value; }
	void setEnd(std::shared_ptr<const RailStation> value) { _end = value; }
	void setLocalStarting(bool on) { _localStarting = on; }
	void setLocalTerminal(bool on) { _localTerminal = on; }
	void setDir(Direction d) { _dir = d; }
	void setAnchorTime(const QTime& t) { _anchorTime = t; }
	void setMaxBackoffTimes(int t) { _maxBackoffTimes = t; }
	void setAnchorAsArrive(bool on) { _anchorAsArrive = on; }
	auto& constraints() { return _constraints; }
	const auto& constraints()const { return _constraints; }
	auto& settledStops() { return _settledStops; }
	const auto& settledStops()const { return _settledStops; }
	auto& fixedStations() { return _fixedStations; }
	const auto& fixedStations()const { return _fixedStations; }
	auto train() { return _train; }
	auto& logs() { return _logs; }
	auto& usedForbids() { return _usedForbids; }

	/**
	 * @brief paint  核心接口函数，铺画运行线。
	 * @param trainName  新铺列车的车次，根据这个车次创建新对象。这个车次其实也没多大用
	 * @return 铺画结果。如果铺画失败，返回是否成功。
	 * see also: train()
	 * 暂定在这里算事件表，以防止中途变化了
	 */
	bool paint(const TrainName& trainName);

private:
	void addLog(std::unique_ptr<CalculationLogAbstract> log);

	// 2024.02.09: internal report enum and class, for hint 
	enum class RecurseStatus {
		Ok = 0,
		NoSpace,    // No space for new train line
		RequireStop,    // Require stop at the start station (very frequently used!)
		FixedStation,   // The time needed to be adjusted, but the start station is fixed
	};

	struct RecurseReport {
		RecurseStatus status;
		int delay_seconds_hint;   // used for FixedStation type, where the hint is used for guessing next trial time
		RecurseReport(RecurseStatus s, int delay_sec_hint = 1):
			status(s), delay_seconds_hint(delay_sec_hint)
		{}
	};

	/**
	 * 递归正向推线算法。
	 * 递归基本约定：每次调用时，_train中最后一个站是node的前站。
	 * 向下递归时压入新的站；回溯调整时弹出。
	 * 原始算法中的starting, anchor其实可以通过比较算出，不用递归传递
	 * @param node 排图标尺的当前区间，同时包含区间的所有数据
	 * @param tm node前站的预告到达时刻
	 * @param stop node前站是否已包含停车附加时分，即该站是否停车
	 */
	RecurseReport calForward(std::shared_ptr<const RailInterval> railint, const QTime& tm, bool stop);

	bool calBackward(std::shared_ptr<const RailInterval> railint, const QTime& tm, bool stop);
};

