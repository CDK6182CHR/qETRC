#include "railinterval.h"
#include "railstation.h"
#include "ruler.h"
#include "rulernode.h"

#include <cassert>

RailInterval::RailInterval(Direction dir_):
    _dir(dir_)
{
}

RailInterval::RailInterval(Direction dir_,
	std::weak_ptr<RailStation> from_, std::weak_ptr<RailStation> to_):
    from(from_),to(to_),_dir(dir_)
{
}

std::shared_ptr<RailInterval>
    RailInterval::construct(Direction _dir, std::shared_ptr<RailStation> from,
                            std::shared_ptr<RailStation> to)
{
    std::shared_ptr<RailInterval> t(new RailInterval(_dir,from,to));
    if(_dir==Direction::Down){
        from->downNext=t;
        to->downPrev=t;
    }else{
        from->upNext=t;
        to->upPrev=t;
    }
    return t;
}

QString RailInterval::fromStationNameLit() const
{
    return fromStation()->name.toSingleLiteral();
}

QString RailInterval::toStationNameLit() const
{
    return toStation()->name.toSingleLiteral();
}

std::shared_ptr<RailInterval> RailInterval::prevInterval()const
{
	if (!fromStation())
		return std::shared_ptr<RailInterval>();
    if (_dir==Direction::Down) {
		return fromStation()->downPrev;
	}
	else {
		return fromStation()->upPrev;
	}
}

std::shared_ptr<RailInterval> RailInterval::nextInterval() const
{
	if(!toStation())
		return std::shared_ptr<RailInterval>();
    return isDown() ? toStation()->downNext : toStation()->upNext;
}

RailInterval RailInterval::mergeWith(const RailInterval& next) const
{
	assert(toStation() == next.fromStation());
    assert(_dir == next._dir);

    RailInterval it(_dir, from, next.to);
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
    if(!fromStation()||!toStation())
        return 0;
    if(isDown()){
        return toStation()->mile - fromStation()->mile;
    }else{
        return fromStation()->counterMile() - toStation()->counterMile();
    }
}

std::shared_ptr<RulerNode> RailInterval::getRulerNode(std::shared_ptr<Ruler> ruler)
{
    return rulerNodeAt(ruler->index());
}

std::shared_ptr<RulerNode> RailInterval::getRulerNode(const Ruler& ruler)
{
    return rulerNodeAt(ruler.index());
}

std::shared_ptr<RailInterval> RailInterval::inverseInterval()
{
    if (fromStation()->direction == PassedDirection::BothVia &&
        toStation()->direction == PassedDirection::BothVia) {
        return toStation()->dirNextInterval(DirFunc::reverse(_dir));
    }
    return nullptr;
}

QDebug operator<<(QDebug debug, const RailInterval& s)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << s.fromStation()->name<<"->"<<s.toStation()->name;
    return debug;
}
