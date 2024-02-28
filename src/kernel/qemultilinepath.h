#pragma once
#include <QPainterPath>

class QPointF;

/**
 * 2024.02.28 A wrapper of QPainterPath, in which each line is treated as a subpath.
 * Thus, the contains() should be automatically right for trainlines.
 */
class QEMultiLinePath {
	QPainterPath _path;

	// this variable logs whether the path is to started a new subpath.
	// if false, moveTo() is first called before lineTo().
	bool _isNewPath = false;

public:
	QEMultiLinePath() = default;
	QEMultiLinePath(const QPointF& p);

	const auto& path()const { return _path; }

	void moveTo(const QPointF& p);
	void moveTo(qreal x, qreal y);

	void lineTo(const QPointF& p);
	void lineTo(qreal x, qreal y);

	void addPolygon(const QPolygonF& p);

	void closeSubPath();

	auto currentPosition()const { return _path.currentPosition(); }
};