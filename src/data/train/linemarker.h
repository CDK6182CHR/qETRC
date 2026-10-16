#pragma once

#include <QPen>
#include <QJsonObject>

/**
 * 2026.10.16: Train marker in the line.
 * Experimental for new train line styles staying in line with the standard CR styles.
 */
enum class LineMarkerType {
	None = 0,
	Square,
	DoubleLine,
	Triangle,
	Circle,
	Cross,
};

/**
 * The data class holding the line marker style.
 * This is currently used along with the QPen for the train lines.
 * In current version, the marker always has the same pen (except color) with the train line. But we choose to
 * store the pen for future flexibility and convience.
 * The size of marker is interpreted by each of the possible types.
 */
class LineMarkerStyle {
	LineMarkerType m_type;
	QPen m_pen;
	double m_marker_size;

public:
	LineMarkerStyle(LineMarkerType type, QPen pen, double marker_size) : 
		m_type(type), m_pen(std::move(pen)), m_marker_size(marker_size) {}

	static LineMarkerStyle no_marker() { return LineMarkerStyle(LineMarkerType::None, QPen(), 0.0); }

	auto type()const { return m_type; }
	auto& pen()const { return m_pen; }
	auto& pen() { return m_pen; }
	auto marker_size()const { return m_marker_size; }

	bool operator==(const LineMarkerStyle& rhs)const {
		return m_type == rhs.m_type && m_pen == rhs.m_pen && m_marker_size == rhs.m_marker_size;
	}

	QJsonObject toJson()const;

	static LineMarkerStyle fromJson(const QJsonObject& obj);
};
