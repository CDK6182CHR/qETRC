#pragma once

#include <QGraphicsItem>
#include <QVector>

class QGraphicsPathItem;
class QGraphicsItem;
class TrainPolyLineBuilder;
class TrainLineStyle;

/**
 * 2026.10.19  Experimental: train polyline item for train-line, part of TrainItem.
 * Previously we just used a PathItem.
 * ---------
 * This class solves as the item used as a part of TrainItem. But we still need another class, a builder,
 * to take the place of the QPainterPath.
 */
class TrainPolyLineItem: public QGraphicsItem
{
	QGraphicsPathItem* m_pathItem = nullptr;
	QGraphicsPathItem* m_expandItem = nullptr;
	QVector<QAbstractGraphicsShapeItem*> m_markers;

public:
	TrainPolyLineItem(QGraphicsItem* parent);

	/**
	 * Core function of this class, must be called before any other usages.
	 * Build the item from the builder (containing the information of path etc).
	 */
	void buildItem(TrainPolyLineBuilder& builder);

	auto* pathItem() { return m_pathItem; }
	auto* expandItem() { return m_expandItem; }
	auto& markers()const { return m_markers; }

	virtual QRectF boundingRect()const override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
		QWidget* widget = nullptr)override;

	void highlight(const TrainLineStyle& style);
	void unhighlight(const TrainLineStyle& style);
};