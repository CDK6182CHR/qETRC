#pragma once

#include <QJsonObject>

/**
 * Global options applied for a diagram. Previously mixed in Config class.
 * Currentlly, only configured at Diagram level, not in DiagramPage level, nor global level.
 */
struct DiagramOptions 
{

	/**
	 * For bind train to railways automatically. Split the trainline if the passed station count
	 * is greater than this value.
	 */
	int max_passed_stations = 3;

	/**
	 * Period (hours) for the train diagrams. 
	 * Default 24, for a full day.
	 */
	int period_hours = 24;
	
	DiagramOptions() = default;
	DiagramOptions(const QJsonObject& obj);
	
	/**
	 * Load from JSON. Typically the object should be the object for diagram_options.
	 * However, for backward compatibility, this could also be the object for config, if the former is not found.
	 * Returns false if current object is empty.
	 */
	bool fromJson(const QJsonObject& obj);
	QJsonObject toJson()const;
};
