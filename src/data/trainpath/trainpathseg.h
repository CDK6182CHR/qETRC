#pragma once
#include <memory>
#include <QJsonObject>
#include "data/common/stationname.h"
#include "data/common/direction.h"

class Railway;
class RailCategory;

/**
 * 2023.08.03  A "segment" for train path.
 * A segment consists of a piece of railway, which is practically specified by
 * providing railway pointer and end station name.
 * Note that the validity of this segment should be verified outside.
 * MIND: the existence of Railway pointer does not imply its validity; the Railways deleted by user are typically
 * not deleted actually (due to undo system).
 */
struct TrainPathSeg {
	QString rail_name;
	std::weak_ptr<Railway> railway;
	StationName end_station;
	Direction dir;
	double mile;

	//TrainPathSeg(const QString& rail_name, const StationName& end_station) :
	//	rail_name(rail_name), end_station(end_station)
	//{

	//}

	//TrainPathSeg(const std::weak_ptr<Railway>& railway, const StationName& end_station) :
	//	railway(railway), end_station(end_station)
	//{

	//}

	TrainPathSeg(const QJsonObject& obj, const RailCategory& cat);

	const QString& railwayName()const;

	void fromJson(const QJsonObject& obj, const RailCategory& cat);
	
	QJsonObject toJson()const;
};