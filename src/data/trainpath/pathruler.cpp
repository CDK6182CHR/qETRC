#include "pathruler.h"

PathRuler::PathRuler(std::weak_ptr<TrainPath> path, const QString& name, std::vector<PathRulerSeg> segments):
	m_path(std::move(path)), m_name(name), m_segments(std::move(segments)), m_valid(true)
{
}

PathRulerConstIterator PathRuler::cbegin() const
{
	return PathRulerConstIterator::makeBegin(this);
}

PathRulerMutableIterator PathRuler::begin()
{
	return PathRulerMutableIterator::makeBegin(this);
}

PathRulerConstIterator PathRuler::cend() const
{
	return PathRulerConstIterator::makeEnd(this);
}

PathRulerMutableIterator PathRuler::end()
{
	return PathRulerMutableIterator::makeEnd(this);
}
