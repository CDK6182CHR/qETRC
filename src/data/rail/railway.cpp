#include "railway.h"
#include "util/qeexceptions.h"
#include <cmath>
#include <algorithm>
#include <QPair>
#include <memory>
#include <QDebug>
#include <QJsonArray>

#include "railinterval.h"
#include "ruler.h"
#include "rulernode.h"
#include "forbid.h"
#include "data/diagram/config.h"


Railway::Railway(const QString& name) :
	_name(name), numberMapEnabled(false)
{
}

Railway::Railway(const QJsonObject& obj) :
	numberMapEnabled(false)
{
	fromJson(obj);
}

Railway::Railway(const Railway& other):
	_name(other._name),_notes(other._notes),_diagramHeight(other._diagramHeight),
	numberMapEnabled(false)
{
	// 通过逐个添加车站的方式，构建空区间
	foreach(auto p, other._stations) {
		appendStation(*p);
	}
	// 标尺 天窗，采用addFrom的方式
	foreach(auto ruler, other._rulers) {
		addRulerFrom(ruler);
	}
	foreach(auto forbid, other._forbids) {
		addForbidFrom(forbid);
	}
}

void Railway::moveStationInfo(Railway&& another)
{
	_name = std::move(another._name);
	_stations = std::move(another._stations);
	nameMap = std::move(another.nameMap);
	fieldMap = std::move(another.fieldMap);
	numberMapEnabled = another.numberMapEnabled;
	numberMap = std::move(another.numberMap);
}

void Railway::fromJson(const QJsonObject& obj)
{
	_stations.clear();
	_rulers.clear();
	_forbids.clear();

	_name = obj.value("name").toString();
	_notes.fromJson(obj.value("notes").toObject());
	const QJsonArray& ar = obj.value("stations").toArray();
	for (auto t = ar.cbegin(); t != ar.cend(); t++) {
		appendStation(RailStation(t->toObject()));
	}

	//this part for Rulers
	const QJsonArray& arrulers = obj.value("rulers").toArray();
	for (auto t = arrulers.begin(); t != arrulers.end(); t++) {
		addRuler(t->toObject());
	}
    //for Forbids
    const QJsonObject& objfor=obj.value("forbid").toObject();
    addForbid(objfor);
    const QJsonObject& objfor2=obj.value("forbid2").toObject();
    addForbid(objfor2);

	_ordinate = rulerByName(obj.value("ordinate").toString());
}

QJsonObject Railway::toJson() const
{
	auto obj = QJsonObject({
		 {"name",_name},
		 {"notes",_notes.toJson()}
		});
	QJsonArray ar;
	for (const auto& t : _stations) {
		ar.append(t->toJson());
	}
	obj.insert("stations", ar);

	QJsonArray arruler;
	for (const auto& t : _rulers) {
        arruler.append(t->toJson());
	}
	obj.insert("rulers", arruler);

    //forbid
    if(_forbids.size()>=1){
        obj.insert("forbid",_forbids[0]->toJson());
    }
    if(_forbids.size()>=2){
        obj.insert("forbid2",_forbids[1]->toJson());
    }

	//排图标尺 新版
	if (_ordinate)
		obj.insert("ordinate", _ordinate->name());
	else
		obj.insert("ordinate", QJsonValue(QJsonValue::Null));

	return obj;
}

void Railway::appendStation(const StationName& name, double mile, int level, 
	std::optional<double> counter, PassedDirection direction, bool show,
	bool passenger, bool freight)
{
	auto&& t = std::make_shared<RailStation>(
		name, mile, level, counter, direction, show, passenger, freight);
	_stations.append(t);
	addMapInfo(t);
	appendInterval(t);
}

void Railway::insertStation(int index, const StationName& name, double mile,
	int level, std::optional<double> counter, PassedDirection direction, bool show,
	bool passenger, bool freight)
{
	auto&& t = std::make_shared<RailStation>(name, mile, level, counter, direction,
		show, passenger, freight);
	if (index == -1)
		_stations.append(t);
	else
		_stations.insert(index, t);
	addMapInfo(t);
	insertInterval(index, t);
}

std::shared_ptr<RailStation> Railway::stationByName(const StationName& name)
{
	return nameMap.value(name);
}

const std::shared_ptr<const RailStation> Railway::stationByName(const StationName& name) const
{
	return nameMap.value(name);
}

std::shared_ptr<RailStation>
Railway::stationByGeneralName(const StationName& name)
{
	auto p = stationByName(name);
	if (p) 
		return p;
	const QList<StationName>& t = fieldMap.value(name.station());
	for (const auto& p : t) {
		if (p.equalOrContains(name)) {
			return stationByName(p);
		}
	}
	return std::shared_ptr<RailStation>(nullptr);
}

const std::shared_ptr<const RailStation>
Railway::stationByGeneralName(const StationName& name) const
{
	auto p = stationByName(name);
	if (p)
		return p;
	const QList<StationName>& t = fieldMap.value(name.station());
	for (const auto& p : t) {
		if (p.equalOrContains(name)) {
			return stationByName(p);
		}
	}
	return std::shared_ptr<RailStation>(nullptr);
}

bool Railway::containsStation(const StationName& name) const
{
	return nameMap.contains(name);
}

bool Railway::containsGeneralStation(const StationName& name) const
{
	if (!fieldMap.contains(name.station()))
		return false;
	const auto& t = fieldMap.value(name.station());
	for (const auto& p : t) {
		if (p.isBare() || p == name)
			return true;
	}
	return false;
}

int Railway::stationIndex(const StationName& name) const
{
	if (numberMapEnabled) {
		auto t = localName(name);
		if (numberMap.contains(t))
			return numberMap.value(t);
		return -1;
	}
	else {
		return stationIndexBrute(name);
	}
}

