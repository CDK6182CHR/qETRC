#pragma once

#include <memory>

#include <vector>
#include "pathsegadapter.h"

class TrainPath;
class Train;

/**
 * 2026.02.23: Experimental.
 * Adapter between the TrainPath and TrainLines, or the adapter for the trains to the paths, similar to TrainAdapter.
 * For better design of algorithms making use of train paths.
 * Basically, a path adapter is a collection of path segment adapters, where the segment adapter is a combination of 
 * a path segment and the corresponding train line. Note that by construction, each path segment could have at most 
 * one train line.
 */
class PathAdapter
{
	std::weak_ptr<Train> m_train;
	TrainPath* m_path;

	std::vector<PathSegAdapter> m_segments;
	bool m_valid;

public:

	/**
	 * Create a valid adapter. In this case, the segment count is always consistent with the path.
	 */
	PathAdapter(std::weak_ptr<Train> train, TrainPath* path, std::vector<PathSegAdapter> segments);

	/**
	 * Create an INVALID adapter. In this case, the segment is always empty.
	 */
	PathAdapter(std::weak_ptr<Train> train, TrainPath* path);

	/**
	 * Core function. The main algorithm is moved from TrainAdapter::bindTrainByPath, with the same calling condition.
	 * The train adapters will be added to the train, and edited, when needed.
	 */
	static PathAdapter bind(std::shared_ptr<Train> train, TrainPath* path);

	const std::vector<PathSegAdapter>& segments() const { return m_segments; }
	std::vector<PathSegAdapter>& segments() { return m_segments; }

	/**
	 * When the path is in an invalid state, the adapter is created, but the status is invalid.
	 */
	bool isValid()const { return m_valid; }

	/**
	 * 2026.02.23  Global simple interpolation using this adapter.
	 * Requires the adapter (including all lines) to be consistent with the path, the train, etc.
	 * For segments with lines, the INTERpolation is implemented using the alg in TrainLine, which may use the information
	 * of ruler (actually, the y-coeff); 
	 * for early-stop or late-start segments, or segments w/o line, the EXTRApolation is implemented using the linear-interp
	 * using the mile information.
	 * returns: number of interpolated stations.
	 */
	int timetableInterpolationSimple(int period_hours);

};
