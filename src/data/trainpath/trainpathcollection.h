#pragma once
#include <vector>
#include <QJsonArray>
#include "trainpath.h"

class RailCategory;
class TrainCollection;
class Railway;

/**
 * 2023.08.05  The collection of TrainPaths in current Diagram.
 * Currently, just simply wraps the list of objects (maintained by unique_ptr for now)
 */
class TrainPathCollection {
	std::vector<std::unique_ptr<TrainPath>> _paths;
public:
	TrainPathCollection() = default;
	TrainPathCollection(const TrainPathCollection&) = delete;
	TrainPathCollection(TrainPathCollection&&)noexcept = default;

	TrainPathCollection& operator=(const TrainPathCollection&) = delete;
	TrainPathCollection& operator=(TrainPathCollection&&)noexcept = default;

	auto& paths() { return _paths; }
	auto& paths()const { return _paths; }

	void fromJson(const QJsonArray& obj, const RailCategory& cat, TrainCollection& coll);
	QJsonArray toJson()const;

	int size()const { return static_cast<int>(_paths.size()); }

	auto* at(int i)const { return _paths.at(i).get(); }

	void clear();

	/**
	 * Simple linear alg.
	 */
	bool isValidNewPathName(const QString& name, TrainPath* ignore=nullptr)const;

	QString validNewPathName(const QString& prefix)const;

	/**
	 * simple linear alg. Find the index of given path
	 */
	int pathIndex(const TrainPath* p)const;

	/**
	 * Refresh the validity of all paths. 
	 * Called by the global refresh process (F5).
	 */
	void checkValidAll(RailCategory& railcat);

	/**
	 * 2023.08.21  update the validity for the paths containing the `rail`.
	 * Note that this is used for cases where the railway is added or removed. 
	 * If the railway is only modified, the process in Diagram::updateRailway() is enough.
	 */
	void checkValidForRailway(RailCategory& railcat, const Railway* rail);

	/**
	 * 2023.08.21: upon the railway is removed. All the paths containing such railway
	 * is set to invalid directly.
	 */
	void invalidateForRailway(const Railway* rail);

	/**
	 * Returns the paths that not assigned to the train.
	 */
	std::vector<TrainPath*> unassignedPaths(const Train& train);

	/**
	 * 2023.08.23  returns the trains that belongs to the paths containing the given railway.
	 */
	std::vector<std::shared_ptr<Train>> affectedTrainsByRailway(std::shared_ptr<const Railway> railway)const;
};