void Railway::removeStation(const StationName& name)
{
	if (numberMapEnabled) {
		if (numberMap.contains(name)) {
			int i = numberMap.value(name);
			_stations.removeAt(i);
			numberMap.remove(name);
			removeInterval(i);
			removeMapInfo(name);
			return;
		}
	}
	//QList.remove应当是线性算法，可能还不如直接手写来得快
	for (int i = 0; i < _stations.count(); i++) {
		const auto& t = _stations[i];
		if (t->name == name) {
			removeInterval(i);
			removeMapInfo(name);
			_stations.removeAt(i);
			break;
		}
	}
}

void Railway::removeStation(int index)
{
	const auto& name = _stations.at(index)->name;
	removeInterval(index);
	_stations.removeAt(index);
	if (numberMapEnabled)
		numberMap.remove(name);
	removeMapInfo(name);
}

void Railway::adjustMileToZero()
{
	if (_stations.empty())
		return;
	double m0 = _stations[0]->mile;
	std::optional<double> c0 = _stations[0]->counter;
	for (auto& t : _stations) {
		t->mile -= m0;
	}
	if (c0.has_value()) {
		for (auto& t : _stations) {
			if (t->counter.has_value()) {
				t->counter.value() -= c0.value();
			}
		}
	}
}

Direction Railway::gapDirection(const StationName& s1, const StationName& s2) const
{
	const auto& t1 = stationByGeneralName(s1),
		t2 = stationByGeneralName(s2);
	if (!t1 || !t2)
		return Direction::Undefined;
	return t1->mile <= t2->mile ? Direction::Down : Direction::Up;
}

Direction Railway::gapDirection(const std::shared_ptr<const RailStation>& s1,
	const std::shared_ptr<const RailStation>& s2) const
{
	return s1->mile <= s2->mile ? Direction::Down : Direction::Up;
}

Direction Railway::gapDirectionByIndex(const StationName& s1, const StationName& s2) const
{
	int i1 = stationIndex(s1), i2 = stationIndex(s2);
	return i1 <= i2 ? Direction::Down : Direction::Up;
}

int Railway::stationsBetween(std::shared_ptr<const RailStation> s1, 
	std::shared_ptr<const RailStation> s2) const
{
	Direction _dir = gapDirection(s1, s2);
	//先正向找
	int cnt = 0;
	auto s = s1->dirAdjacent(_dir);
	for (; s && s != s2; s = s->dirAdjacent(_dir)) {
		cnt++;
	}
	if (s) {
		//找到了目标站
		return cnt;
	}
	//没找到，方向出了问题
	qDebug() << "Railway::stationsBetween: WARNING: invalid direction encountered " << s1->name << "->"
		<< s2->name <<", direction: "<<static_cast<int>(_dir)
		<< Qt::endl;
	cnt = 0;
	_dir = DirFunc::reverse(_dir);
	s = s1->dirAdjacent(_dir);
	for (; s && s != s2; s = s->dirAdjacent(_dir))
		cnt++;
	return cnt;
}

double Railway::mileBetween(const StationName& s1,
	const StationName& s2) const
{
	const auto& t1 = stationByGeneralName(s1),
		t2 = stationByGeneralName(s2);
	if (!t1 || !t2) {
		return -1;
	}
	if (gapDirection(t1, t2) == Direction::Up) {
		if (t1->counter.has_value() && t2->counter.has_value()) {
			return std::abs(t1->counter.value() - t2->counter.value());
		}
	}
	return std::abs(t1->mile - t2->mile);
}

bool Railway::isSplitted() const
{
	for (const auto& p : _stations) {
		if (p->direction == PassedDirection::DownVia ||
			p->direction == PassedDirection::UpVia)
			return true;
	}
	return false;
}

std::shared_ptr<Ruler> Railway::rulerByName(const QString& name)
{
	if (name.isEmpty() || name.isNull())
		return nullptr;
	for (auto p = _rulers.begin(); p != _rulers.end(); ++p) {
		if ((*p)->name() == name)
			return *p;
	}
	return std::shared_ptr<Ruler>();
}

bool Railway::rulerNameExisted(const QString& name, std::shared_ptr<const Ruler> ignore) const
{
	for (const auto& p : _rulers) {
		if (p->name() == name && p != ignore) {
			return true;
		}
	}
	return false;
}

QString Railway::validRulerName(const QString& prefix) const
{
	for (int i = 0;; i++) {
		QString name = prefix;
		if (i)
			name += QString::number(i);
		if (!rulerNameExisted(name))
			return name;
	}
}


void Railway::removeRuler(std::shared_ptr<Ruler> ruler)
{
	int index = ruler->index();
	if (index == ordinateIndex()) {
		resetOrdinate();
	}
	//首先从头结点里面抹掉
	_rulers.removeAt(index);
	//改后面的序号
	for (int i = index; i < _rulers.size(); i++) {
		_rulers[i]->_index--;
	}
	//遍历interval  只需要删掉结点元素
	auto p = firstDownInterval();
	for (; p; p = nextIntervalCirc(p)) {
		p->_rulerNodes.removeAt(index);
	}
}

std::shared_ptr<Ruler> Railway::takeLastRuler()
{
	auto t = _rulers.last();
	removeRuler(t);
	return t;
}

void Railway::undoRemoveRuler(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data)
{
	int idx = ruler->index();
	_rulers.insert(idx, ruler);
	//修改序号
	for (int i = idx + 1; i < _rulers.size(); i++) {
		_rulers.at(i)->_index++;
	}
	auto r = data->getRuler(0);
	auto p = firstDownInterval();
	auto n = r->firstDownNode();
	for (; p && n; p = nextIntervalCirc(p), n = n->nextNodeCirc()) {
		//qDebug() << "addRulerFrom: "<<*p<<'\t' << p->_rulerNodes.last().get() << " @ " << p.get();
		p->_rulerNodes.insert(idx,
			std::make_shared<RulerNode>(*ruler, *p, n->interval, n->start, n->stop));
	}
	if (p || n) {
		qDebug() << "Railway::undoRemoveRuler: unexpected early termination!";
	}
}

