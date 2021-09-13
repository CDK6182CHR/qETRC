#include "railcategory.h"
#include "railway.h"

RailCategory::RailCategory(std::weak_ptr<RailCategory> parent):
	_parent(parent)
{
}

bool RailCategory::railNameIsValid(const QString& name, std::shared_ptr<Railway> rail) const
{
	if (name.isEmpty())return false;
	for (auto p : _railways) {
		if (p->name() == name && p != rail)
			return false;
	}
	return true;
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
