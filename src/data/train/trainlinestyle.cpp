#include "trainlinestyle.h"

bool TrainLineStyle::operator==(const TrainLineStyle& rhs) const
{
	return m_line_pen == rhs.m_line_pen && m_marker == rhs.m_marker;
}
