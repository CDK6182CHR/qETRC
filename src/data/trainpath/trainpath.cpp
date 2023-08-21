#include "trainpath.h"
#include <QJsonArray>
#include <cassert>

#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "log/IssueManager.h"

const StationName& TrainPath::endStation() const
{
	assert(!empty());
	return _segments.back().end_station;
}

void TrainPath::fromJson(const QJsonObject& obj, const RailCategory& cat, TrainCollection& coll)
{
	_name = obj.value("name").toString();
	_note = obj.value("note").toString();
	_start_station = obj.value("start_station").toString();   // implicit convert
	_valid = obj.value("valid").toBool();

	_segments.clear();
	const auto& segval = obj.value("segments").toArray();
	for (const auto& s : segval) {
		_segments.emplace_back(s.toObject(), cat);
	}

	const auto& arrtrains = obj.value("trains").toArray();
	for (const auto& a : arrtrains) {
		const auto& trainName = a.toString();
		auto train = coll.findFullName(trainName);
		if (!train) [[unlikely]] {
			qCritical() << "Train not found " << train->trainName().full()
				<< ", required by path " << _name;
		}
		else {
			addTrain(train);
		}
	}

	checkIsValid();
}

QJsonObject TrainPath::toJson() const
{
	QJsonArray arr;
	for (const auto& seg : _segments) {
		arr.append(seg.toJson());
	}
	QJsonArray arrtrain;
	for (const auto& t : _trains) {
		if (t.expired()) [[unlikely]] {
			qCritical() << "Invalid train encountered in path";
		}
		else {
			arrtrain.append(t.lock()->trainName().full());
		}
	}
	return QJsonObject{
		{"name", _name},
		{"note", _note},
		{"start_station", _start_station.toSingleLiteral()},
		{"valid",_valid},
		{"segments", arr},
		{"trains", arrtrain}
	};
}

void TrainPath::swapDataWith(TrainPath& other)
{
	std::swap(_name, other._name);
	std::swap(_note, other._note);
	std::swap(_start_station, other._start_station);
	std::swap(_valid, other._valid);
	std::swap(_segments, other._segments);
}

bool TrainPath::checkIsValid()
{
	do {
		if (empty()) {
			_valid = true;
			break;
		}
		
		_valid = true;
		StationName start_st = _start_station;
		for (auto& seg : _segments) {
			if (!seg.checkIsValid(start_st)) {
				_valid = false;
				break;
			}
			start_st = seg.end_station;
		}

	} while (false);
	return _valid;
}

void TrainPath::updateRailways(RailCategory& cat)
{
	for (auto& seg : _segments) {
		seg.updateRailway(cat);
	}
}

void TrainPath::addTrain(std::shared_ptr<Train> train)
{
	_trains.emplace_back(train);
	train->paths().emplace_back(this);
}

void TrainPath::removeTrainFromBack(std::shared_ptr<Train> train)
{
	for (auto itr = _trains.rbegin(); itr != _trains.rend(); ++itr) {
		if (itr->lock() == train) {
			_trains.erase(std::prev(itr.base()));  
			break;
		}
	}

	for (auto itr = train->paths().rbegin(); itr != train->paths().rend(); ++itr) {
		if (*itr == this) {
			train->paths().erase(std::prev(itr.base()));
			break;
		}
	}
}

void TrainPath::insertTrainWithIndex(std::shared_ptr<Train> train, int train_index, int path_index_in_train)
{
	_trains.insert(_trains.begin() + train_index, train);
	train->paths().insert(train->paths().begin() + path_index_in_train, this);
}

void TrainPath::removeTrainWithIndex(std::shared_ptr<Train> train, int train_index, int path_index_in_train)
{
	const auto& t = _trains.at(train_index).lock();
	if (t != train) [[unlikely]] {
		qCritical() << "train index not matched " << train->trainName().full();
		return;
	}
	auto* p = train->paths().at(path_index_in_train);
	if (p != this) [[unlikely]] {
		qCritical() << "path index not matched " << name();
		return;
	}

	_trains.erase(_trains.begin() + train_index);

	train->paths().erase(train->paths().begin() + path_index_in_train);
}

int TrainPath::getTrainIndex(std::shared_ptr<Train> train) const
{
	for (int i = 0; i < _trains.size(); i++) {
		auto t = _trains.at(i).lock();
		if (t == train) {
			return i;
		}
	}
	return -1;
}

bool TrainPath::containsRailway(const Railway* rail) const
{
	for (const auto& seg : _segments) {
		auto r = seg.railway.lock();
		if (r.get() == rail) {
			return true;
		}
	}
	return false;
}
