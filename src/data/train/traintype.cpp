#include "traintype.h"

#include <QJsonObject>



TypeManager::TypeManager():
	defaultPen(QColor(0,128,0)),
	defaultType(std::make_shared<TrainType>(QObject::tr("其他_"),defaultPen))
{
}

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
	defaultPen = another.defaultPen;
	defaultType = std::make_shared<TrainType>(*(another.defaultType));   //copy
	return *this;
}

void TypeManager::readForDefault(const QJsonObject& obj)
{
	bool flag = fromJson(obj);
	if (!flag) {
		initDefaultTypes();
	}
}

void TypeManager::readForDiagram(const QJsonObject& obj, const TypeManager& defaultManager)
{
	_types.clear();
	_regs.clear();
	bool flag = fromJson(obj);
	if (!flag) {
		qDebug() << "TypeManager::readForDiagram: WARNING: load type configuration for Diagram failed."
			<< " Default TypeManager will be used. " << Qt::endl;
		operator=(defaultManager);
	}
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

std::shared_ptr<TrainType> TypeManager::appendRegex(const QRegExp& reg, const QString& name, bool passenger)
{
	auto t = findOrCreate(name);
	t->setIsPassenger(passenger);
	_regs.append(qMakePair(reg, t));
	return t;
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

bool TypeManager::fromJson(const QJsonObject& obj)
{
	if (obj.isEmpty())
		return false;

	//先读取正则
	const QJsonArray& arreg = obj.value("type_regex").toArray();
	double pw = obj.value("default_keche_width").toDouble(1.5),
		fw = obj.value("default_huoche_width").toDouble(0.75);
	for (const auto& p:arreg) {
		const QJsonArray tp = p.toArray();
		auto t = appendRegex(QRegExp(tp.at(1).toString()), tp.at(0).toString(), tp.at(2).toBool());
		if (t->isPassenger())
			t->pen().setWidthF(pw);
		else
			t->pen().setWidthF(fw);
	}

	//读取颜色
	const QJsonObject& obcolor = obj.value("default_colors").toObject();
	for (auto p = obcolor.begin(); p != obcolor.end(); ++p) {
		if (p.key() == "default") {
			defaultType->pen().setColor(QColor(p.value().toString()));
		}
		else {
			auto tp = findOrCreate(p.key());
			tp->pen().setColor(QColor(p.value().toString()));
		}
	}

	//读取线型 新增
	const QJsonObject& ls = obj.value("line_styles").toObject();
	for (auto p = ls.begin(); p != ls.end(); ++p) {
		const QJsonArray& arsub = p.value().toArray();
		if (p.key() == "default") {
			defaultType->pen().setStyle(static_cast<Qt::PenStyle>(arsub.at(0).toInt()));
			defaultType->pen().setWidthF(arsub.at(1).toDouble());
		}
		else {
			auto tp = findOrCreate(p.key());
			tp->pen().setStyle(static_cast<Qt::PenStyle>(arsub.at(0).toInt()));
			tp->pen().setWidthF(arsub.at(1).toDouble());
		}
	}

	if (!_types.contains(defaultType->name())) {
		_types.insert(defaultType->name(), defaultType);
	}

	return true;
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

void TypeManager::toJson(QJsonObject& obj) const
{
	//颜色
	QJsonObject objcolor;
	//LineStype二元组：线型，粗细。新增
	QJsonObject ls;
	for (auto p = _types.begin(); p != _types.end(); ++p) {
		if (p.value() == defaultType) {
			objcolor.insert("default", p.value()->pen().color().name());
			ls.insert("default", QJsonArray{ p.value()->pen().style(),p.value()->pen().widthF() });
		}
		else {
			objcolor.insert(p.key(), p.value()->pen().color().name());
			ls.insert(p.key(),
				QJsonArray{ p.value()->pen().style(),p.value()->pen().widthF() });
		}
	}
	obj.insert("default_colors", objcolor);
	obj.insert("line_styles", ls);
	//正则
	QJsonArray arreg;
	for (auto p = _regs.begin(); p != _regs.end(); ++p) {
		arreg.append(
			QJsonArray{ p->second->name(), p->first.pattern(),p->second->isPassenger() });
	}
	obj.insert("type_regex", arreg);
}
