﻿#include "trainevents.h"
#include <algorithm>
#include "trainadapter.h"

bool qeutil::timeCompare(const QTime& tm1, const QTime& tm2)
{
	static constexpr int secsOfADay = 3600 * 24;
	int secs = tm1.secsTo(tm2);
	bool res = (secs > 0);
	if (std::abs(secs) > secsOfADay / 2)
		return !res;
	return res;
}

bool StationEvent::operator<(const StationEvent& another) const
{
	if (time == another.time) {
		return static_cast<int8_t>(type) < static_cast<int8_t>(another.type);
	}
	return qeutil::timeCompare(time, another.time);
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
	auto s = QString::asprintf("%.3lf km. ", st->mile);
	s += time.toString(" hh:mm:ss ");
	s += QString(" [") + st->name.toSingleLiteral() + QString("] 站 ");
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



bool IntervalEvent::operator<(const IntervalEvent& another) const
{
	if (time == another.time) {
		return static_cast<int8_t>(type) < static_cast<int8_t>(another.type);
	}
	return qeutil::timeCompare(time, another.time);
}

QString IntervalEvent::toString() const
{
	auto s = QString::asprintf("%.3lf km. ", mile);
	s += time.toString(" hh:mm:ss ");
	s += QString(" [") + former->name.toSingleLiteral() + " - "
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

QString RailStationEvent::posString() const
{
	switch (pos) {
	case 1:return QObject::tr("站前");
	case 2:return QObject::tr("站后");
	case 3:return QObject::tr("前后");
	default:return "INVALID POS";
	}
}

QString RailStationEvent::toString() const
{
	return QStringLiteral("%1 %2 %3").arg(time.toString("hh:mm:ss "))
		.arg(another->get().trainName().full())
		.arg(qeutil::eventTypeString(type));
}
