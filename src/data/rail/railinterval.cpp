#include "railinterval.h"
#include "railstation.h"
#include "ruler.h"
#include "rulernode.h"

#include <cassert>

RailInterval::RailInterval(Direction dir_):
    dir(dir_)
{
}

RailInterval::RailInterval(Direction dir_,
	std::shared_ptr<RailStation> from_, std::shared_ptr<RailStation> to_):
    from(from_),to(to_),dir(dir_)
{
}

std::shared_ptr<RailInterval>
    RailInterval::construct(Direction dir, std::shared_ptr<RailStation> from,
                            std::shared_ptr<RailStation> to)
{
    std::shared_ptr<RailInterval> t(new RailInterval(dir,from,to));
    if(dir==Direction::Down){
        from->downNext=t;
        to->downPrev=t;
    }else{
        from->upNext=t;
        to->upPrev=t;
    }
    return t;
}

std::shared_ptr<RailInterval> RailInterval::prevInterval()const
{
	if (!from)
		return std::shared_ptr<RailInterval>();
    if (dir==Direction::Down) {
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
    return isDown() ? to->downNext : to->upNext;
}

RailInterval RailInterval::mergeWith(const RailInterval& next) const
{
	assert(to == next.from);
    assert(dir == next.dir);

    RailInterval it(dir, from, next.to);
    for(int i=0;i<_rulerNodes.count();i++){
        auto p=it._rulerNodes[i];
        const auto p1=_rulerNodes.at(i);
        const auto p2=next._rulerNodes.at(i);
        p->start=p1->start;
        p->stop=p2->stop;
        p->interval=p1->interval+p2->interval;
    }
    return it;
}

double RailInterval::mile() const
{
    if(!from||!to)
        return 0;
    if(isDown()){
        return to->mile-from->mile;
    }else{
        return from->counterMile()-to->counterMile();
    }
}

QDebug operator<<(QDebug debug, const RailInterval& s)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << s.fromStation()->name<<"->"<<s.toStation()->name;
    return debug;
}
