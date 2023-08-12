#include "trainpathcollection.h"

void TrainPathCollection::fromJson(const QJsonArray& obj, const RailCategory& cat)
{
	clear();
	for (const auto& a : obj) {
		_paths.emplace_back(std::make_unique<TrainPath>(a.toObject(), cat));
	}
}

QJsonArray TrainPathCollection::toJson() const
{
	QJsonArray arr{};
	for (const auto& p : _paths) {
		arr.append(p->toJson());
	}
	return arr;
}

void TrainPathCollection::clear()
{
	_paths.clear();
}
