#include "trainevents.h"
#include <algorithm>
#include "trainadapter.h"
#include "trainline.h"
#include "data/rail/railstation.h"
#include "data/train/train.h"
#include "util/utilfunc.h"

bool StationEvent::operator<(const StationEvent& rhs) const
{
	if (time == rhs.time) {
		return static_cast<int8_t>(type) < static_cast<int8_t>(rhs.type);
	}
	return qeutil::timeCompare(time, rhs.time, period_hours);
}

//enum class TrainEventType :int8_t {
//	Meet,   //会车
//	OverTaking,  //越行
//	Avoid,   //待避
//	Arrive,  //到达
//	Depart,  //出发
//	SettledPass,   //通过 （图定）
//	CalculatedPass,    //通过 （推算）
//	Origination,    //始发
//	Destination,    //终到
//	Coincidence,    //重合，或共线
//	Undefined
//};

QString StationEvent::toString() const
{
	auto st = station.lock();
	auto s = QString::asprintf("%.3lf km.  ", st->mile);
	s += time.toString(TrainTime::HMS);
	s += QString("  [") + st->name.toSingleLiteral() + QString("] 站 ");
	switch (type) {
	case TrainEventType::Arrive: return s + "到达";
	case TrainEventType::Depart:return s + "发车";
	case TrainEventType::SettledPass:return s + "通过 (图定)";
	case TrainEventType::CalculatedPass:return s + "通过 (推算)";
	case TrainEventType::Origination:return s + "始发";
	case TrainEventType::Destination:return s + "终到";
	case TrainEventType::Meet:return s + "会 " + another.value().get().trainName().full();
	case TrainEventType::Avoid:return s + "让 " + another.value().get().trainName().full();
	case TrainEventType::OverTaking:return s + "越 " + another.value().get().trainName().full();
	case TrainEventType::Coincidence:return s + "共线 " + another.value().get().trainName().full();
	default:return s + " ERROR TrainEventType";
	}
}



bool IntervalEvent::operator<(const IntervalEvent& rhs) const
{
	if (time == rhs.time) {
		return static_cast<int8_t>(type) < static_cast<int8_t>(rhs.type);
	}
	return qeutil::timeCompare(time, rhs.time, period_hours);
}

QString IntervalEvent::toString() const
{
	auto s = QString::asprintf("%.3lf km.  ", mile);
	s += time.toString(TrainTime::HMS);
	s += QString("  [") + former->name.toSingleLiteral() + " - "
		+ latter->name.toSingleLiteral() + QString("] 区间 ");
	switch (type) {
	case TrainEventType::Meet:return s + "会 " + another.get().trainName().full();
	case TrainEventType::Avoid:return s + "让 " + another.get().trainName().full();
	case TrainEventType::OverTaking:return s + "越 " + another.get().trainName().full();
	case TrainEventType::Coincidence:return s + "共线 " + another.get().trainName().full();
	default:return s + " ERROR TrainEventType";
	}
}


void StationEventList::emplace(StationEvent&& e)
{
	//采用upper_bound -- 新插入的被认为是最大的。
	//QList的这个insert其实是Copy
	auto it = std::upper_bound(stEvents.begin(), stEvents.end(), e);
	stEvents.insert(it, std::forward<StationEvent>(e));
}

void StationEventList::emplace(IntervalEvent&& e)
{
	auto it = std::upper_bound(itEvents.begin(), itEvents.end(), e);
	itEvents.insert(it, std::forward<IntervalEvent>(e));
}

QString qeutil::eventTypeString(TrainEventType t)
{
	switch (t)
	{
	case TrainEventType::Meet:return QObject::tr("交会");
		break;
	case TrainEventType::OverTaking:return QObject::tr("越行");
		break;
	case TrainEventType::Avoid:return QObject::tr("让行");
		break;
	case TrainEventType::Arrive:return QObject::tr("到达");
		break;
	case TrainEventType::Depart:return QObject::tr("出发");
		break;
	case TrainEventType::SettledPass:
	case TrainEventType::CalculatedPass:return QObject::tr("通过");
		break;
	case TrainEventType::Origination:return QObject::tr("始发");
		break;
	case TrainEventType::Destination:return QObject::tr("终到");
		break;
	case TrainEventType::Coincidence:return QObject::tr("共线");
		break;
	default:return QObject::tr("非法事件");
		break;
	}
}

TrainEventType qeutil::formerEventType(bool isStop)
{
	return isStop ? TrainEventType::Arrive : TrainEventType::SettledPass;
}

