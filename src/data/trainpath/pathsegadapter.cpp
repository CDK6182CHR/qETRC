#include "pathsegadapter.h"

#include <cassert>

#include "data/diagram/trainline.h"

const AdapterStation& PathSegAdapter::firstAdapterStation() const
{
	assert(m_type != EmptyBind);
	if (m_type == SingleBind) {
		return m_single_adp_station.value();
	}
	else {
		assert(m_line != nullptr);
		return m_line->stations().front();
	}
}

const AdapterStation& PathSegAdapter::lastAdapterStation() const
{
	assert(m_type != EmptyBind);
	if (m_type == SingleBind) {
		return m_single_adp_station.value();
	}
	else {
		assert(m_line != nullptr);
		return m_line->stations().back();
	}
}
