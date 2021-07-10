#include "railcategory.h"

bool RailCategory::railNameIsValid(const QString& name, std::shared_ptr<Railway> rail) const
{
	if (name.isEmpty())return false;
	for (auto p : _railways) {
		if (p->name() == name && p != rail)
			return false;
	}
	return true;
}

int RailCategory::getRailwayIndex(std::shared_ptr<Railway> rail) const
{
	for (int i = 0; i < _railways.size(); i++) {
		auto p = _railways.at(i);
		if (p == rail)
			return i;
	}
	return -1;
}
