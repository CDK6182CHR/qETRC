#include "traintype.h"

QPen TypeManager::defaultPen(QColor(0, 128, 0), 1.0, Qt::SolidLine);
std::shared_ptr<TrainType> TypeManager::defaultType = 
	std::make_shared<TrainType>(QObject::tr("其他"), TypeManager::defaultPen);


TypeManager& TypeManager::operator=(const TypeManager& another)
{
	_types.clear();
	_regs.clear();
	for (auto p=another._types.begin();p!=another._types.end();++p) {
		_types.insert(p.key(), std::make_shared<TrainType>(*(p.value())));
	}
	for (const auto& p : another._regs) {
		_regs.append(qMakePair(p.first, findOrCreate(p.second->name())));
	}
	return *this;
}

std::shared_ptr<TrainType> TypeManager::addType(const QString& name, const QPen& pen)
{
	auto t = std::make_shared<TrainType>(name, pen);
	_types.insert(name, t);
	return t;
}

void TypeManager::appendRegex(const QRegExp& reg, const QString& name)
{
	_regs.append(qMakePair(reg, findOrCreate(name)));
}

void TypeManager::appendRegex(const QRegExp& reg, const QString& name, bool passenger)
{
	auto t = findOrCreate(name);
	t->setIsPassenger(passenger);
	_regs.append(qMakePair(reg, t));
}

std::shared_ptr<const TrainType> TypeManager::fromRegex(const TrainName& name) const
{
	for (const auto& p : _regs) {
		if (p.first.indexIn(name.full()) == 0)
			return p.second;
	}
	return defaultType;
}

std::shared_ptr<TrainType> TypeManager::findOrCreate(const QString& name)
{
	if (_types.contains(name))
		return _types.value(name);
	else {
		auto t = std::make_shared<TrainType>(name, defaultPen);
		_types.insert(name, t);
		return t;
	}
}

void TypeManager::initDefaultTypes()
{
	_types.insert(QObject::tr("其他"), defaultType);
	addType(QObject::tr("快速"), QPen(QColor(255, 0, 0), 1.5));
	addType(QObject::tr("特快"), QPen(QColor(0, 0, 255), 1.5));
	addType(QObject::tr("直达特快"), QPen(QColor(255, 0, 255), 1.5));
	addType(QObject::tr("动车组"), QPen(QColor(128, 64, 0), 1.5));
	addType(QObject::tr("动车"), QPen(QColor(128, 64, 0), 1.5));
	addType(QObject::tr("高速"), QPen(QColor(255, 0, 190), 1.5));
	addType(QObject::tr("城际"), QPen(QColor(255, 51, 204), 1.5));

	appendRegex(QRegExp(R"(G\d+)"), QObject::tr("高速"), true);
	appendRegex(QRegExp(R"(D\d+)"), QObject::tr("动车组"), true);
	appendRegex(QRegExp(R"(C\d+)"), QObject::tr("城际"), true);
	appendRegex(QRegExp(R"(Z\d+)"), QObject::tr("直达特快"), true);
	appendRegex(QRegExp(R"(T\d+)"), QObject::tr("特快"), true);
	appendRegex(QRegExp(R"(K\d+)"), QObject::tr("快速"), true);
	appendRegex(QRegExp(R"([1-5]\d{3}$)"), QObject::tr("普快"), true);
	appendRegex(QRegExp(R"([1-5]\d{3}\D)"), QObject::tr("普快"), true);
	appendRegex(QRegExp(R"(6\d{3}$)"), QObject::tr("普客"), true);
	appendRegex(QRegExp(R"(6\d{3}\D)"), QObject::tr("普客"), true);
	appendRegex(QRegExp(R"(7[1-5]\d{2}$)"), QObject::tr("普客"), true);
	appendRegex(QRegExp(R"(7[1-5]\d{2}\D)"), QObject::tr("普客"), true);
	appendRegex(QRegExp(R"(7\d{3}$)"), QObject::tr("通勤"), true);
	appendRegex(QRegExp(R"(7\d{3}\D)"), QObject::tr("通勤"), true);
	appendRegex(QRegExp(R"(8\d{3}$)"), QObject::tr("通勤"), true);
	appendRegex(QRegExp(R"(8\d{3}\D)"), QObject::tr("通勤"), true);
	appendRegex(QRegExp(R"(Y\d+)"), QObject::tr("旅游"), true);
	appendRegex(QRegExp(R"(57\d+)"), QObject::tr("路用"), true);
	appendRegex(QRegExp(R"(X1\d{2})"), QObject::tr("特快行包"), true);
	appendRegex(QRegExp(R"(DJ\d+)"), QObject::tr("动检"), true);
	appendRegex(QRegExp(R"(0[GDCZTKY]\d+)"), QObject::tr("客车底"), true);
	appendRegex(QRegExp(R"(L\d+)"), QObject::tr("临客"), true);
	appendRegex(QRegExp(R"(0\d{4})"), QObject::tr("客车底"), true);
	appendRegex(QRegExp(R"(X\d{3}\D)"), QObject::tr("行包"), false);
	appendRegex(QRegExp(R"(X\d{3}$)"), QObject::tr("行包"), false);
	appendRegex(QRegExp(R"(X\d{4})"), QObject::tr("班列"), false);
	appendRegex(QRegExp(R"(1\d{4})"), QObject::tr("直达"), false);
	appendRegex(QRegExp(R"(2\d{4})"), QObject::tr("直货"), false);
	appendRegex(QRegExp(R"(3\d{4})"), QObject::tr("区段"), false);
	appendRegex(QRegExp(R"(4[0-4]\d{3})"), QObject::tr("摘挂"), false);
	appendRegex(QRegExp(R"(4[5-9]\d{3})"), QObject::tr("小运转"), false);
	appendRegex(QRegExp(R"(5[0-2]\d{3})"), QObject::tr("单机"), false);
	appendRegex(QRegExp(R"(5[3-4]\d{3})"), QObject::tr("补机"), false);
	appendRegex(QRegExp(R"(55\d{3})"), QObject::tr("试运转"), false);
}
