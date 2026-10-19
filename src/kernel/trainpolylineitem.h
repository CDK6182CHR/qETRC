#pragma once

#include <QVector>

class QGraphicsPathItem;
class QGraphicsItem;

/**
 * 2026.10.19  Experimental: train polyline item for train-line, part of TrainItem.
 * Previously we just used a PathItem.
 * ---------
 * This class solves as the item used as a part of TrainItem. But we still need another class, a builder,
 * to take the place of the QPainterPath.
 */
class TrainPolyLineItem 
{
	QGraphicsPathItem* m_pathItem;
	QGraphicsPathItem* m_expandItem;
	QVector<QGraphicsItem*> m_markers;
};