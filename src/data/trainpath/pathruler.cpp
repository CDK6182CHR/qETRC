#include "pathruler.h"

#include <QJsonArray>

#include "trainpath.h"

PathRuler::PathRuler(std::weak_ptr<TrainPath> path, const QString& name, std::vector<PathRulerSeg> segments):
	m_path(std::move(path)), m_name(name), m_segments(std::move(segments)), m_valid(true)
{
}

PathRuler::PathRuler(const QJsonObject& obj, const TrainPath& path, const RailCategory& cat)
{
	fromJson(obj, path, cat);
}

PathRulerConstIterator PathRuler::cbegin() const
{
	return PathRulerConstIterator::makeBegin(this);
}

PathRulerMutableIterator PathRuler::begin()
{
	return PathRulerMutableIterator::makeBegin(this);
}

PathRulerConstIterator PathRuler::cend() const
{
	return PathRulerConstIterator::makeEnd(this);
}

PathRulerMutableIterator PathRuler::end()
{
	return PathRulerMutableIterator::makeEnd(this);
}

QJsonObject PathRuler::toJson() const
{
	QJsonArray arr{};

	for (const auto& seg : m_segments) {
		arr.append(seg.toJson());
	}

	return QJsonObject{
		{"name", m_name},
		{"segments", arr},
	};
}

void PathRuler::fromJson(const QJsonObject& obj, const TrainPath& path, const RailCategory& cat)
{
	m_name = obj.value("name").toString();
	const auto& arr = obj.value("segments").toArray();

	for (int i = 0; i < arr.size(); i++) {
		m_segments.emplace_back(arr.at(i).toObject(), path.segments().at(i));
	}

	checkIsValid();
}

void PathRuler::checkIsValid()
{
	m_valid = true;
	for (auto& seg : m_segments) {
		if (!seg.checkIsValid()) {
			m_valid = false;
		}
	}
}