void Railway::clearRulers()
{
	//先去清理结点数据，免得Ruler对象被析构引起引用的危险
	for (auto p = firstDownInterval(); p; p = nextIntervalCirc(p)) {
		p->_rulerNodes.clear();
	}
	_rulers.clear();
}

void Railway::clearForbids()
{
	//先去清理结点数据，免得Ruler对象被析构引起引用的危险
	for (auto p = firstDownInterval(); p; p = nextIntervalCirc(p)) {
		p->_forbidNodes.clear();
	}
	_forbids.clear();
}

//void Railway::changeStationName(const StationName& oldname,
//	const StationName& newname)
//{
//	auto p = stationByName(oldname);
//	p->name = newname;
//
//	//更新映射表
//	removeMapInfo(oldname);
//	addMapInfo(p);
//}

void Railway::swapStationName(std::shared_ptr<RailStation> station, StationName& name)
{
	std::swap(station->name, name);
	removeMapInfo(name);    //现在这是旧的站名！
	addMapInfo(station);
}

double Railway::counterLength() const
{
	if (_stations.empty())
		return 0;
	const auto& last = _stations.last();
	return last->counter.has_value() ?
		last->counter.value() : last->mile;
}

void Railway::reverse()
{
	double length = railLength(), ctlen = counterLength();
    for (auto q=_stations.begin();q!=_stations.end();++q) {
        auto p=*q;
		p->mile = length - p->mile;
		if (p->counter.has_value()) {
			p->counter = ctlen - p->counter.value();
		}
		else {
			p->counter = p->mile;
		}
		std::swap(p->counter.value(), p->mile);
		switch (p->direction) {
		case PassedDirection::DownVia:
			p->direction = PassedDirection::UpVia; break;
		case PassedDirection::UpVia:
			p->direction = PassedDirection::DownVia; break;
		default:break;
		}
        std::swap(p->downNext,p->upNext);
        std::swap(p->downPrev,p->upPrev);
        //翻转每个interval内部保存的方向
        if(p->downNext){
            p->downNext->_dir=DirFunc::reverse(p->downNext->_dir);
        }
        if(p->upNext){
            p->upNext->_dir=DirFunc::reverse(p->upNext->_dir);
        }
	}
	std::reverse(_stations.begin(), _stations.end());
}

QList<QPair<StationName, StationName>> Railway::adjIntervals(bool down) const
{
	QList<QPair<StationName, StationName>> res;
	res.reserve(stationCount());   //预留空间

	if (empty()) return res;

	if (down) {
		auto p = _stations.cbegin();
		StationName last = (*p)->name;
		for (++p; p != _stations.cend(); ++p) {
			auto& q = *p;
			if (!q->isDownVia())
				continue;
			res.append(qMakePair(last, q->name));
			last = q->name;
		}
	}
	else {  //not down
		auto p = _stations.crbegin();
		StationName last = (*p)->name;
		for (++p; p != _stations.crend(); ++p) {
			auto& q = *p;
			if (!q->isUpVia())continue;
			res.append(qMakePair(last, q->name));
			last = q->name;
		}
	}
	return res;
}

void Railway::mergeCounter(const Railway& another)
{
	//注意插入会导致迭代器失效，因此不可用迭代器
	//2021.09.24 提醒修改区间情况！
	int i = 0;
	int j = another.stationCount() - 1;
	while (true) {
		if (i >= stationCount() || j < 0)
			break;
		const auto& si = _stations[i];
		const auto& sj = another._stations.at(j);
		if (si->name == sj->name) {
			si->direction = PassedDirection::BothVia;
			si->counter = another.railLength() - sj->mile;
			i++; j--;
		}
		else {
			if (!containsStation(sj->name)) {
				//上行单向站，插入处理
				double m = another.railLength() - sj->mile;
				insertStation(i, sj->name, m, sj->level,
					m, PassedDirection::UpVia);
				i++; j--;
			}
			else {
				//下行单向站
				i++;
			}
		}
	}
	//[对里程]的修正：使得零点和正里程一样
	if (empty())return;
	if (!_stations.at(0)->isUpVia()) {
		int i = 0;
		while (i < stationCount() && !_stations.at(i)->isUpVia())
			i++;
		if (i < stationCount()) {
			double m0 = _stations.at(i)->mile;
			for (int j = i; j < stationCount(); j++) {
				if (_stations.at(j)->counter.has_value()) {
					_stations.at(j)->counter.value() += m0;
				}
			}
		}
	}
	initUpIntervals();

    QMap<QString,int> rulerMap;
    // 初始化 自己的标尺下标
    foreach(const auto& r,_rulers){
        rulerMap.insert(r->name(),r->index());
    }

    // 2021.09.24 新增 标尺天窗的合并
    // 以上算法保证现在this的上行径路和another的下行径路（区间）完全一致
    std::shared_ptr<RailInterval> p1;
    std::shared_ptr<const RailInterval> p2;
    for(p1=firstUpInterval(), p2=another.firstDownInterval();
        p1 && p2; p1=p1->nextInterval(),p2=p2->nextInterval()){
        // 天窗 按下标
		//qDebug() << "merge intervals: " << *p1 << ", " << *p2;
        for(int i=0;i<std::min(forbids().size(),another.forbids().size());i++){
            p1->_forbidNodes.at(i)->operator=(*(p2->_forbidNodes.at(i)));
        }

        // 标尺 按名称
        for(int i=0;i<another._rulers.size();i++){
            auto ruler=another._rulers.at(i);
            int idx;
            if (auto itr=rulerMap.find(ruler->name());itr!=rulerMap.end()){
                idx=itr.value();
            }else{
                idx=rulerMap.size();
                addEmptyRuler(ruler->name(), true);
                rulerMap.insert(ruler->name(),idx);
            }
            p1->_rulerNodes.at(idx)->operator=(
                        *(p2->_rulerNodes.at(ruler->index())));
        }
    }
    if ((!p1) != (!p2)){
        qDebug()<<"Railway::mergeCounter: WARNING: "<<
                  "interval not terminated simutaneously!"<<Qt::endl;
		//qDebug() << *p2 << Qt::endl;
    }
}

