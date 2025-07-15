#include "diagramoptions.h"

DiagramOptions::DiagramOptions(const QJsonObject& obj)
{
	fromJson(obj);
}

bool DiagramOptions::fromJson(const QJsonObject& obj)
{
	if (obj.empty())
		return false;
	max_passed_stations = obj["max_passed_stations"].toInt(3);
	period_hours = obj["period_hours"].toInt(24);
	return true;
}

QJsonObject DiagramOptions::toJson() const
{
	return QJsonObject{
		{"max_passed_stations", max_passed_stations},
		{"period_hours", period_hours},
	};
}
