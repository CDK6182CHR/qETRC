#include "trainpolylinebuilder.h"

#include "data/diagram/config.h"

TrainPolyLineBuilder::TrainPolyLineBuilder(const Config& config, TrainLineStyle style):
	m_config(config), m_style(std::move(style))
{
}

void TrainPolyLineBuilder::moveTo(const QPointF& p)
{
	m_mlpath.moveTo(p);
}

void TrainPolyLineBuilder::moveTo(qreal x, qreal y)
{
	m_mlpath.moveTo(x, y);
}

void TrainPolyLineBuilder::lineTo(const QPointF& p)
{
	m_mlpath.lineTo(p);
}

void TrainPolyLineBuilder::lineTo(qreal x, qreal y)
{
	m_mlpath.lineTo(x, y);
}