#if 0
Railway Railway::slice(int start, int end) const
{
	Railway rail;
	rail._stations.reserve(end - start);
	for (int i = start; i < end; i++) {
		rail._stations.append(_stations.at(i));
	}
	// todo ruler
	rail.setMapInfo();
	return rail;
}
#endif

void Railway::jointWith(const Railway& another, bool former, bool reverse)
{
	if (reverse) {
		this->reverse();
	}
	if (former) {
		//对方在前
		double len = another.railLength();
		for (auto& p : _stations) {
			p->mile += len;
		}
		for (auto p = another._stations.crbegin();
			p != another._stations.crend(); p++) {
			if (!containsStation((*p)->name)) {
				insertStation(0, **p);
			}
		}
	}
	else {  //not former
		double length = railLength();
		double ctlen = counterLength();
		for (const auto& t : another._stations) {
			appendStation(*t);
			auto& last = _stations.last();
			last->mile += length;
			if (last->counter.has_value())
				last->counter.value() += ctlen;
		}
	}

	//todo: 标尺天窗...
}

std::shared_ptr<const RailInterval> Railway::firstDownInterval() const
{
	for (int i = 0; i < stationCount(); i++) {
		const auto& t = _stations.at(i);
		if (t->isDownVia())
			return t->downNext;
	}
    return std::shared_ptr<RailInterval>();
}

std::shared_ptr<RailInterval> Railway::firstDownInterval()
{
    for (int i = 0; i < stationCount(); i++) {
        const auto& t = _stations.at(i);
        if (t->isDownVia())
            return t->downNext;
    }
    return std::shared_ptr<RailInterval>();
}

std::shared_ptr<const RailInterval> Railway::firstUpInterval() const
{
	for (int i = stationCount() - 1; i >= 0; i--) {
		const auto& t = _stations.at(i);
		if (t->isUpVia()) {
			return t->upNext;
		}
	}
    return std::shared_ptr<RailInterval>();
}

std::shared_ptr<RailInterval> Railway::firstUpInterval()
{
    for (int i = stationCount() - 1; i >= 0; i--) {
        const auto& t = _stations.at(i);
        if (t->isUpVia()) {
            return t->upNext;
        }
    }
    return std::shared_ptr<RailInterval>();
}

std::shared_ptr<RailInterval> Railway::lastDownInterval()
{
    for(auto p=_stations.rbegin();p!=_stations.rend();++p){
        if((*p)->isDownVia())
            return (*p)->downPrev;
    }
    return nullptr;
}

std::shared_ptr<const RailStation> Railway::firstDownStation() const
{
	for (auto p : _stations) {
		if (p->isDownVia())
			return p;
	}
	return nullptr;
}

std::shared_ptr<const RailStation> Railway::firstUpStation() const
{
	for (auto p = _stations.crbegin(); p != _stations.crend(); ++p) {
		if ((*p)->isUpVia())
			return *p;
	}
	return {};
}

std::shared_ptr<const RailStation> Railway::firstDirStation(Direction dir) const
{
	switch (dir)
	{
	case Direction::Down: return firstDownStation();
		break;
	case Direction::Up: return firstUpStation();
		break;
	default: return {};
		break;
	}
}



void Railway::showStations() const
{
	qDebug() << "Stations list for railway: " << _name << Qt::endl;
	for (int i = 0; i < stationCount(); i++) {
		const auto& p = _stations.at(i);
		qDebug() << i << '\t' << p->name << '\t' << p->mile << '\t' <<
			p->counterStr() << '\t' <<
			static_cast<int>(p->direction) << Qt::endl;
	}
}

void Railway::showIntervals() const
{
	qDebug() << "Down intervals for railway: " << _name << Qt::endl;
	auto p = firstDownInterval();
	for (; p; p = p->nextInterval()) {
		qDebug() << p->fromStation()->name << "->" << p->toStation()->name << '\t'
			<< p->mile() << '\t' << p.get() << Qt::endl;
	}
	qDebug() << "Up intervals for railway: " << _name << Qt::endl;
	p = firstUpInterval();
	for (; p; p = p->nextInterval()) {
		qDebug() << p->fromStation()->name << "->" << p->toStation()->name << '\t'
			<< p->mile() << '\t' << p.get() << Qt::endl;
	}
}

std::shared_ptr<Ruler> Railway::addEmptyRuler(const QString& name, bool different)
{
	int idx = _rulers.count();
    auto r=std::shared_ptr<Ruler>(new Ruler(*this,name,different,idx));
    _rulers.append(r);
	//下行区间
	auto p = firstDownInterval();
	for (; p; p = p->nextInterval()) {
        p->_rulerNodes.append(std::make_shared<RulerNode>(*r, *p));
	}
	p = firstUpInterval();
	for (; p; p = p->nextInterval()) {
        p->_rulerNodes.append(std::make_shared<RulerNode>(*r, *p));
	}
	return r;
}

std::shared_ptr<Ruler> Railway::addRulerFrom(std::shared_ptr<Ruler> r)
{
	return addRulerFrom(*r);
}

std::shared_ptr<Ruler> Railway::addRulerFrom(const Ruler& r)
{
	int idx = _rulers.count();
	auto ruler = std::shared_ptr<Ruler>(new Ruler(*this, r.name(), r.different(), idx));
	_rulers.append(ruler);
	auto p = firstDownInterval();
	auto n = r.firstDownNode();
	for (; p && n; p = nextIntervalCirc(p), n = n->nextNodeCirc()) {
		//qDebug() << "addRulerFrom: "<<*p<<'\t' << p->_rulerNodes.last().get() << " @ " << p.get();
		p->_rulerNodes.append(std::make_shared<RulerNode>(*ruler, *p, n->interval, n->start, n->stop));
	}
	if (p || n) {
		qDebug() << "Railway::addRulerFrom: unexpected early termination!";
	}
	return ruler;
}

