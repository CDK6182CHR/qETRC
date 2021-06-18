#include "railinterval.h"
#include "railstation.h"

#include <cassert>

RailInterval::RailInterval(bool down_):
	down(down_)
{
}

RailInterval::RailInterval(bool down_, 
	std::shared_ptr<RailStation> from_, std::shared_ptr<RailStation> to_):
	from(from_),to(to_),down(down_)
{
	if (down) {
		from->downNext = shared_from_this();
		to->downPrev = shared_from_this();
	}
	else {
		from->upNext = shared_from_this();
		to->upPrev = shared_from_this();
	}
}

std::shared_ptr<RailInterval> RailInterval::prevInterval()const
{
	if (!from)
		return std::shared_ptr<RailInterval>();
	if (down) {
		return from->downPrev;
	}
	else {
		return from->upPrev;
	}
}

std::shared_ptr<RailInterval> RailInterval::nextInterval() const
{
	if(!to)
		return std::shared_ptr<RailInterval>();
	return down ? to->downNext : to->upNext;
}

RailInterval RailInterval::mergeWith(const RailInterval& next) const
{
	assert(to == next.from);
	assert(down == next.down);

	RailInterval it(down, from, next.to);
	//todo: 标尺等数据处理
	return it;
}
