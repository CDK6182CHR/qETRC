#pragma once 
#include <vector>
#include "trainpath.h"
#include "trainpathseg.h"


class RailCategory;


/**
 * 2023.8.3
 * The class representing the train path (列车径路).
 * It seems that, directly using the vector of TrainPathSeg (value type) is enough for now
 */
class TrainPath {
	QString _name;
	StationName _start_station;
	std::vector<TrainPathSeg> _segments;
	QString _note;
	bool _valid;

public:

	// this ctor is not used by now
	//TrainPath(const QString& name, const QString& start_station, const std::vector<TrainPathSeg>& segs, 
	//	const QString& note):
	//	_name(name), _start_station(start_station), _segments(segs), _note(note), _valid(true)
	//{}

	TrainPath(const QJsonObject& obj, const RailCategory& cat) :
		_name(), _start_station(), _segments(), _note()
	{
		fromJson(obj, cat);
	}

	TrainPath(const QString& name):
		_name(name), _valid(false)
	{}

	// used for duplicate
	TrainPath(const TrainPath& other) = default;

	const QString& name()const { return _name; }
	void setName(const QString& n) { _name = n; }
	const QString& note()const { return _note; }
	void setNote(const QString& n) { _note = n; }
	const auto& startStation()const { return _start_station; }
	void setStartStation(const StationName& n) { _start_station = n; }

	bool valid()const { return _valid; }

	/**
	 * This function is not trivial; only works for non-empty paths!!!
	 * Be sure not to call this function for empty ones.
	 */
	const StationName& endStation()const;

	const auto& segments()const { return _segments; }
	auto& segments() { return _segments; }

	void fromJson(const QJsonObject& obj, const RailCategory& cat);
	QJsonObject toJson()const;

	bool empty()const { return _segments.empty(); }

	void swapDataWith(TrainPath& other);

	/**
	 * Check whether current Path is valid, and set the flag.
	 * The direction/mile data may be changed.
	 * Note: this operation may be slow.
	 */
	bool checkIsValid();

	/**
	 *  Try to update the railway of all the segments.
	 */
	void updateRailways(RailCategory& cat);
};
