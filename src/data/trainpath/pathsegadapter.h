#pragma once

#include <memory>

class TrainLine;
struct TrainPathSeg;


/**
 * Adapter for a segment of train path.
 * Contains the information of the path segment and the pointer to the corresponding train line (if any).
 * ---
 * The object is always created upon painting train lines. The data is always consistent with that in the train path.
 * At this moment, we use shared_ptr for the line. In principle we could also use weak_ptr.
 */
class PathSegAdapter
{
	int m_seg_index;
	std::shared_ptr<TrainLine> m_line;

public:
	PathSegAdapter(int seg_index, std::shared_ptr<TrainLine> line) :
		m_seg_index(seg_index), m_line(line)
	{}
	PathSegAdapter(int seg_index):
		m_seg_index(seg_index), m_line(nullptr)
	{ }

	int segIndex() const { return m_seg_index; }
	std::shared_ptr<TrainLine> line() const { return m_line; }
};
