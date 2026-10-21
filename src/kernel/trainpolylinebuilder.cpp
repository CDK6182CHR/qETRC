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
	//qDebug() << "cur_length " << cur_length << ", acc: " << m_accRunningLength;
	if (!m_style.hasMarker()) {
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

	// The marker size is dependent on the index. This is only a reference.
	double marker_occ_max = m_markerBuilder->max_occupied_length(m_style.marker().marker_size());
	if (cur_length < marker_occ_max) {
		// the total length is not enough for even one marker. Thus just ignore the marker.
		return lineTo(p);
	}

	// 2026.10.20  we determine the positions of the markers in each section (call of this function) separately.
	// the markers are put in the center when possible.

	int marker_interval_count = static_cast<int>(std::floor((cur_length - marker_occ_max) / m_config.marker_distance));
	if (marker_interval_count > 0 && cur_length - marker_interval_count * m_config.marker_distance < m_config.marker_distance / 2) {
		// Avoid two markers that are too close the the ending point, which seems to be very strange.
		// the threshold, m_config.marker_distance / 2, is adjustable.
		marker_interval_count -= 1;
	}

	double marker_dist = (cur_length - marker_interval_count * m_config.marker_distance) / 2;
	QPointF cur_sec_start = m_mlpath.currentPosition();

	double dx = p.x() - cur_sec_start.x();
	double dy = p.y() - cur_sec_start.y();

	double dxnorm = dx / cur_length;
	double dynorm = dy / cur_length;

	/**
	 * Returns the position (QPointF) with required distance from the starting point of this section
	 * (i.e., the position when calling runLineTo).
	 */
	auto get_position_with_distance = [this, &p, &cur_sec_start, cur_length, dx, dy](double d) -> QPointF {
		double scale = d / cur_length;
		return QPointF(cur_sec_start.x() + dx * scale, cur_sec_start.y() + dy * scale);
		};

	// Each iteration processes the LEFT marker for the interval. We have (marker_interval_count + 1) markers.
	for (int idx = 0; idx <= marker_interval_count; idx++) {
		double marker_occ = m_markerBuilder->occupied_length(m_style.marker().marker_size(), m_numMarkers);
		m_markerInfos.emplace_back(get_position_with_distance(marker_dist), dxnorm, dynorm, m_numMarkers);
		m_mlpath.lineTo(get_position_with_distance(marker_dist - marker_occ / 2));
		m_mlpath.moveTo(get_position_with_distance(marker_dist + marker_occ / 2));
		m_numMarkers++;

		marker_dist += m_config.marker_distance;
	}
	// The final section
	m_mlpath.lineTo(p);
}

void TrainPolyLineBuilder::generalLineTo(const QPointF& p, bool is_running)
{
	if (is_running)
		return runLineTo(p);
	else
		return lineTo(p);
}

