#include "trainpolylinebuilder.h"

#include <cassert>
#include "data/diagram/config.h"
#include "util/utilfunc.h"

TrainPolyLineBuilder::TrainPolyLineBuilder(const Config& config, TrainLineStyle style):
	m_config(config), m_style(std::move(style)), m_markerBuilder(AbstractMarkerBuilder::creeate(style.marker().type()))
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

void TrainPolyLineBuilder::runLineTo(const QPointF& p)
{
	double cur_length = qeutil::distanceBetween(m_mlpath.currentPosition(), p);
	if (!m_style.hasMarker()) {
		m_accRunningLength += cur_length;
		return lineTo(p);
	}

	if (m_config.marker_distance <= 0) {
		qWarning() << "TrainPolyLineBuilder::runLineTo: invalid marker_distance " << m_config.marker_distance;
		return lineTo(p);
	}

	if (cur_length == 0) {
		qWarning() << "TrainPolyLineBuilder::runLineTo: zero distance!";
		return lineTo(p);
	}

	// 50  -> 0
	// 150 -> 1  draw one

	double marker_occ = m_markerBuilder->occupied_length(m_style.marker().marker_size());
	int cur_idx = static_cast<int>(std::floor(m_accRunningLength / m_config.marker_distance));
	int expected_idx = static_cast<int>(std::floor((m_accRunningLength + cur_length) / m_config.marker_distance));
	double next_length = m_accRunningLength + cur_length;

	QPointF cur_sec_start = m_mlpath.currentPosition();

	/**
	 * Returns the position (QPointF) with required distance from the starting point of this section
	 * (i.e., the position when calling runLineTo).
	 */
	auto get_position_with_distance = [this, &p, &cur_sec_start, cur_length](double d) -> QPointF {
		double dx = p.x() - cur_sec_start.x();
		double dy = p.y() - cur_sec_start.y();
		double scale = d / cur_length;
		return QPointF(cur_sec_start.x() + dx * scale, cur_sec_start.y() + dy * scale);
		};

	double marker_center_length = cur_idx * m_config.marker_distance;
	for (int idx = cur_idx; idx <= expected_idx; idx++, marker_center_length += m_config.marker_distance) {
		if (idx == cur_idx && m_accRunningLength - marker_center_length <= marker_occ / 2) {
			// Skip the first marker because we do not have enough space
			continue;
		}
		if (idx == expected_idx && next_length - marker_center_length <= marker_occ / 2) {
			// Skip the final marker, because we do not have enough space.
			// Currently we choose to just omit it (without drawing it later)
			continue;
		}

		// Now: we have a marker at current position
		double center_dist_to_start = marker_center_length + m_config.marker_distance - m_accRunningLength;
		assert(center_dist_to_start >= 0);

		// Add the center position
		m_markerCenters.push_back(get_position_with_distance(center_dist_to_start));

		// Make the path (currently we choose to draw only the lines for showing)
		m_mlpath.lineTo(get_position_with_distance(center_dist_to_start - marker_occ / 2));
		m_mlpath.moveTo(get_position_with_distance(center_dist_to_start + marker_occ / 2));
	}

	// The final section
	m_mlpath.lineTo(p);

	// Finalizing: remember the total length.
	m_accRunningLength += cur_length;
}

void TrainPolyLineBuilder::generalLineTo(const QPointF& p, bool is_running)
{
	if (is_running)
		return runLineTo(p);
	else
		return lineTo(p);
}

