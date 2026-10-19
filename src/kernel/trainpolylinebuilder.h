#pragma once

#include <QVector>
#include <QPointF>

#include "qemultilinepath.h"
#include "marker_builders.h"
#include "data/train/trainlinestyle.h"


struct Config;

/**
 * 2026.10.19: The builder for the train polyline (prevously the QPainterPath for the TrainItem::pathItem).
 * The main task of this class is to build the QPainterPath for the path, as well as all information for the 
 * markers. It is NOT the task of this class to build the graphisItem.
 */
class TrainPolyLineBuilder
{
	const Config& m_config;
	TrainLineStyle m_style;
	QEMultiLinePath m_mlpath;
	std::unique_ptr<AbstractMarkerBuilder> m_markerBuilder;
	QVector<QPointF> m_markerCenters;

	/**
	 * Accumulated running line lengths for determining the markers.
	 */
	double m_accRunningLength = 0;

public:
	TrainPolyLineBuilder(const Config& config, TrainLineStyle style);

	auto& path()const { return m_mlpath; }
	auto& path() { return m_mlpath; }

	/**
	 * The simple moveTo() and lineTo() functions just pass the call directly to the mlpath.
	 * We use runLineTo() function to process the requirement for the running-lines, that is, the lines not in
	 * one station. Markers are required for this case.
	 */
	void moveTo(const QPointF& p);
	void moveTo(qreal x, qreal y);

	void lineTo(const QPointF& p);
	void lineTo(qreal x, qreal y);

	/**
	 * This is the core function of this class. Wrapper for drawing a part of train running line (lean line).
	 * We add markers in a required interval.
	 */
	void runLineTo(const QPointF& p);
};
