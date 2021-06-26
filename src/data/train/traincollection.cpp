#include "traincollection.h"

#include <QJsonArray>

TrainCollection::TrainCollection(const QJsonObject& obj)
{
	fromJson(obj);
}

void TrainCollection::fromJson(const QJsonObject& obj)
{
	_trains.clear();
	const QJsonArray& artrains = obj.value("trains").toArray();
	for (const auto& p : artrains) {
		_trains.append(std::make_shared<Train>(p.toObject(), _manager));
	}
	//todo 交路 类型系统
    resetMapInfo();
}

QJsonObject TrainCollection::toJson() const
{
	QJsonArray artrains;
	for (const auto& p : _trains) {
		artrains.append(p->toJson());
	}
	//todo 交路 类型系统
	return QJsonObject{
		{"trains",artrains}
	};
}

void TrainCollection::appendTrain(std::shared_ptr<Train> train)
{
	_trains.append(train);
	addMapInfo(train);
}

void TrainCollection::removeTrain(std::shared_ptr<Train> train)
{
	removeMapInfo(train);
	_trains.removeOne(train);
}

bool TrainCollection::trainNameExisted(const TrainName& name) const
{
	return fullNameMap.contains(name);
}

std::shared_ptr<Train> TrainCollection::findFullName(const TrainName& name)
{
	return fullNameMap.value(name);
}

QList<std::shared_ptr<Train>> TrainCollection::findAllSingleName(const QString& name)
{
	return singleNameMap.value(name);   //copy construct
}

std::shared_ptr<Train> TrainCollection::findFirstSingleName(const QString& name)
{
	return singleNameMap.value(name).first();
}

void TrainCollection::addMapInfo(const std::shared_ptr<Train>& t)
{
	fullNameMap.insert(t->trainName(), t);
	const TrainName& n = t->trainName();
	if (!n.full().isEmpty()) {
		singleNameMap[n.full()].append(t);
	}
	if (!n.down().isEmpty()) {
		singleNameMap[n.down()].append(t);
	}
	if (!n.up().isEmpty()) {
		singleNameMap[n.up()].append(t);
	}
}

void TrainCollection::removeMapInfo(std::shared_ptr<Train> t)
{
	fullNameMap.remove(t->trainName());
	const auto& n = t->trainName();
	if (!n.full().isEmpty()) {
		singleNameMap[n.full()].removeAll(t);
	}
	if (!n.down().isEmpty()) {
		singleNameMap[n.down()].removeAll(t);
	}
	if (!n.up().isEmpty()) {
		singleNameMap[n.up()].removeAll(t);
	}
}

void TrainCollection::resetMapInfo()
{
	fullNameMap.clear();
	singleNameMap.clear();
	for (const auto& p : _trains) {
		addMapInfo(p);
	}
}

