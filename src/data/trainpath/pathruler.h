#pragma once

#include "data/rail/abstractruler.h"

#include <vector>
#include <memory>
#include <QString>
#include <QJsonObject>

#include "pathrulerseg.h"
#include "pathruleriter.h"

class Ruler;
class TrainPath;
class RailCategory;

/**
 * The ruler that attached with a TrainPath. Similar to the `Ruler` class, 
 * sharing the same set of abstract API set
 */
class PathRuler : public AbstractRuler
{
	std::weak_ptr<TrainPath> m_path;
	QString m_name;
	std::vector<PathRulerSeg> m_segments;
	bool m_valid;

public:

	/**
	 * Constructor provided for GUI construction. All data must be valid at this construction stage.
	 */
	PathRuler(std::weak_ptr<TrainPath> path, const QString& name, std::vector<PathRulerSeg> segments);

	PathRuler(const QJsonObject& obj, const TrainPath& path, const RailCategory& cat);

	/**
	 * Ctor for empty ruler.
	 */
	explicit PathRuler(std::weak_ptr<TrainPath> path);

	auto& segments()const { return m_segments; }
	auto& segmentsRef() { return m_segments; }

	auto& path()const { return m_path; }

	bool valid()const { return m_valid; }
	auto& name()const { return m_name; }

	PathRulerConstIterator cbegin()const;
	PathRulerMutableIterator begin();

	PathRulerConstIterator cend()const;
	PathRulerMutableIterator end();

	PathRulerConstIterator begin()const {
		return cbegin();
	}
	PathRulerConstIterator end()const {
		return cend();
	}

	QJsonObject toJson()const;
	void fromJson(const QJsonObject& obj, const TrainPath& path, const RailCategory& cat);

	/**
	 * Check the validity of current PathRuler object.
	 */
	void checkIsValid();
};
