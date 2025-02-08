#pragma once

#include "data/rail/abstractruler.h"

#include <vector>
#include <memory>
#include <QString>

#include "pathrulerseg.h"
#include "pathruleriter.h"

class Ruler;
class TrainPath;

/**
 * The ruler that attached with a TrainPath. Similar to the `Ruler` class, 
 * sharing the same set of abstract API set
 */
class PathRuler : public AbstractRuler
{
	std::weak_ptr<TrainPath> m_path;
	QString m_name;
	std::vector<PathRulerSeg> m_segments;
	bool m_valid;

public:

	/**
	 * Constructor provided for GUI construction. All data must be valid at this construction stage.
	 */
	PathRuler(std::weak_ptr<TrainPath> path, const QString& name, std::vector<PathRulerSeg> segments);

	auto& segments()const { return m_segments; }
	auto& segmentsRef() { return m_segments; }

	auto& path()const { return m_path; }

	bool valid()const { return m_valid; }

	PathRulerIterator begin();
	PathRulerIterator cbegin()const;
	
};
