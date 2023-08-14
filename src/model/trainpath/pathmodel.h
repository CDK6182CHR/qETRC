#pragma once

#include <QStandardItemModel>

class TrainPath;
struct TrainPathSeg;
class StationName;

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

private:
	void setupRow(int row, const TrainPathSeg& seg, const StationName& lastStation);
};