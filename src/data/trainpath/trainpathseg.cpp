#include "trainpathseg.h"
#include "data/rail/railcategory.h"
#include "data/rail/railway.h"


TrainPathSeg::TrainPathSeg(const QJsonObject& obj, const RailCategory& cat):
	rail_name(), railway(), end_station()
{
	fromJson(obj, cat);
}

const QString& TrainPathSeg::railwayName() const
{
	if (railway.expired()) {
		return rail_name;
	}
	else {
		return railway.lock()->name();
	}
}

void TrainPathSeg::fromJson(const QJsonObject& obj, const RailCategory& cat)
{
	rail_name = obj.value("rail_name").toString();
	end_station = obj.value("end_station").toString();
	dir = static_cast<Direction>(obj.value("dir").toInt());
	mile = obj.value("mile").toDouble();

	if (auto rail = cat.railwayByName(rail_name)) {
		railway = rail;   // shared_ptr -> weak_ptr
	}
}

QJsonObject TrainPathSeg::toJson() const
{
	return QJsonObject{
		{"rail_name", rail_name},
		{"end_station", end_station.toSingleLiteral()},
		{"dir", static_cast<int>(dir)},
		{"mile", mile},
	};
}
