#include "linemarker.h"

#include "data/common/qesystem.h"

std::array<QString, static_cast<int>(LineMarkerType::MAX)> line_marker_type_names{
	QObject::tr("无标记"),
	QObject::tr("方框"),
	QObject::tr("双杠"),
	QObject::tr("三角"),
	QObject::tr("圆圈"),
	QObject::tr("X"),
	QObject::tr("+|交替"),
};

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
	if (pen.style() == Qt::CustomDashLine) {
		pen.setDashPattern(SystemJson::get().dash_line_pattern());
	}
	return LineMarkerStyle(type, pen, marker_size);
}
