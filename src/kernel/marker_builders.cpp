#include "marker_builders.h"

#include <QDebug>

AbstractMarkerBuilder::AbstractMarkerBuilder(QPointF center):
	m_center(std::move(center))
{
}

std::unique_ptr<AbstractMarkerBuilder> AbstractMarkerBuilder::creeate(LineMarkerType type, QPointF center)
{
	switch (type) {
	case LineMarkerType::Square:
		return std::unique_ptr<AbstractMarkerBuilder>(new SquareMarkerBuilder(std::move(center)));
	default:
		qWarning() << "AbstractMarkerBuilder::create: invalid type encountered " << static_cast<int>(type);
		return std::unique_ptr<AbstractMarkerBuilder>(new NullMarkerBuilder(std::move(center)));
	}
}

double NullMarkerBuilder::occupied_length(double s) const
{
	return 0.0;
}

double SquareMarkerBuilder::occupied_length(double s) const
{
	return s;
}
