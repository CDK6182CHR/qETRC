#include "trainpathseg.h"
#include "data/rail/railcategory.h"
#include "data/rail/railway.h"


TrainPathSeg::TrainPathSeg(const std::shared_ptr<Railway>& railway_, const StationName& end_station,
	Direction dir, double mile):
	railway(railway_), end_station(end_station), dir(dir), mile(mile)
{
	rail_name = railway_->name();
}

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
		{"rail_name", railwayName()},
		{"end_station", end_station.toSingleLiteral()},
		{"dir", static_cast<int>(dir)},
		{"mile", mile},
	};
}

bool TrainPathSeg::checkIsValid(const StationName& start_name)
{
	if (railway.expired()) {
		qInfo() << "TrainPathSeg::isValid()  railway object is deleted";
		return false;
	}
	auto rail = railway.lock();
	if (!rail->valid()) {
		qWarning() << "Railway " << rail->name() << " is invalid (mostly deleted)";
		return false;
	}

	auto start_st = rail->stationByName(start_name);
	auto end_st = rail->stationByName(end_station);

	if (!start_st) {
		qWarning() << "Start station " << start_name.toSingleLiteral()
			<< " does not belong to railway";
		return false;
	}

	if (!end_st) {
		qWarning() << "End station " << end_station.toSingleLiteral()
			<< " does not belong to railway";
		return false;
	}

	if (start_st->mile == end_st->mile) {
		qWarning() << "Start and end station at same mile, direction cannot be determined";
		return false;
	}

	auto dir = rail->gapDirection(start_st, end_st);

	if (!start_st->isDirectionVia(dir) || !end_st->isDirectionVia(dir)) {
		qWarning() << "Start/end station does not pass the direction " << DirFunc::dirToString(dir);
		return false;
	}

	if (dir != this->dir) {
		qInfo() << "The direction is changed from " << DirFunc::dirToString(this->dir)
			<< " to " << DirFunc::dirToString(dir);
		this->dir = dir;
	}

	double mile = rail->mileBetween(start_st, end_st);
	if (mile == -1) [[unlikely]] {
		qCritical() << "The mile between start and end station cannot be determined";
		return false;
	}

	if (mile != this->mile) {
		qInfo() << "The mile is updated from " << this->mile << " to " << mile;
		this->mile = mile;
	}
	return true;
}

void TrainPathSeg::updateRailway(RailCategory& railcat)
{
	if (railway.expired()) {
		railway = railcat.railwayByName(rail_name);
	}
}
