#include "railcategory.h"
#include "railway.h"

RailCategory::RailCategory(const QString& name):
	 _name(name)
{
}

bool RailCategory::railNameIsValid(const QString& name, std::shared_ptr<const Railway> rail) const
{
	if (name.isEmpty())return false;
	for (auto p : _railways) {
		if (p->name() == name && p != rail)
			return false;
	}
	foreach(auto p, _subcats) {
		if (p->name() == name)
			return false;
	}
	return true;
}

bool RailCategory::categoryNameIsValid(const QString& name, 
	std::shared_ptr<const RailCategory> cat) const
{
	if (name.isEmpty())return false;
	for (auto p : _railways) {
		if (p->name() == name)
			return false;
	}
	foreach(auto p, _subcats) {
		if (p->name() == name && p!=cat)
			return false;
	}
	return true;
}

bool RailCategory::railNameIsValidRec(const QString& name, std::shared_ptr<const Railway> rail) const
{
	if (name.isEmpty())return false;
	for (auto p : _railways) {
		if (p->name() == name && p != rail)
			return false;
	}
	foreach(auto p, _subcats) {
		if (p->name() == name || !p->railNameIsValidRec(name,rail))
			return false;
	}
	return true;
}

bool RailCategory::categoryNameIsValidRec(const QString& name,
	std::shared_ptr<const RailCategory> cat) const
{
	if (name.isEmpty())return false;
	for (auto p : _railways) {
		if (p->name() == name)
			return false;
	}
	foreach(auto p, _subcats) {
		if ((p->name() == name && p != cat) || !p->categoryNameIsValidRec(name,cat))
			return false;
	}

	return true;
}

QString RailCategory::validRailwayName(const QString& prefix)const
{
	for (int i = 0;; i++) {
		QString res = prefix;
		if (i)res += QString::number(i);
		if (railNameIsValid(res, nullptr))
			return res;
	}
}

QString RailCategory::validRailwayNameRec(const QString& prefix)const
{
	for (int i = 0;; i++) {
		QString res = prefix;
		if (i)res += QString::number(i);
		if (railNameIsValidRec(res, nullptr))
			return res;
	}
}

QString RailCategory::validCategoryNameRec(const QString& prefix) const
{
	for (int i = 0;; i++) {
		QString res = prefix;
		if (i)res += QString::number(i);
		if (categoryNameIsValidRec(res, nullptr))
			return res;
	}
}

int RailCategory::getRailwayIndex(std::shared_ptr<const Railway> rail) const
{
	for (int i = 0; i < _railways.size(); i++) {
		auto p = _railways.at(i);
		if (p == rail)
			return i;
	}
	return -1;
}

int RailCategory::getRailwayIndex(const Railway& rail) const
{
	for (int i = 0; i < _railways.size(); i++) {
		auto p = _railways.at(i);
		if (p.get() == &rail)
			return i;
	}
	return -1;
}

void RailCategory::fromJson(const QJsonObject& obj)
{
	clear();
	for (auto p = obj.constBegin(); p != obj.constEnd(); ++p) {
		const auto& subobj = p.value().toObject();
		if (isRailway(subobj)) {
			_railways.push_back(std::make_shared<Railway>(subobj));
		}
		else {
			auto subcat = std::make_shared<RailCategory>(p.key());
			subcat->fromJson(subobj);
			_subcats.push_back(subcat);
		}
	}
}

QJsonObject RailCategory::toJson() const
{
	QJsonObject obj;
	foreach(auto p, _subcats) {
		obj.insert(p->name(), p->toJson());
	}
	foreach(auto p, _railways) {
		obj.insert(p->name(), p->toJson());
	}
	return obj;
}

void RailCategory::clear()
{
	_subcats.clear();
	_railways.clear();
}

bool RailCategory::isNull() const
{
	return _subcats.empty() && _railways.empty();
}

RailCategory RailCategory::shallowCopy() const
{
	RailCategory other(*this);   // copy construct!
	return other;
}

bool RailCategory::isRailway(const QJsonObject& obj)
{
	return obj.contains("name") && obj.value("name").isString() &&
		obj.contains("stations") && obj.value("stations").isArray();
}
