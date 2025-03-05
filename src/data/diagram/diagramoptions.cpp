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
	return true;
}

QJsonObject DiagramOptions::toJson() const
{
	return QJsonObject{
		{"max_passed_stations", max_passed_stations}
	};
}
