#pragma once 
#include <vector>
#include "trainpath.h"
#include "trainpathseg.h"


/**
 * 2023.8.3
 * The class representing the train path (ÁÐ³µ¾¶Â·).
 * It seems that, directly using the vector of TrainPathSeg (value type) is enough for now
 */
class TrainPath {
	QString _name;
	StationName _start_station;
	std::vector<TrainPathSeg> _segments;
	QString _note;

public:
	TrainPath(const QString& name, const QString& start_station, const std::vector<TrainPathSeg>& segs, 
		const QString& note):
		_name(name), _start_station(start_station), _segments(segs), _note(note)
	{}

	const QString& name()const { return _name; }
	void setName(const QString& n) { _name = n; }
	const QString& note()const { return _note; }
	void setNote(const QString& n) { _note = n; }
	const auto& startStation()const { return _start_station; }
	void setStartStation(const StationName& n) { _start_station = n; }

	const auto& segments()const { return _segments; }
	auto& segments() { return _segments; }
};
