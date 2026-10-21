#include "marker_builders.h"

#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QPolygonF>

#include "marker_info.h"

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

double AbstractMarkerBuilder::occupied_length(double size, [[maybe_unused]] int index)const
{
	return max_occupied_length(size);
}

double NullMarkerBuilder::max_occupied_length([[maybe_unused]] double s) const
{
	return 0.0;
}

QAbstractGraphicsShapeItem* NullMarkerBuilder::build_marker(const MarkerInfo& info, double size, const QPen& pen, QGraphicsItem* parent)
{
	Q_UNUSED(info);
	Q_UNUSED(size);
	Q_UNUSED(pen);
	Q_UNUSED(parent);
	return nullptr;
}

double SquareMarkerBuilder::max_occupied_length(double s) const
{
	return s;
}

QAbstractGraphicsShapeItem* SquareMarkerBuilder::build_marker(const MarkerInfo& info, double size, const QPen& pen, QGraphicsItem* parent)
{
	// normalized vector for the line: [dx, dy]; vertical one: [-dy, dx]
	QList<QPointF> points;
	points.append(QPointF(info.center.x() + (-info.dx - info.dy) * size / 2, info.center.y() + (-info.dy + info.dx) * size / 2));
	points.append(QPointF(info.center.x() + (-info.dx + info.dy) * size / 2, info.center.y() + (-info.dy - info.dx) * size / 2));
	points.append(QPointF(info.center.x() + (+info.dx + info.dy) * size / 2, info.center.y() + (+info.dy - info.dx) * size / 2));
	points.append(QPointF(info.center.x() + (+info.dx - info.dy) * size / 2, info.center.y() + (+info.dy + info.dx) * size / 2));

	auto* p = new QGraphicsPolygonItem(QPolygonF(points), parent);
	p->setPen(pen);
	return p;
}
