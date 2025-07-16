#include "stationeventaxis.h"
#include "gapconstraints.h"
#include <util/utilfunc.h>
#include <QDebug>

void StationEventAxis::sortEvents()
{
	std::sort(begin(), end(), RailStationEvent::PtrTimeComparator());
}

void StationEventAxis::constructLineMap()
{
	_preEvents.clear();
	_postEvents.clear();
	for (auto ev : *this) {
		if (ev->pos & RailStationEventBase::Pre) {
			_preEvents.emplace(ev->line, ev);
		}
		if (ev->pos & RailStationEventBase::Post) {
			_postEvents.emplace(ev->line, ev);
		}
	}
}

const typename StationEventAxis::line_map_t&
StationEventAxis::positionEvents(RailStationEventBase::Position pos) const
{
	switch (pos)
	{
	case RailStationEventBase::Pre: return preEvents();
		break;
	case RailStationEventBase::Post:return postEvents();
		break;
	default:
		qDebug() << "StationEventAxis::positionEvents: INVALID pos " << pos << Qt::endl;
		return preEvents();
		break;
	}
}

void StationEventAxis::buildAxis()
{
	sortEvents();
	constructLineMap();
}

void StationEventAxis::insertEvent(std::shared_ptr<RailStationEvent> ev)
{
	//如果有时刻一样的，新的在后面
	auto itr = std::upper_bound(begin(), end(), ev, RailStationEvent::PtrTimeComparator());
	insert(itr, ev);
	if (ev->pos & RailStationEventBase::Pre) {
		_preEvents.emplace(ev->line, ev);
	}
	if (ev->pos & RailStationEventBase::Post) {
		_postEvents.emplace(ev->line, ev);
	}
}

std::shared_ptr<RailStationEvent> StationEventAxis::conflictEvent(
	const RailStationEventBase& ev,
	const GapConstraints& constraint, bool singleLine, int period_hours) const
{
	int bound = constraint.correlationRange();
	TrainTime leftBound = ev.time.addSecs(-bound, period_hours), rightBound = ev.time.addSecs(bound, period_hours);
	// upper_bound 正好与reverse_iterator配合使用
	auto citr = std::upper_bound(begin(), end(), ev.time,
		RailStationEvent::PtrTimeComparator());

	// 左侧
	if (leftBound > ev.time) {
		// 发生左跨日情况
		for (auto ritr = std::make_reverse_iterator(citr); ritr != rend(); ++ritr) {
			if (isConflict(**ritr, ev, constraint, singleLine))
				return *ritr;
		}
		for (auto ritr = rbegin(); ritr != rend(); ++ritr) {
			if ((*ritr)->time < leftBound)
				break;
			if (isConflict(**ritr, ev, constraint, singleLine))
				return *ritr;
		}
	}
	else {
		// 不发生左跨日
		for (auto ritr = std::make_reverse_iterator(citr);
			ritr != rend() && (*ritr)->time >= leftBound; ++ritr) {
			if (isConflict(**ritr, ev, constraint, singleLine))
				return *ritr;
		}
	}

	// 右侧
	if (rightBound < ev.time) {
		// 右跨日
		for (auto itr = citr; itr != end(); ++itr) {
			if (isConflict(ev, **itr, constraint, singleLine))
				return *itr;
		}
		for (auto itr = begin(); itr != end() && (*itr)->time <= rightBound; ++itr) {
			if (isConflict(ev, **itr, constraint, singleLine))
				return *itr;
		}
	}
	else {
		for (auto itr = citr; itr != end() && (*itr)->time <= rightBound; ++itr) {
			if (isConflict(ev, **itr, constraint, singleLine)) {
				return *itr;
			}
		}
	}
	return nullptr;
}

bool StationEventAxis::isConflict(const RailStationEventBase& left,
	const RailStationEventBase& right,
	const GapConstraints& constraint,
	bool singleLine) const
{
	auto gap_type = TrainGap::gapTypeBetween(left, right, singleLine);
	if (gap_type.has_value()) {
		// 构成相干事件，检查间隔是否符合要求。
		// 注意约定正好相等的情况是符合要求（不冲突）的
		int secs = qeutil::secsTo(left.time, right.time);
		//if (secs < constraint.at(*gap_type)) {
		//	return true;
		//}
		// API V2
		if (constraint.checkConflict(*gap_type, secs)) {
			return true;
		}
	}
	return false;
}