TrainEventType qeutil::latterEventType(bool isStop)
{
	return isStop ? TrainEventType::Depart : TrainEventType::SettledPass;
}

RailStationEvent::RailStationEvent(TrainEventType type_, const TrainTime& time_, 
	std::weak_ptr<const RailStation> station_, std::shared_ptr<const TrainLine> line_,
	Positions pos_, const QString& note_):
	RailStationEventBase(type_,time_,pos_,line_->dir()),
	station(station_),line(line_),note(note_)
{
}

QString RailStationEventBase::posString() const
{
	switch (pos) {
	case Pre:return QObject::tr("站前");
	case Post:return QObject::tr("站后");
	case Both:return QObject::tr("前后");
	default:return "INVALID POS";
	}
}

QString RailStationEvent::toString() const
{
	return QStringLiteral("%1 %2 %3 ").arg(time.toString(TrainTime::HMS))
		.arg(line->train()->trainName().full())
		.arg(qeutil::eventTypeString(type));
}

QString RailStationEventBase::posToString(const Positions& pos)
{
	switch (pos) {
	case Pre:return QObject::tr("站前");
	case Post:return QObject::tr("站后");
	case Both:return QObject::tr("前后");
	//case NoPos:return QObject::tr("对向");
	default:return "";
	}
}

int RailStationEventBase::secsTo(const RailStationEventBase& rhs, int period_hours) const
{
	return qeutil::secsTo(time, rhs.time, period_hours);
}

bool RailStationEventBase::hasAppend() const
{
	switch (type)
	{
	case TrainEventType::Arrive:
	case TrainEventType::Depart:
	case TrainEventType::Origination:
	case TrainEventType::Destination:return true;
	case TrainEventType::SettledPass:
	case TrainEventType::CalculatedPass:return false;
	default: return false;
	}
}

QString qeutil::diagnoLevelString(DiagnosisLevel level)
{
	switch (level)
	{
	case qeutil::Information: return "INFO";
		break;
	case qeutil::Warning: return "WARNING";
		break;
	case qeutil::Error:return "ERROR";
		break;
	default:return {};
		break;
	}
}

QString qeutil::diagnoTypeString(DiagnosisType type)
{
	switch (type)
	{
	case DiagnosisType::StopTooLong1:
	case DiagnosisType::StopTooLong2:return QObject::tr("停时过长");
		break;
	case DiagnosisType::IntervalTooLong1:
	case DiagnosisType::IntervalTooLong2:return QObject::tr("区间过长");
		break;
	case DiagnosisType::IntervalMeet:return QObject::tr("区间交会");
		break;
	case DiagnosisType::IntervalOverTaking:return QObject::tr("区间越行");
		break;
	case DiagnosisType::CollidForbid:return QObject::tr("天窗冲突");
		break;
	case DiagnosisType::SystemError:return QObject::tr("系统错误");
	default:return {};
		break;
	}
}

QString DiagnosisIssue::posString() const
{
	if (std::holds_alternative<std::shared_ptr<const RailStation>>(pos)) {
		return std::get<std::shared_ptr<const RailStation>>(pos)->name.toSingleLiteral();
	}
	else {
		return std::get<std::shared_ptr<const RailInterval>>(pos)->toString();
	}
}

bool DiagnosisIssue::inRange(std::shared_ptr<RailStation> start, std::shared_ptr<RailStation> end) const
{
	double m_min = start->mile, m_max = end->mile;
	if (std::holds_alternative<std::shared_ptr<const RailStation>>(pos)) {
		auto st = std::get<std::shared_ptr<const RailStation>>(pos);
		return m_min <= st->mile && st->mile <= m_max;
	}
	else {
		auto railint = std::get<std::shared_ptr<const RailInterval>>(pos);
		double inter_min = railint->fromStation()->mile,
			inter_max = railint->toStation()->mile;
		if (inter_min > inter_max)
			std::swap(inter_min, inter_max);
		return std::max(m_min, inter_min) < std::min(m_max, inter_max);
	}
}

RailStationEventBase::Position qeutil::dirFormerPos(Direction dir)
{
	switch (dir)
	{
	case Direction::Down: return RailStationEventBase::Pre;
		break;
	case Direction::Up:return RailStationEventBase::Post;
		break;
	default:return RailStationEventBase::NoPos;
		break;
	}
}

RailStationEventBase::Position qeutil::dirLatterPos(Direction dir)
{
	switch (dir)
	{
	case Direction::Down: return RailStationEventBase::Post;
		break;
	case Direction::Up:return RailStationEventBase::Pre;
		break;
	default:return RailStationEventBase::NoPos;
		break;
	}
}