std::shared_ptr<Ruler> Railway::restoreRulerFrom(std::shared_ptr<Ruler> head, const Ruler& data)
{
	auto ruler = head;
	_rulers.append(ruler);
	auto p = firstDownInterval();
	auto n = data.firstDownNode();
	for (; p && n; p = nextIntervalCirc(p), n = n->nextNodeCirc()) {
		//qDebug() << "addRulerFrom: "<<*p<<'\t' << p->_rulerNodes.last().get() << " @ " << p.get();
		p->_rulerNodes.append(std::make_shared<RulerNode>(*ruler, *p, n->interval, n->start, n->stop));
	}
	if (p || n) {
		qDebug() << "Railway::addRulerFrom: unexpected early termination!";
	}
	return ruler;
}

std::shared_ptr<Ruler> Railway::restoreRulerFrom(std::shared_ptr<Ruler> head, std::shared_ptr<const Ruler> data)
{
	return restoreRulerFrom(head, *data);
}

std::shared_ptr<Ruler> Railway::addRuler(const QJsonObject& obj)
{
	const QString& name = obj.value("name").toString();
	bool dif = obj.value("different").toBool();
	auto ruler = addEmptyRuler(name, dif);
	const QJsonArray& nodes = obj.value("nodes").toArray();
	for (auto p = nodes.cbegin(); p != nodes.cend(); ++p) {
		const QJsonObject& node = p->toObject();
		StationName
			from = StationName::fromSingleLiteral(node.value("fazhan").toString()),
			to = StationName::fromSingleLiteral(node.value("daozhan").toString());
		auto it = findGeneralInterval(from, to);
		if (!it) {
			qDebug() << "Railway::addRuler: WARNING: invalid interval " << from << "->" << to <<
				", to be ignored. " << Qt::endl;
		}
		else {
			it->rulerNodeAt(ruler->index())->fromJson(node);
		}
	}
	if (!ruler->different()) {
		qDebug() << "Railway::addRuler(const QJsonObject&): INFO: " <<
			"Ruler with different=false from old version is set to different." << Qt::endl;
		ruler->setDifferent(true);
		ruler->copyDownToUp();
	}
	return ruler;
}

std::shared_ptr<RailInterval> Railway::findInterval(const StationName& from, const StationName& to)
{
	auto t = stationByName(from);
	if (t && t->hasDownAdjacent() && t->downAdjacent()->name == to)
		return t->downNext;
	else if (t && t->hasUpAdjacent() && t->upAdjacent()->name == to)
		return t->upNext;
	return std::shared_ptr<RailInterval>();
}

std::shared_ptr<const RailInterval> Railway::findInterval(const StationName& from,
	const StationName& to) const
{
	auto t = stationByName(from);
	if (t && t->hasDownAdjacent() && t->downAdjacent()->name == to)
		return t->downNext;
	else if (t && t->hasUpAdjacent() && t->upAdjacent()->name == to)
		return t->upNext;
	return nullptr;
}

std::shared_ptr<RailInterval> Railway::findGeneralInterval(const StationName& from, const StationName& to)
{
	auto t = stationByGeneralName(from);
	//首先精确查找to
	if (t && t->hasDownAdjacent() && t->downAdjacent()->name == to)
		return t->downNext;
	else if (t && t->hasUpAdjacent() && t->upAdjacent()->name == to)
		return t->upNext;

	if (t && t->hasDownAdjacent() && t->downAdjacent()->name.equalOrContains(to))
		return t->downNext;
	else if (t &&t->hasUpAdjacent() && t->upAdjacent()->name.equalOrContains(to))
		return t->upNext;
	return std::shared_ptr<RailInterval>();
}

QList<std::shared_ptr<RailInterval>> 
	Railway::multiIntervalPath(const StationName& from, const StationName& to)
{
	using ret_t = QList<std::shared_ptr<RailInterval>>;
	//如果是近邻区间，直接解决问题
	auto p = findGeneralInterval(from, to);
	if (p) {
		return ret_t{p};
	}

	auto s1 = stationByGeneralName(from), s2 = stationByGeneralName(to);
	if (!s1 || !s2)
		return ret_t();

	Direction _dir;
	if (numberMapEnabled) {
		_dir = gapDirectionByIndex(from, to);
	}
	else {
		_dir = gapDirection(s1, s2);
	}
	//不启用时：通过里程判定上下行，原则上应该八九不离十
	//但如果很不幸方向是错的，那么就是遍历完一遍之后，再来一遍
	//首先：假定这个方向是对的
	ret_t res;
	auto it = s1->dirNextInterval(_dir);
	for (; it && it->toStation() != s2; it = it->nextInterval()) {
		res.append(it);
	}
	if (it || numberMapEnabled) {
		//到最后it没有跑到空指针，说明找到了
		//如果是用index索引的，那么肯定不会错，不用考虑另一方向
		return res;
	}
	//到这里：方向是不对的，很不幸，只能重来
	qDebug() << "Railway::multiIntervalPath: WARNING: direction seems to be wrong. " <<
		from << "->" << to << Qt::endl;
	res.clear();
	_dir = DirFunc::reverse(_dir);
	it = s1->dirNextInterval(_dir);
	for (; it && it->toStation() != s2; it = it->nextInterval()) {
		res.append(it);
	}
	if (it) {
		//到最后it没有跑到空指针，说明找到了
		return res;
	}
	else {
		return ret_t();
	}
}

std::shared_ptr<Forbid> Railway::firstForbid()
{
	if (_forbids.empty())
		addEmptyForbid();
    return _forbids.at(0);
}

void Railway::ensureForbids(int count)
{
    while(_forbids.size()<count){
        addEmptyForbid(true);
    }
}

int Railway::ordinateIndex() const
{
	return _ordinate ? _ordinate->index() : -1;
}

