#include "traincollection.h"
#include "routing.h"

#include <QJsonArray>

TrainCollection::TrainCollection(const QJsonObject& obj, const TypeManager& defaultManager)
{
	fromJson(obj, defaultManager);
}

void TrainCollection::fromJson(const QJsonObject& obj, const TypeManager& defaultManager)
{
	_trains.clear();
	_manager.readForDiagram(obj.value("config").toObject(), defaultManager);
	_routings.clear();

	//Train类型的正确设置依赖于TypeManager的正确初始化
	const QJsonArray& artrains = obj.value("trains").toArray();
	for (const auto& p : artrains) {
		_trains.append(std::make_shared<Train>(p.toObject(), _manager));
	}
	
	resetMapInfo();

	//注意Routing的读取依赖车次查找
	const QJsonArray& arrouting = obj.value("circuits").toArray();
	for (auto p = arrouting.begin(); p != arrouting.end(); ++p) {
		auto r = std::make_shared<Routing>(*this);
		r->fromJson(p->toObject());
		_routings.append(r);
	}
    
}

QJsonObject TrainCollection::toJson() const
{
	QJsonArray artrains;
	for (const auto& p : _trains) {
		artrains.append(p->toJson());
	}
	QJsonArray arrouting;
	for (auto p : _routings) {
		arrouting.append(p->toJson());
	}
	return QJsonObject{
		{"trains",artrains},
		{"circuits",arrouting}
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
	return fullNameMap.contains(name.full());
}

std::shared_ptr<Train> TrainCollection::findFullName(const QString& name)
{
	return fullNameMap.value(name);
}

std::shared_ptr<Train> TrainCollection::findFullName(const TrainName& name)
{
	return findFullName(name.full());
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
	fullNameMap.insert(t->trainName().full(), t);
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
	fullNameMap.remove(t->trainName().full());
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

