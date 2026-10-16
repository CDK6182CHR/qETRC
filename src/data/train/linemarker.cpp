#include "linemarker.h"

QJsonObject LineMarkerStyle::toJson() const
{
	QJsonObject obj;
	obj["type"] = static_cast<int>(m_type);
	obj["marker_size"] = m_marker_size;
	QJsonObject penObj;
	penObj["color"] = m_pen.color().name();
	penObj["width"] = m_pen.widthF();
	penObj["style"] = static_cast<int>(m_pen.style());
	obj["pen"] = penObj;
	return obj;
}

LineMarkerStyle LineMarkerStyle::fromJson(const QJsonObject& obj)
{
	LineMarkerType type = static_cast<LineMarkerType>(obj["type"].toInt());
	double marker_size = obj["marker_size"].toDouble();
	QJsonObject penObj = obj["pen"].toObject();
	QColor color(penObj["color"].toString());
	double width = penObj["width"].toDouble();
	Qt::PenStyle style = static_cast<Qt::PenStyle>(penObj["style"].toInt());
	QPen pen(color, width, style);
	return LineMarkerStyle(type, pen, marker_size);
}