bool Railway::calStationYValue(const Config& config)
{
	clearYValues();
	if (!ordinate()) {
		calStationYValueByMile(config);
		return true;
	}
	//标尺排图
	auto ruler = ordinate();
	double y = 0;
	auto p = ruler->firstDownNode();
	p->railInterval().fromStation()->y_value = y;  //第一站
	//下行
	for (; p; p = p->nextNode()) {
		if (p->isNull()) {
			qDebug() << "Railway::calStationYValue: WARNING: "
				<< "Ruler [" << ruler->name() << "] not complate, cannot be used as"
				<< "ordinate ruler. Interval: " << p->railInterval() << Qt::endl;
			resetOrdinate();
			calStationYValueByMile(config);
			return false;
		}
		y += p->interval / config.seconds_per_pix_y;
		p->railInterval().toStation()->y_value = y;
	}
	_diagramHeight = y;
	//上行，仅补缺漏
	//这里利用了最后一站必定是双向的条件！
	for (p = ruler->firstUpNode(); p; p = p->nextNode()) {
		auto toStation = p->railInterval().toStation();
		if (!toStation->y_value.has_value()) {
			if (p->isNull()) {
				qDebug() << "Railway::calStationYValue: WARNING: "
					<< "Ruler [" << ruler->name() << "] not complate, cannot be used as"
					<< "ordinate ruler. Interval: " << p->railInterval() << Qt::endl;
				resetOrdinate();
				calStationYValueByMile(config);
				return false;
			}
			auto rboth = rightBothStation(toStation), lboth = leftBothStation(toStation);

			int upLeft = ruler->totalInterval(toStation, lboth, Direction::Up);
			int upRight = ruler->totalInterval(rboth, toStation, Direction::Up);
			double toty = rboth->y_value.value() - lboth->y_value.value();
			y = rboth->y_value.value() - toty * upRight / (upLeft + upRight);
			toStation->y_value = y;
		}
	}
	return true;
}

bool Railway::topoEquivalent(const Railway& another) const
{
	if (stationCount() != another.stationCount())
		return false;
	for (auto p = _stations.begin(), q = another.stations().begin();
		p != _stations.end() && q != another.stations().end(); ++p, ++q) {
		if ((*p)->direction != (*q)->direction)
			return false;
	}
	return true;
}

bool Railway::stationNameExisted(const QString& name) const
{
	return nameMap.contains(StationName::fromSingleLiteral(name));
}

bool Railway::stationNameExisted(const StationName& name) const
{
	return nameMap.contains(name);
}

bool Railway::mergeIntervalData(const Railway& other)
{
	bool equiv = topoEquivalent(other);
	//首先按照对方的，添加标尺和天窗
	clearRulers();
	clearForbids();

	if (equiv) {
		for (auto r : other._rulers)
			addRulerFrom(r);
		for (auto f : other._forbids)
			addForbidFrom(f);
	}
	else {
		//非equiv，只能逐个区间检索。先搞空的
		for (auto r : other._rulers) {
			addEmptyRuler(r->name(), r->different());
		}
		for (auto f : other._forbids) {
			addEmptyForbid(f->different());
		}
		for (auto p = firstDownInterval(); p; p = nextIntervalCirc(p)) {
			auto it = other.findInterval(p->fromStation()->name, p->toStation()->name);
			if (it) {
				//!! 不能直接赋值或者std::copy，这是对shared_ptr的操作！！
				for (int i = 0; i < p->_rulerNodes.count(); i++) {
					p->_rulerNodes[i]->operator=(*(it->_rulerNodes[i]));   //注意这里只assign了数据
				}
				for (int i = 0; i < p->_forbidNodes.count(); i++) {
					p->_forbidNodes[i]->operator = (*(it->_forbidNodes[i]));
				}
				//std::copy(it->_rulerNodes.begin(), it->_rulerNodes.end(), p->_rulerNodes.begin());
				//std::copy(it->_forbidNodes.begin(), it->_forbidNodes.end(), p->_forbidNodes.begin());
			}
		}
	}
	return equiv;
}

void Railway::swapBaseWith(Railway& other)
{
	std::swap(_stations, other._stations);   //浅拷贝（移动）
	std::swap(_rulers, other._rulers);
	std::swap(_forbids, other._forbids);
	std::swap(nameMap, other.nameMap);
	std::swap(fieldMap, other.fieldMap);
	for (int i = 0; i < _rulers.count(); i++) {
		std::swap(_rulers[i]->_railway, other._rulers[i]->_railway);
	}
	for (int i = 0; i < _forbids.count(); i++) {
		std::swap(_forbids[i]->_railway, other._forbids[i]->_railway);
	}
	//ordinate
	if (_ordinate) {
		_ordinate = _rulers.at(_ordinate->index());
		assert(&(_ordinate->_railway.get()) == this);
	}
		
	if (!_rulers.empty()) {
		assert(&(_rulers.first()->_railway.get()) == this);
		qDebug() << "Assertion of ruler railway passed.";
	}
}

std::shared_ptr<Railway> Railway::cloneBase() const
{
	auto res = std::make_shared<Railway>();
	for (auto p : _stations) {
		res->appendStation(*p);
	}
	return res;
}

Railway::SectionInfo Railway::getSectionInfo(double mile)const
{
	//按照里程进行二分搜索
	auto p = std::lower_bound(_stations.begin(), _stations.end(), mile, RailStationMileLess());
	if (p == _stations.end()) {
		return std::nullopt;
	}
	std::shared_ptr<RailInterval> itDown, itUp;
	//现在：p指向符合条件的区间后站。
	//但需注意p所指站并不一定是上行（下行）通过的站；因此需要探查到一个合理的站
	auto pr = p;
	//根据区间后站的约定，应该往后找，然后用Prev
	while (pr != _stations.end() && !(*pr)->isDownVia())
		++pr;
	if (pr != _stations.end()) {
		itDown = (*pr)->downPrev;
	}
	//上行区间
	auto po = p;
	while (po != _stations.end() && !(*po)->isUpVia())
		++po;
	if (po != _stations.end()) {
		itUp = (*po)->upNext;
	}

	//现在计算y坐标
	if (itDown) {
		//正常插值
		double y0 = itDown->from.lock()->y_value.value();
		double yn = itDown->to.lock()->y_value.value();
		double scale = (mile - itDown->from.lock()->mile) / itDown->mile();
		double yi = (yn - y0) * scale + y0;
		return std::make_tuple(yi, itDown, itUp);
	}
	else if (p == _stations.begin() && (*p)->mile == mile) {
		return std::make_tuple((*p)->y_value.value(), itDown, itUp);
	}
	else {
		return std::nullopt;
	}
}

