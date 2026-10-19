#include "marker_builders.h"

#include <QDebug>


std::unique_ptr<AbstractMarkerBuilder> AbstractMarkerBuilder::creeate(LineMarkerType type)
{
	switch (type) {
	case LineMarkerType::Square:
		return std::unique_ptr<AbstractMarkerBuilder>(new SquareMarkerBuilder);
	default:
		qWarning() << "AbstractMarkerBuilder::create: invalid type encountered " << static_cast<int>(type); 
		[[fallthrough]];
	case LineMarkerType::None:
		return std::unique_ptr<AbstractMarkerBuilder>(new NullMarkerBuilder);
	}
}

double NullMarkerBuilder::occupied_length([[maybe_unused]] double s) const
{
	return 0.0;
}

double SquareMarkerBuilder::occupied_length(double s) const
{
	return s;
}
