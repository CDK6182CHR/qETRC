#pragma once

#include <memory>
#include <QString>
#include <QJsonObject>

class Ruler;
struct TrainPathSeg;

/**
 * A segment of the ruler for a TrainPath, corresponding to the TrainPathSeg.
 * The segment is valid iff the `ruler` holds a valid object.
 */
struct PathRulerSeg {
	QString ruler_name;
	std::weak_ptr<Ruler> ruler;

	QString getRulerName()const;

	explicit PathRulerSeg(std::shared_ptr<Ruler> ruler_);
	explicit PathRulerSeg(const QString& name);
	PathRulerSeg(const QJsonObject& obj, const TrainPathSeg& pathseg);

	void fromJson(const QJsonObject& obj, const TrainPathSeg& pathseg);
	QJsonObject toJson()const;

	bool checkIsValid()const;
};