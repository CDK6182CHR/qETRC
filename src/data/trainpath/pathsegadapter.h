#pragma once

#include <memory>
#include <optional>

#include "data/diagram/trainadapterstation.h"

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
public:
	enum BindType {
		EmptyBind = 0,
		SingleBind,
		NormalBind,
	};

private:
	int m_seg_index;
	std::shared_ptr<TrainLine> m_line;

	BindType m_type;
	std::optional<AdapterStation> m_single_adp_station;   // filled only if m_type == Single

public:

	PathSegAdapter(int seg_index, std::shared_ptr<TrainLine> line) :
		m_seg_index(seg_index), m_line(line), m_type(NormalBind)
	{}
	PathSegAdapter(int seg_index):
		m_seg_index(seg_index), m_line(nullptr), m_type(EmptyBind)
	{ }
	PathSegAdapter(int seg_index, AdapterStation adp_station) :
		m_seg_index(seg_index), m_line(nullptr), m_type(SingleBind), m_single_adp_station(adp_station)
	{
	}

	int segIndex() const { return m_seg_index; }
	std::shared_ptr<TrainLine> line() const { return m_line; }

	auto type()const { return m_type; }
	auto& singleAdapterStation()const { return m_single_adp_station; }

	const AdapterStation& firstAdapterStation()const;
	const AdapterStation& lastAdapterStation()const;
};