void Railway::addMapInfo(const std::shared_ptr<RailStation>& st)
{
	//nameMap  直接添加
	const auto& n = st->name;
	nameMap.insert(n, st);
	fieldMap[n.station()].append(n);
}

void Railway::removeMapInfo(const StationName& name)
{
	nameMap.remove(name);

	auto t = fieldMap.find(name.station());
	if (t == fieldMap.end())
		return;
	else if (t.value().count() == 1) {
		fieldMap.remove(name.station());
	}
	else {
		QList<StationName>& lst = t.value();
		lst.removeAll(name);
	}
}

void Railway::setMapInfo()
{
	nameMap.clear();
	fieldMap.clear();
	nameMap.reserve(stationCount());
	fieldMap.reserve(stationCount());

	for (const auto& p : _stations) {
		nameMap.insert(p->name, p);
		fieldMap[p->name.station()].append(p->name);
	}
}

void Railway::enableNumberMap()
{
	if (numberMapEnabled)
		return;
	numberMap.clear();
	for (int i = 0; i < _stations.count(); i++) {
		const auto& t = _stations[i];
		numberMap.insert(t->name, i);
	}
	numberMapEnabled = true;
}

void Railway::disableNumberMap()
{
	numberMap.clear();
	numberMapEnabled = false;
}

int Railway::stationIndexBrute(const StationName& name) const
{
	for (int i = 0; i < _stations.count(); i++) {
		if (_stations[i]->name == name)
			return i;
	}
	return -1;
}

StationName Railway::localName(const StationName& name) const
{
	const auto& t = stationByGeneralName(name);
	if (t)
		return t->name;
	else
		return name;
}

void Railway::insertStation(int i, const RailStation& station)
{
	auto&& t = std::make_shared<RailStation>(station);    //copy constructed!!
	if (i == -1)
		_stations.append(t);
	else
		_stations.insert(i, t);
	addMapInfo(t);
	insertInterval(i, t);
}

void Railway::appendStation(const RailStation& station)
{
	auto&& t = std::make_shared<RailStation>(station);    //copy constructed!!
	_stations.append(t);
	addMapInfo(t);
	appendInterval(t);
}

void Railway::appendStation(RailStation&& station)
{
	auto&& t = std::make_shared<RailStation>(std::forward<RailStation>(station));    //move constructed!!
	_stations.append(t);
	addMapInfo(t);
	appendInterval(t);
}

void Railway::appendInterval(std::shared_ptr<RailStation> st)
{
	int n = stationCount() - 1;
	if (st->isDownVia()) {
		//添加下行区间
		auto pr = leftDirStation(n, Direction::Down);
		if (pr) {
            addInterval(Direction::Down, pr, st);
		}
	}
	if (st->isUpVia()) {
		//添加上行区间
		auto pr = leftDirStation(n, Direction::Up);
		if (pr) {
            addInterval(Direction::Up, st, pr);
		}
	}
}

void Railway::insertInterval(int index, std::shared_ptr<RailStation> st)
{
	if (st->isDownVia()) {
		auto pr = leftDirStation(index, Direction::Down),
			nx = rightDirStation(index, Direction::Up);
		if (pr) {
			//前区间数据，相当于在append
            addInterval(Direction::Down, pr, st);
		}
		else {
			//后区间
            addInterval(Direction::Down, st, nx);
		}
	}
	if (st->isUpVia()) {   //上行方向
		auto pr = leftDirStation(index, Direction::Up),
			nx = rightDirStation(index, Direction::Up);
		if (pr) {
            addInterval(Direction::Up, st, pr);
		}
		if (nx) {
            addInterval(Direction::Up, nx, st);
		}
	}
}

void Railway::removeInterval(int index)
{
	auto st = _stations.at(index);
	if (st->isDownVia()) {
		auto pr = leftDirStation(index, Direction::Down),
			nx = rightDirStation(index, Direction::Down);
		if (pr && nx) {
			//区间合并
			auto it1 = st->downPrev, it2 = st->downNext;
			it1->mergeWith(*it2);
		}
		else if (pr) {
			//只有前区间，应当删除
			pr->downNext.reset();
		}
		else if (nx) {
			nx->downPrev.reset();
		}
	}
	if (st->isUpVia()) {
		auto pr = leftDirStation(index, Direction::Up),
			nx = rightDirStation(index, Direction::Up);
		if (pr && nx) {
			auto it1 = st->upPrev, it2 = st->upNext;
			it1->mergeWith(*it2);
		}
		else if (pr) {
			pr->upPrev.reset();
		}
		else if (nx) {
			nx->upNext.reset();
		}
	}
}

std::shared_ptr<RailStation> Railway::leftDirStation(int cur, Direction _dir) const
{
	for (int i = cur - 1; i >= 0; i--) {
		if (_stations.at(i)->isDirectionVia(_dir))
			return _stations.at(i);
	}
	return std::shared_ptr<RailStation>();
}

std::shared_ptr<RailStation> Railway::rightDirStation(int cur, Direction _dir) const
{
	for (int i = cur + 1; i < stationCount(); i++) {
		if (_stations.at(i)->isDirectionVia(_dir)) {
			return _stations.at(i);
		}
	}
	return std::shared_ptr<RailStation>();
}

std::shared_ptr<RailStation> Railway::leftBothStation(std::shared_ptr<RailStation> st)
{
	auto p = st->upNext;
	for (; p; p = p->nextInterval()) {
		if (p->toStation()->direction == PassedDirection::BothVia)
			return p->toStation();
	}
	qDebug() << "Railway::leftBothStation: WARNING: Unexpected null value." <<
		st->name << Qt::endl;
	return nullptr;
}

