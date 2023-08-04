#include "trainpath.h"
#include <QJsonArray>

void TrainPath::fromJson(const QJsonObject& obj, const RailCategory& cat)
{
	_name = obj.value("name").toString();
	_note = obj.value("note").toString();
	_start_station = obj.value("start_station").toString();   // implicit convert

	_segments.clear();
	const auto& segval = obj.value("segments").toArray();
	for (const auto& s : segval) {
		_segments.emplace_back(s.toObject(), cat);
	}
}

QJsonObject TrainPath::toJson() const
{
	QJsonArray arr;
	for (const auto& seg : _segments) {
		arr.append(seg.toJson());
	}
	return QJsonObject{
		{"name", _name},
		{"note", _note},
		{"start_station", _start_station.toSingleLiteral()},
		{"segments", arr}
	};
}
