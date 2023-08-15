#pragma once

#include <memory>
#include <QStandardItemModel>

class TrainPath;
struct TrainPathSeg;
class StationName;
class Railway;

class PathModel :public QStandardItemModel
{
	Q_OBJECT;
	TrainPath* path = nullptr;
public:
	enum {
		ColRail=0,
		ColStart,
		ColEnd,
		ColDir,
		ColMile,
		ColMAX
	};

	PathModel(QObject* parent = nullptr);

	/**
	 * Set the data from the class to the model.
	 * Currently, the data class (TrainPath) is not refreshed or checked.
	 */
	void refreshData();

	/**
	 * Simply set path and NOT refresh data.
	 * The data refreshing is called outside.
	 */
	void setPath(TrainPath* p) {
		path = p;
	}

	/**
	 * Insert an empty row at given row index.
	 * Take the starting station automatically from the former row (if exists).
	 */
	void insertEmptyRow(int row);

	std::shared_ptr<Railway> getRailwayForRow(int row);

	/**
	 * Called by the delegate AFTER the railway is changed. 
	 */
	void onRailwayChanged(int row);

	/**
	 * Called by the delegate AFTER the station is changed.
	 */
	void onStationChanged(const QModelIndex& idx);

private:
	void setupRow(int row, const TrainPathSeg& seg, const StationName& lastStation);


	/**
	 * Check the validity of the given row. If valid, calculate and set the mile and direction.
	 * Otherwise, set the foreground to red.
	 * The row is valid if and only if:
	 * (1) the railway exists,
	 * (2) the start and end station belongs to the railway, and
	 * (3) the interval from the start station to the end station exists.
	 * The railway is checked by the data stored in the item, while the stations are just the names.
	 */
	void checkRowValidity(int row);
};