std::shared_ptr<RailStation> Railway::rightBothStation(std::shared_ptr<RailStation> st)
{
	auto p = st->upPrev;
	for (; p; p = p->prevInterval()) {
		if (p->fromStation()->direction == PassedDirection::BothVia)
			return p->fromStation();
	}
	qDebug() << "Railway::rightBothStation: WARNING: Unexpected null value. " <<
		st->name << Qt::endl;
	return nullptr;
}

std::shared_ptr<RailInterval> Railway::addInterval(Direction _dir, std::shared_ptr<RailStation> from, std::shared_ptr<RailStation> to)
{
    auto t = RailInterval::construct(_dir, from, to);
	t->_rulerNodes.reserve(_rulers.count());
	for (int i = 0; i < _rulers.count(); i++) {
        t->_rulerNodes.append(std::make_shared<RulerNode>(*_rulers[i], *t));
	}
    for(int i=0;i<_forbids.count();i++){
        t->_forbidNodes.append(std::make_shared<ForbidNode>(*_forbids[i],*t));
    }
    return t;
}

void Railway::initUpIntervals()
{
	if (empty())return;
	auto p = _stations.rbegin();
	for (; p != _stations.rend(); ++p) {
		if ((*p)->isUpVia())break;
	}
	auto pr = p;   // pr: 上一个上行通过的站
	for (++p; p != _stations.rend(); ++p) {
		if ((*p)->isUpVia()) {
			addInterval(Direction::Up, *pr, *p);
			pr = p;
		}
	}

}

std::shared_ptr<Forbid> Railway::addEmptyForbid(bool different)
{
    int n=_forbids.count();
	auto forbid = std::shared_ptr<Forbid>(new Forbid(*this, different, n));
    _forbids.append(forbid);
    auto p=firstDownInterval();
    for(;p;p=p->nextInterval()){
        p->_forbidNodes.append(std::make_shared<ForbidNode>(*forbid,*p));
    }
    p=firstUpInterval();
    for(;p;p=p->nextInterval()){
        p->_forbidNodes.append(std::make_shared<ForbidNode>(*forbid,*p));
    }
    return forbid;
}

std::shared_ptr<Forbid> Railway::addForbid(const QJsonObject &obj)
{
    bool diff=obj.value("different").toBool(true);
    auto forbid=addEmptyForbid(diff);
    const QJsonArray& ar=obj.value("nodes").toArray();
    for(auto p=ar.cbegin();p!=ar.cend();++p){
        const QJsonObject& obnode=p->toObject();
        const StationName
                & from=StationName::fromSingleLiteral( obnode.value("fazhan").toString()),
                & to=StationName::fromSingleLiteral(obnode.value("daozhan").toString());
        auto pnode=forbid->getNode(from,to);
        if(!pnode){
            qDebug()<<"Railway::addForbid: WARNING: invalid interval "<<
                      from<<"->"<<to<<Qt::endl;
        }else{
            pnode->fromJson(obnode);
        }
    }
    forbid->downShow=obj.value("downShow").toBool();
    forbid->upShow=obj.value("upShow").toBool();
    return forbid;
}

std::shared_ptr<RailInterval> Railway::nextIntervalCirc(std::shared_ptr<RailInterval> railint)
{
	auto t = railint->nextInterval();
	if (!t && railint->isDown()) {
		return firstUpInterval();
	}
    return t;
}

std::shared_ptr<const RailInterval> Railway::nextIntervalCirc(
        std::shared_ptr<const RailInterval> railint) const
{
    auto t = railint->nextInterval();
    if (!t && railint->isDown()) {
        return firstUpInterval();
    }
    return t;
}

std::shared_ptr<Forbid> Railway::addForbidFrom(std::shared_ptr<Forbid> other)
{
	return addForbidFrom(*other);
}

std::shared_ptr<Forbid> Railway::addForbidFrom(const Forbid& other)
{
	int idx = _forbids.count();
	auto forbid = std::shared_ptr<Forbid>(new Forbid(*this, other.different(), idx));
	forbid->downShow = other.downShow;
	forbid->upShow = other.upShow;
	_forbids.append(forbid);
	auto p = firstDownInterval();
	auto n = other.firstDownNode();
	for (; p && n; p = nextIntervalCirc(p), n = n->nextNodeCirc()) {
		p->_forbidNodes.append(std::make_shared<ForbidNode>(*forbid, *p, n->beginTime, n->endTime));
	}
	return forbid;
}

std::shared_ptr<RailInterval> Railway::intervalCircByIndex(int index)
{
	auto p = firstDownInterval();
	for (int i = 0; i < index; i++) {
		p = nextIntervalCirc(p);
		if (!p) {
			qDebug() << "Railway::intervalCircByIndex: WARNING: invalid index: "
				<< index << ", early terminate. " << Qt::endl;
			break;
		}
	}
	return p;
}

void Railway::filtRulerByCount(int minCount)
{
	int i = 0;
	while (i < _rulers.size()) {
		auto r = _rulers.at(i);
		if (r->validNodeCount() < minCount) {
			removeRuler(r);
		}
		else {
			i++;
		}
	}
}

void Railway::symmetrize()
{
	foreach(const auto & p, _stations) {
		p->direction = PassedDirection::BothVia;
	}
	initUpIntervals();
	foreach(const auto& ruler, _rulers) {
		ruler->copyDownToUp();
	}
	foreach(const auto& forbid, _forbids) {
		forbid->copyDownToUp();
	}
}

double Railway::calStationYValueByMile(const Config& config)
{
	for (auto& p : _stations) {
		p->y_value = p->mile * config.pixels_per_km;
	}
	if (!_stations.empty())
		_diagramHeight = _stations.last()->y_value.value();
	return _diagramHeight;
}

void Railway::clearYValues()
{
	for (auto& p : _stations) {
		p->y_value = std::nullopt;
	}
	_diagramHeight = -1;
}



