#include "pathruler.h"

PathRuler::PathRuler(std::weak_ptr<TrainPath> path, const QString& name, std::vector<PathRulerSeg> segments):
	m_path(std::move(path)), m_name(name), m_segments(std::move(segments)), m_valid(true)
{
}
