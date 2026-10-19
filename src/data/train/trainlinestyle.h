#pragma once

#include "linemarker.h"

/**
 * 2026.10.16: experimental.
 * Storing the train line style information.
 * In previous version, this is stored only by the QPen.
 * It seems that using a new class for describing the train line style as a whole entity is more reasonable.
 */
class TrainLineStyle {
	QPen m_line_pen;
	LineMarkerStyle m_marker;

public:
	TrainLineStyle(QPen pen, LineMarkerStyle maker):
		m_line_pen(std::move(pen)), m_marker(std::move(maker)) {
	}

	auto& line_pen()const { return m_line_pen; }
	auto& line_pen() { return m_line_pen; }
	auto& marker()const { return m_marker; }
	auto& marker() { return m_marker; }

	bool hasMarker()const;

	void setMarker(LineMarkerStyle marker) { m_marker = std::move(marker); }

	bool operator==(const TrainLineStyle& rhs)const;
};
