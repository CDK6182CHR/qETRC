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
	foreach (const auto& p , artrains) {
		_trains.append(std::make_shared<Train>(p.toObject(), _manager));
	}
	
	resetMapInfo();

	//注意Routing的读取依赖车次查找
	const QJsonArray& arrouting = obj.value("circuits").toArray();
	for (auto p = arrouting.cbegin(); p != arrouting.cend(); ++p) {
		auto r = std::make_shared<Routing>();
		r->fromJson(p->toObject(), *this);
		_routings.append(r);
	}
    
}

bool TrainCollection::fromJson(const QString& filename, const TypeManager& defaultManager)
{
	QFile file(filename);
	file.open(QFile::ReadOnly);
	if (!file.isOpen()) {
		qDebug() << "TrainCollection::fromJson: WARNING: Open file " << filename
			<< " failed." << Qt::endl;
		return false;
	}
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	fromJson(doc.object(), defaultManager);
	file.close();
	return true;
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

QJsonObject TrainCollection::toLocalJson(std::shared_ptr<Railway> rail, bool localOnly) const
{
	QJsonArray artrains;
	for (const auto& p : _trains) {
		if(!localOnly || p->adapterFor(*rail))
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

bool TrainCollection::trainNameIsValid(const TrainName& name, std::shared_ptr<Train> train) const
{
	if (name.full().isEmpty())
		return false;
	if (fullNameMap.contains(name.full()) && fullNameMap.value(name.full()) != train)
		return false;
	return true;
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

QList<std::shared_ptr<Train>> TrainCollection::multiSearchTrain(const QString& name)
{
	QList<std::shared_ptr<Train>> res;
	foreach (auto t , _trains) {
		if (t->trainName().contains(name))
			res.append(t);
	}
	return res;
}

void TrainCollection::clear(const TypeManager& defaultManager)
{
	_trains.clear();
	_routings.clear();
	fullNameMap.clear();
	singleNameMap.clear();
	_manager = defaultManager;
}

void TrainCollection::clearTrains()
{
	//这样操作是为了保证交路正确
	while (!_trains.empty())
		removeTrainAt(0);
}

std::shared_ptr<Train> TrainCollection::takeTrainAt(int i)
{
	auto t = _trains.takeAt(i);
	removeMapInfo(t);
	if (t->hasRouting()) {
		t->routingNode().value()->makeVirtual();
	}
	return t;
}

std::shared_ptr<Train> TrainCollection::takeLastTrain()
{
	auto t = _trains.takeLast();
	removeMapInfo(t);
	if (t->hasRouting()) {
		t->routingNode().value()->makeVirtual();
	}
	return t;
}

void TrainCollection::removeTrainAt(int i)
{
	auto t = _trains.takeAt(i);
	removeMapInfo(t);
	if (t->hasRouting()) {
		auto it = t->routingNode();
		it.value()->makeVirtual();
		it.reset();
	}
}

void TrainCollection::insertTrainForUndo(int i, std::shared_ptr<Train> train)
{
	_trains.insert(i, train);
	if (train->hasRouting()) {
		train->routingNode().value()->setTrain(train);
	}
	addMapInfo(train);
}

void TrainCollection::removeUnboundTrains()
{
	//倒着遍历
	for (int i = _trains.size() - 1; i >= 0; i--) {
		auto t = _trains.at(i);
		if (t->adapters().empty()) {
			//删除  调用函数来正确处理交路
			removeTrainAt(i);
		}
	}
}

bool TrainCollection::routingNameExisted(const QString& name, 
	std::shared_ptr<Routing> ignore) const
{
	for (auto p : _routings)
		if (p->name() == name && p!=ignore)
			return true;
	return false;
}

QString TrainCollection::validRoutingName(const QString& prefix)
{
	for (int i = 0;; i++) {
		QString res = prefix;
		if (i)res += "_" + QString::number(i);
		if (!routingNameExisted(res))
			return res;
	}
}

int TrainCollection::getTrainIndex(std::shared_ptr<Train> train) const
{
	auto p = std::find(_trains.begin(), _trains.end(), train);
	if (p == _trains.end())
		return -1;
	else
        return std::distance(_trains.begin(), p);
}

int TrainCollection::getRoutingIndex(std::shared_ptr<const Routing> routing) const
{
    for(int i=0;i<_routings.size();i++){
        if(_routings.at(i) == routing)
            return i;
    }
    return -1;
}

TrainName TrainCollection::validTrainFullName(const QString& prefix) const
{
	for (int i = 0;; i++) {
		QString name = prefix;
		if (i)
			name += QString::number(i);
		TrainName tn(name);
		if (!trainNameExisted(tn))
			return tn;
	}
}

void TrainCollection::invalidateAllTempData()
{
	for (auto p : _trains)
		p->invalidateTempData();
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
	if (!t->type()) {
		t->setType(_manager.fromRegex(t->trainName()));
	}
	++_typeCount[t->type()];   //利用默认为0的特性
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
	--_typeCount[t->type()];
}

void TrainCollection::resetMapInfo()
{
	fullNameMap.clear();
	singleNameMap.clear();
	for (const auto& p : _trains) {
		addMapInfo(p);
	}
}

