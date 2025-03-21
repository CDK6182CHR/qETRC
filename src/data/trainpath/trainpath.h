﻿#pragma once 
#include <vector>
#include <memory>
#include "trainpath.h"
#include "trainpathseg.h"
#include "pathruler.h"


class RailCategory;
class Train;
class TrainCollection;


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
	std::vector<std::weak_ptr<Train>> _trains;

	std::vector<std::shared_ptr<PathRuler>> _rulers;

	friend class TrainPathCollection;

public:

	// this ctor is not used by now
	//TrainPath(const QString& name, const QString& start_station, const std::vector<TrainPathSeg>& segs, 
	//	const QString& note):
	//	_name(name), _start_station(start_station), _segments(segs), _note(note), _valid(true)
	//{}

	TrainPath(const QJsonObject& obj, const RailCategory& cat, TrainCollection& coll) :
		_name(), _start_station(), _segments(), _note()
	{
		fromJson(obj, cat, coll);
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

	auto& trains() { return _trains; }
	auto& trains()const { return _trains; }

	auto& rulers() { return _rulers; }
	auto& rulers()const { return _rulers; }

	std::vector<std::shared_ptr<Train>> trainsShared();

	bool valid()const { return _valid; }

	/**
	 * This function is not trivial; only works for non-empty paths!!!
	 * Be sure not to call this function for empty ones.
	 */
	const StationName& endStation()const;

	const auto& segments()const { return _segments; }
	auto& segments() { return _segments; }

	void fromJson(const QJsonObject& obj, const RailCategory& cat, TrainCollection& coll);
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

	/**
	 * Assign this path to train.
	 * Add train to this->_trains, and add this to train->_paths at the same time.
	 * The *order* of trains in path is maintained, but the order of paths in train is not.
	 * (This is the technique result of the JSON i/o stragety, where only the trains of paths are written.)
	 * N.B. The function is written in TrainPath but not in Train, currently. This is only, considering the trainpath.cpp
	 * is much shorter than train.cpp ...
	 */
	void addTrain(std::shared_ptr<Train> train);

	/**
	 * 2023.08.16  remove the train from the path, also remove this path from the train.
	 * Called by the undo command  AssignPathsToTrain.
	 * In principle, the requested train *should* be the last in the list, and so does the path in train.
	 */
	void removeTrainFromBack(std::shared_ptr<Train> train);

	void insertTrainWithIndex(std::shared_ptr<Train> train, int train_index, int path_index_in_train);

	/**
	 * Remove train at given index. Used by the qecmd::RemovePathsFromTrain
	 */
	void removeTrainWithIndex(std::shared_ptr<Train> train, int train_index, int path_index_in_train);

	void removeAllTrains();

	/**
	 * Get the index of train in this path.
	 * Return -1 if not found.
	 * Simple linear alg.
	 */
	int getTrainIndex(std::shared_ptr<Train> train)const;

	int getRulerIndex(std::shared_ptr<PathRuler> ruler)const;

	bool rulerNameIsValid(const QString& name, std::shared_ptr<PathRuler> ignored)const;

    /**
     * Simple linear alg, calling getTrainIndex()
     */
    bool containsTrain(std::shared_ptr<Train> train)const;

	/**
	 * Linear alg., check every seg
	 */
	bool containsRailway(const Railway* rail)const;

	/**
	 * See also containsRailway.
	 * This version uses name, which is provided for the importRailways cmd
	 * (where the Railway is not actually added)
	 */
	bool containsRailwayByName(const QString& name)const;

	/**
	 * 2025.02.08  Returns the station name of the first station of the segment given by the index.
	 */
	const StationName& segStartStation(size_t idx)const;
};
