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

bool TrainPathCollection::isValidNewPathName(const QString& name, TrainPath* ignore) const
{
	if (name.isEmpty()) return false;
	for (const auto& p : _paths) {
		if (p->name() == name && p.get()!=ignore)
			return false;
	}
	return true;
}

QString TrainPathCollection::validNewPathName(const QString& prefix) const
{
	for (int i = 0;; i++) {
		QString name = prefix;
		if (i) {
			name += QString::number(i);
		}
		if (isValidNewPathName(name))
			return name;
	}
}

int TrainPathCollection::pathIndex(const TrainPath* p) const
{
	for (int i = 0; i < _paths.size(); i++) {
		if (_paths.at(i).get() == p) {
			return i;
		}
	}
	return -1;
}

void TrainPathCollection::checkValidAll(RailCategory& railcat)
{
	for (auto& path : _paths) {
		path->updateRailways(railcat);
		path->checkIsValid();
	}
}
