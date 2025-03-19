#include "pathruleriter.h"

#include <cassert>

#include "pathruler.h"
#include "trainpath.h"
#include "data/rail/ruler.h"
#include "data/rail/rulernode.h"


template<typename TyNode>
PathRulerIteratorGen<TyNode>::PathRulerIteratorGen(TyPathRuler* ruler, size_t seg_index, std::shared_ptr<TyNode> cur_node):
	m_ruler(ruler), m_seg_index(seg_index), m_cur_node(std::move(cur_node))
{
}

template<typename TyNode>
PathRulerIteratorGen<TyNode> PathRulerIteratorGen<TyNode>::makeBegin(TyPathRuler* ruler)
{
	PathRulerIteratorGen<TyNode> res(ruler, 0, {});
	res.toSegFirstNode();
	return res;
}

template<typename TyNode>
PathRulerIteratorGen<TyNode> PathRulerIteratorGen<TyNode>::makeEnd(TyPathRuler* ruler)
{
	return PathRulerIteratorGen<TyNode>(ruler, ruler->segments().size(), {});
}

template<typename TyNode>
PathRulerIteratorGen<TyNode>::PathRulerIteratorGen():
	m_ruler(nullptr), m_seg_index(0), m_cur_node()
{
}

template <typename TyNode>
PathRulerIteratorGen<TyNode>& PathRulerIteratorGen<TyNode>::operator++()
{
	next();
	return *this;
}

template <typename TyNode>
PathRulerIteratorGen<TyNode> PathRulerIteratorGen<TyNode>::operator++(int)
{
	auto tmp = *this;   // copy construct
	next();
	return tmp;
}

template <typename TyNode>
PathRulerIteratorGen<TyNode>& PathRulerIteratorGen<TyNode>::operator--()
{
	prev();
	return *this;
}

template <typename TyNode>
PathRulerIteratorGen<TyNode> PathRulerIteratorGen<TyNode>::operator--(int)
{
	auto tmp = *this;
	prev();
	return tmp;
}

template <typename TyNode>
bool PathRulerIteratorGen<TyNode>::operator==(const PathRulerIteratorGen<TyNode>& rhs) const
{
	checkSameRuler(rhs);
	return m_seg_index == rhs.m_seg_index && m_cur_node == rhs.m_cur_node;
}

template <typename TyNode>
bool PathRulerIteratorGen<TyNode>::operator!=(const PathRulerIteratorGen<TyNode>& rhs) const
{
	checkSameRuler(rhs);
	return m_seg_index != rhs.m_seg_index || m_cur_node != rhs.m_cur_node;
}

template <typename TyNode>
void PathRulerIteratorGen<TyNode>::next()
{
	auto& seg = m_ruler->segments().at(m_seg_index);
	assert(!seg.ruler.expired());
	auto seg_ruler = seg.ruler.lock();

	auto& pathseg = m_ruler->path()->segments().at(m_seg_index);
	if (pathseg.end_station == m_cur_node->toStationName()) {
		// We should switch to next segment
		++m_seg_index;
		toSegFirstNode();
	}
	else {
		// Simply the next node
		m_cur_node = m_cur_node->nextNode();
	}
}

template <typename TyNode>
void PathRulerIteratorGen<TyNode>::prev()
{
	auto path = m_ruler->path();
	const auto& seg_start = path->segStartStation(m_seg_index);
	if (seg_start == m_cur_node->fromStationName()) {
		// Switch to previous segment
		assert(m_seg_index > 0);
		--m_seg_index;
		toSegLastNode();
	}
	else {
		m_cur_node = m_cur_node->prevNode();
	}
}

template <typename TyNode>
void PathRulerIteratorGen<TyNode>::toSegFirstNode()
{
	if (m_seg_index >= m_ruler->segments().size()) {
		// pass-the-end iterator
		m_cur_node.reset();
	}
	else {
		// Find the first node of the segment
		auto path = m_ruler->path();
		auto& pathseg = path->segments().at(m_seg_index);
		auto rail = pathseg.railway.lock();
		std::shared_ptr<RailStation> start_st;

		if (m_seg_index == 0) {
			start_st = rail->stationByName(path->startStation());
		}
		else {
			start_st = rail->stationByName(path->segments().at(m_seg_index - 1).end_station);
		}

		auto cur_railint = start_st->dirNextInterval(pathseg.dir);
		m_cur_node = cur_railint->getRulerNode(m_ruler->segments().at(m_seg_index).ruler.lock());
	}
}

template <typename TyNode>
void PathRulerIteratorGen<TyNode>::toSegLastNode()
{
	auto path = m_ruler->path();
	auto& pathseg = path->segments().at(m_seg_index);
	auto rail = pathseg.railway.lock();

	auto last_st = rail->stationByName(pathseg.end_station);
	auto cur_railint = last_st->dirPrevInterval(pathseg.dir);
	m_cur_node = cur_railint->getRulerNode(m_ruler->segments().at(m_seg_index).ruler.lock());
}


template class PathRulerIteratorGen<RulerNode>;
template class PathRulerIteratorGen<const RulerNode>;

