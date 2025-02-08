#include "pathruleriter.h"

#include <cassert>

#include "pathruler.h"
#include "trainpath.h"
#include "data/rail/ruler.h"
#include "data/rail/rulernode.h"

PathRulerIterator::PathRulerIterator():
	m_ruler(nullptr), m_seg_index(0), m_cur_node()
{
}

PathRulerIterator& PathRulerIterator::operator++()
{
	next();
	return *this;
}

PathRulerIterator PathRulerIterator::operator++(int)
{
	auto tmp = *this;   // copy construct
	next();
	return tmp;
}

PathRulerIterator& PathRulerIterator::operator--()
{
	prev();
	return *this;
}

PathRulerIterator PathRulerIterator::operator--(int)
{
	auto tmp = *this;
	prev();
	return tmp;
}

bool PathRulerIterator::operator==(const PathRulerIterator& rhs) const
{
	checkSameRuler(rhs);
	return m_seg_index == rhs.m_seg_index && m_cur_node == rhs.m_cur_node;
}

bool PathRulerIterator::operator!=(const PathRulerIterator& rhs) const
{
	checkSameRuler(rhs);
	return m_seg_index != rhs.m_seg_index || m_cur_node != rhs.m_cur_node;
}

void PathRulerIterator::next()
{
	auto& seg = m_ruler->segments().at(m_seg_index);
	assert(!seg.ruler.expired());
	auto seg_ruler = seg.ruler.lock();

	auto& pathseg = m_ruler->path().lock()->segments().at(m_seg_index);
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

void PathRulerIterator::prev()
{
	auto path = m_ruler->path().lock();
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

void PathRulerIterator::toSegFirstNode()
{
	if (m_seg_index >= m_ruler->segments().size()) {
		// pass-the-end iterator
		m_cur_node.reset();
	}
	else {
		// Find the first node of the segment
		auto path = m_ruler->path().lock();
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

void PathRulerIterator::toSegLastNode()
{
	auto path = m_ruler->path().lock();
	auto& pathseg = path->segments().at(m_seg_index);
	auto rail = pathseg.railway.lock();

	auto last_st = rail->stationByName(pathseg.end_station);
	auto cur_railint = last_st->dirPrevInterval(pathseg.dir);
	m_cur_node = cur_railint->getRulerNode(m_ruler->segments().at(m_seg_index).ruler.lock());
}

void PathRulerIterator::checkSameRuler(const PathRulerIterator& rhs)const
{
	assert(m_ruler == rhs.m_ruler);
}
