#pragma once

#include <QString>

/**
 * Returns the configuration data for a station, used in greedy-paint.
 * Including settled stop time, fixed check box data etc
 */
struct GreedyPaintStationConfigData 
{
	int settledStopSecs = 0;
	bool fixedStop = false;
	QString track;
	int adjustIntervalSecs = 0;
};
