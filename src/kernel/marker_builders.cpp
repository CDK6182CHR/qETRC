#include "marker_builders.h"

#include <QDebug>
#include <QGraphicsPolygonItem>
#include <QPolygonF>

#include "marker_info.h"

std::unique_ptr<AbstractMarkerBuilder> AbstractMarkerBuilder::creeate(LineMarkerType type)
{
	switch (type) {
	case LineMarkerType::Square: return std::unique_ptr<AbstractMarkerBuilder>(new SquareMarkerBuilder);
	case LineMarkerType::DoubleLine: return std::unique_ptr<AbstractMarkerBuilder>(new DoubleLineMarkerBuilder);
	case LineMarkerType::Triangle: return std::unique_ptr<AbstractMarkerBuilder>(new TriangleMarkerBuilder);
	case LineMarkerType::Circle: return std::unique_ptr<AbstractMarkerBuilder>(new CircleMarkerBuilder);
	case LineMarkerType::Cross: return std::unique_ptr<AbstractMarkerBuilder>(new CrossMarkerBuilder);
	case LineMarkerType::AddLine: return std::unique_ptr<AbstractMarkerBuilder>(new AddLineMarkerBuilder);
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

QAbstractGraphicsShapeItem* NullMarkerBuilder::build_marker(const MarkerInfo& info, double size, QGraphicsItem* parent)
{
	Q_UNUSED(info);
	Q_UNUSED(size);
	Q_UNUSED(parent);
	return nullptr;
}

double SquareMarkerBuilder::max_occupied_length(double s) const
{
	return s;
}

QAbstractGraphicsShapeItem* SquareMarkerBuilder::build_marker(const MarkerInfo& info, double size, QGraphicsItem* parent)
{
	// normalized vector for the line: [dx, dy]; vertical one: [-dy, dx]
	QList<QPointF> points;
	points.append(QPointF(info.center.x() + (-info.dx - info.dy) * size / 2, info.center.y() + (-info.dy + info.dx) * size / 2));
	points.append(QPointF(info.center.x() + (-info.dx + info.dy) * size / 2, info.center.y() + (-info.dy - info.dx) * size / 2));
	points.append(QPointF(info.center.x() + (+info.dx + info.dy) * size / 2, info.center.y() + (+info.dy - info.dx) * size / 2));
	points.append(QPointF(info.center.x() + (+info.dx - info.dy) * size / 2, info.center.y() + (+info.dy + info.dx) * size / 2));

	auto* p = new QGraphicsPolygonItem(QPolygonF(points), parent);
	return p;
}

double DoubleLineMarkerBuilder::max_occupied_length(double s) const
{
	return s / 2;
}

QAbstractGraphicsShapeItem* DoubleLineMarkerBuilder::build_marker(const MarkerInfo& info, double size,  QGraphicsItem* parent)
{
	QPainterPath path;
	path.moveTo(QPointF(info.center.x() + (-info.dx / 2 - info.dy) * size / 2, info.center.y() + (-info.dy / 2 + info.dx) * size / 2));
	path.lineTo(QPointF(info.center.x() + (-info.dx / 2 + info.dy) * size / 2, info.center.y() + (-info.dy / 2 - info.dx) * size / 2));
	path.closeSubpath();

	path.moveTo(QPointF(info.center.x() + (+info.dx / 2 + info.dy) * size / 2, info.center.y() + (+info.dy / 2 - info.dx) * size / 2));
	path.lineTo(QPointF(info.center.x() + (+info.dx / 2 - info.dy) * size / 2, info.center.y() + (+info.dy / 2 + info.dx) * size / 2));

	return new QGraphicsPathItem(path, parent);
}

double TriangleMarkerBuilder::max_occupied_length(double s) const
{
	return s;
}

QAbstractGraphicsShapeItem* TriangleMarkerBuilder::build_marker(const MarkerInfo& info, double size, QGraphicsItem* parent)
{
	QList<QPointF> points;
	static const double sqrt3 = std::sqrt(3);
	points.append(QPointF(info.center.x() + (-info.dx / 2 - info.dy / sqrt3) * size, info.center.y() + (-info.dy / 2 + info.dx / sqrt3) * size));
	points.append(QPointF(info.center.x() + (-info.dx / 2 + info.dy / sqrt3) * size, info.center.y() + (-info.dy / 2 - info.dx / sqrt3) * size));
	points.append(QPointF(info.center.x() + (+info.dx / 2) * size, info.center.y() + (+info.dy) * size / 2));

	auto* p = new QGraphicsPolygonItem(QPolygonF(points), parent);
	return p;
}

double CircleMarkerBuilder::max_occupied_length(double s) const
{
	return s;
}

QAbstractGraphicsShapeItem* CircleMarkerBuilder::build_marker(const MarkerInfo& info, double size, QGraphicsItem* parent)
{
	return new QGraphicsEllipseItem(info.center.x() - size / 2, info.center.y() - size / 2, size, size, parent);
}

double CrossMarkerBuilder::max_occupied_length(double s) const
{
	return s;
}

QAbstractGraphicsShapeItem* CrossMarkerBuilder::build_marker(const MarkerInfo& info, double size, QGraphicsItem* parent)
{
	QPainterPath path;
	path.moveTo(QPointF(info.center.x() + (-info.dx - info.dy) * size / 2, info.center.y() + (-info.dy + info.dx) * size / 2));
	path.lineTo(QPointF(info.center.x() + (+info.dx + info.dy) * size / 2, info.center.y() + (+info.dy - info.dx) * size / 2));

	path.closeSubpath();

	path.moveTo(QPointF(info.center.x() + (+info.dx - info.dy) * size / 2, info.center.y() + (+info.dy + info.dx) * size / 2));
	path.lineTo(QPointF(info.center.x() + (-info.dx + info.dy) * size / 2, info.center.y() + (-info.dy - info.dx) * size / 2));

	return new QGraphicsPathItem(path, parent);
}

double AddLineMarkerBuilder::max_occupied_length(double s) const
{
	return s + 6;
}

double AddLineMarkerBuilder::occupied_length(double s, int index) const
{
	return index % 2 == 0 ? max_occupied_length(s): s;
}

QAbstractGraphicsShapeItem* AddLineMarkerBuilder::build_marker(const MarkerInfo& info, double size, QGraphicsItem* parent)
{
	if (info.index % 2 == 0) {
		// Even: + sign
		QPainterPath path;
		path.moveTo(info.center.x() - info.dx * size / 2, info.center.y() - info.dy * size / 2);
		path.lineTo(info.center.x() + info.dx * size / 2, info.center.y() + info.dy * size / 2);
		path.closeSubpath();

		path.moveTo(info.center.x() + info.dy * size / 2, info.center.y() - info.dx * size / 2);
		path.lineTo(info.center.x() - info.dy * size / 2, info.center.y() + info.dx * size / 2);
		return new QGraphicsPathItem(path, parent);
	}
	else {
		// We in principle want to use QGraphicsLineItem, but this is not a subclass of QAbstractGraphicsShapeItem,
		// thus we use PathItem as alternative...
		QPainterPath path;

		path.moveTo(info.center.x() + info.dy * size / 2, info.center.y() - info.dx * size / 2);
		path.lineTo(info.center.x() - info.dy * size / 2, info.center.y() + info.dx * size / 2);
		return new QGraphicsPathItem(path, parent);
	}
}
