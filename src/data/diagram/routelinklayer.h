/**
 * Route link layer manager data structures
 * The algorithm is actually like both RailTrack and LabelHeightManager.
 * Here, we use similar alg to that used in RailTrack, but with lighter data, for more encapsule.
 */

#pragma once
#include <set>
#include <vector>

class Train;

/**
 * Similar to TrackOccupy. Here, the struct that similar to TrackItem is omitted.
 */
struct RouteLinkOccupy {
	const Train* train;
	int start_x, end_x;

	RouteLinkOccupy(const Train* train, int start_x, int end_x):
		train(train), start_x(start_x), end_x(end_x)
	{ }

	/**
	 * Similar to TrackOccupy::Comparator, compare according to END x!!
	 */
	struct Comparator {
		using is_transparent = std::true_type;
		bool operator()(const RouteLinkOccupy& a, const RouteLinkOccupy& b)const {
			return a.end_x < b.end_x;
		}
		bool operator()(const RouteLinkOccupy& a, int x)const {
			return a.end_x < x;
		}
		bool operator()(int x, const RouteLinkOccupy& b)const {
			return x < b.end_x;
		}
	};
};

/**
 * Similar to Track, one layer with non-intersected occupations.
 */
class RouteLinkLayer {
	using item_set_t = std::set<RouteLinkOccupy, RouteLinkOccupy::Comparator>;
	item_set_t _items;

public:
	RouteLinkLayer() = default;

	auto& items()const { return _items; }

	/**
	 * Returns the iterator of the first occupation in specified range.
	 */
	item_set_t::const_iterator occupationInRange(int start_x, int end_x)const;

	/**
	 * 2024.02.22  This version considers PBC. If start_x > end_x, the range is automatically split into two ranges.
	 * Note, the x's should start from 0.
	 * Note also, both start_x and end_x should be in vaild range
	 */
	item_set_t::const_iterator occupationInRangePBC(int start_x, int end_x, const int width)const;

	/**
	 * 2024.02.21  Returns whether the range specified by x's is occupied.
	 * Precondition: start_x <= end_x  (no PBC considered!)
	 * Same x-value between adjacent occupations is allowed.
	 */
	bool isOccupied(int start_x, int end_x)const;

	bool isOccupiedPBC(int start_x, int end_x, const int width)const;

	/**
	 * 2024.02.22 Add occupation. The conflication is NOT checked here!
	 * MIND: PBC should be considered!!
	 */
	void addOccupation(const RouteLinkOccupy& occ, const int width);

	/**
	 * Delete occupation using the train pointer and the x coords.
	 * PBC is considered.
	 */
	void delOccupation(const Train* train, int x1, int x2, const int width);

private:

	/**
	 * Simply delete occupation, without PBC
	 */
	void delOccupationSimple(const Train* train, int x_end);
};


/**
 * A manager is a sequential-list (implemented with vector) off RouteLinkLayers, 
 * which is similar to a station in the context of RailTrack analysis.
 * Each station has two managers, one for above and another for below.
 */
class RouteLinkLayerManager {
	std::vector<RouteLinkLayer> _layers;

public:
	RouteLinkLayerManager() = default;

	size_t num_layers()const { return _layers.size(); }

	/**
	 * Add the given occupation info to layers, and return the number of layer it being added to.
	 * Here, the width should be TOTAL width (i.e. corresponding to 24 hour), even if some part may not be shown.
	 */
	int addOccupation(const RouteLinkOccupy& occ, const int width);

	void delOccupation(int layer, const Train* train, int x1, int x2, const int width);
};

