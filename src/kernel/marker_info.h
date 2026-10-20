#pragma once


/**
 * The information for drawing one marker.
 * The dx, dy marks the direction of the line section to which the marker belongs, and should be NORMALIZED.
 */
struct MarkerInfo {
	QPointF center;
	double dx, dy;

	MarkerInfo(const QPointF& center, double dx, double dy) :
		center(center), dx(dx), dy(dy)
	{
	}
};