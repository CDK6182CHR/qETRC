#include "trainline.h"
#include "data/common/stationname.h"
#include "data/train/train.h"
#include "trainadapter.h"
#include "data/train/traincollection.h"
#include "data/train/train.h"
#include "data/rail/rail.h"
#include "util/utilfunc.h"

#include <QDebug>

//I/O部分暂不实现
#if 0
TrainLine::TrainLine(const QJsonObject& obj, Train& train)
{
	fromJson(obj, train);
}

void TrainLine::fromJson(const QJsonObject& obj, Train& train)
{
	StationName startname = StationName::fromSingleLiteral(obj.value("start").toString());
	StationName endname = StationName::fromSingleLiteral(obj.value("end").toString());
	bool started = false;   //强制要求end必须在start之后
	for (auto p = train.timetable().begin(); p != train.nullStation(); ++p) {
		if (!started && p->name == startname) {
			start = p;
			started = true;
		}
		//不知道有没有同一个站终止的极端情况。暂时不用else if
		if (started && p->name == endname) {
			end = p;
		}
	}
	dir = DirFunc::fromIsDown(obj.value("down").toBool());
	startLabel = obj.value("show_start_label").toBool();
	endLabel = obj.value("show_end_label").toBool();
}

QJsonObject TrainLine::toJson() const
{
	return QJsonObject{
		{"show",show},
		{"show_start_label",startLabel},
		{"show_end_label",endLabel},
		{"start",start->name.toSingleLiteral()},
		{"end",end->name.toSingleLiteral()}
	};
}
#endif

TrainLine::TrainLine(TrainAdapter& adapter) :
	_adapter(adapter), _dir(Direction::Undefined), _show(adapter.train()->isShow()),
	_startLabel(true), _endLabel(true)
{
}

void TrainLine::print() const
{
	qDebug() << "TrainLine  labels (" << _startLabel << ", " << _endLabel << "): ";
	qDebug() << train()->trainName().full() << " @ " << _adapter.railway().name() << Qt::endl;
	for (const auto& p : _stations) {
		qDebug() << *p.trainStation << " -> " << p.railStation.lock()->name << Qt::endl;
	}
}

std::shared_ptr<Train> TrainLine::train()
{
	return _adapter.train();
}

std::shared_ptr<const Train> TrainLine::train() const
{
	return _adapter.train();
}

bool TrainLine::startAtThis() const
{
	return !isNull() && train()->isStartingStation(_stations.front().trainStation->name);
}

bool TrainLine::endAtThis() const
{
	return !isNull() && train()->isTerminalStation(_stations.back().trainStation->name);
}

LineEventList TrainLine::listLineEvents(const TrainCollection& coll) const
{
	LineEventList res;
	res.reserve(_stations.size());
	for (int i = 0; i < _stations.size(); i++) {
		res.push_back(StationEventList());
	}

	//车站到开时刻
	listStationEvents(res);

	//与其他列车的互作用
	for (auto t : coll.trains()) {
		if (t == train())
			continue;
		for (auto adp : t->adapters()) {
			if (_adapter.isInSameRailway(*adp)) {
				for (auto line : adp->lines()) {
					if (line->dir() == dir()) {
						eventsWithSameDir(res, *line, *t);
					}
					else {
						eventsWithCounter(res, *line, *t);
					}
				}
			}
		}
	}
	return res;
}

int TrainLine::totalSecs() const
{
	if (isNull())
		return 0;
	const QTime& first = startLabel() ? _stations.front().trainStation->arrive :
		_stations.front().trainStation->depart;
	const QTime& last = _stations.back().trainStation->depart;
	int secs = first.secsTo(last);
	if (secs <= 0)
		secs += 24 * 3600;
	return secs;
}

std::pair<int, int> TrainLine::runStaySecs() const
{
	if (isNull())
		return std::make_pair(0, 0);
	else if (_stations.size() == 1) {
		return std::make_pair(0, qeutil::secsTo(_stations.front().trainStation->arrive,
			_stations.front().trainStation->depart));
	}
	int run = 0, stay = 0;
	auto p = _stations.begin();
	if (!isStartingStation(p) && startLabel()) {
		stay += p->trainStation->stopSec();
	}
	auto p0 = p; ++p;
	for (; p != _stations.end(); ++p) {
		//run
		run += qeutil::secsTo(p0->trainStation->depart, p->trainStation->arrive);
		if (!isTerminalStation(p))
			stay += p->trainStation->stopSec();
		p0 = p;
	}
	return std::make_pair(run, stay);
}

double TrainLine::totalMile() const
{
	if (isNull())
		return 0.0;
	return std::abs(_stations.back().railStation.lock()->mile -
		_stations.front().railStation.lock()->mile);
}

void TrainLine::listStationEvents(LineEventList& res) const
{
	int i = 0;
	auto p = _stations.begin();
	//先处理第一站，免得循环里面紧到写判断
	if (startLabel()) {
		if (startAtThis()) {
			//始发
			res[0].emplace(StationEvent(
				TrainEventType::Origination,
				p->trainStation->depart,
				p->railStation,
				std::nullopt
			));
		}
		else {
			if (p->trainStation->isStopped()) {
				//到达出发
				res[0].emplace(StationEvent(
					TrainEventType::Arrive,
					p->trainStation->arrive,
					p->railStation,
					std::nullopt
				));
				res[0].emplace(StationEvent(
					TrainEventType::Depart,
					p->trainStation->depart,
					p->railStation,
					std::nullopt
				));
			}
			else {
				res[0].emplace(StationEvent(
					TrainEventType::SettledPass,
					p->trainStation->arrive,
					p->railStation,
					std::nullopt
				));
			}
		}
	}
	detectPassStations(res, 0, p);
	auto lastIter = _stations.end(); --lastIter;

	//中间站的判断
	for (++p, ++i; p != lastIter; ++p, ++i) {
		auto ts = p->trainStation;
		auto rs = p->railStation;
		if (ts->isStopped()) {
			//到达出发
			res[i].emplace(StationEvent(
				TrainEventType::Arrive, ts->arrive, rs, std::nullopt
			));
			res[i].emplace(StationEvent(
				TrainEventType::Depart, ts->depart, rs, std::nullopt
			));
		}
		else {
			//通过
			res[i].emplace(StationEvent(
				TrainEventType::SettledPass, ts->arrive, rs, std::nullopt
			));
		}
		detectPassStations(res, i, p);
	}

	//最后一站，判断是不是终到站
	auto rs = p->railStation;
	auto ts = p->trainStation;
	if (endAtThis()) {
		res[i].emplace(StationEvent(
			TrainEventType::Destination, ts->depart, rs, std::nullopt
		));
	}
	else {
		if (ts->isStopped()) {
			res[i].emplace(StationEvent(
				TrainEventType::Arrive, ts->arrive, rs, std::nullopt
			));
			res[i].emplace(StationEvent(
				TrainEventType::Depart, ts->depart, rs, std::nullopt
			));
		}
		else {
			res[i].emplace(StationEvent(
				TrainEventType::SettledPass, ts->depart, rs, std::nullopt
			));
		}
	}
}

void TrainLine::detectPassStations(LineEventList& res, int index, ConstAdaPtr itr) const
{
	ConstAdaPtr right = itr; ++right; 
	if (right == _stations.end())return;
	auto rs0 = itr->railStation.lock(), rsn = right->railStation.lock();
	auto ts0 = itr->trainStation, tsn = right->trainStation;
	if (rs0->isAdjacentWith(rsn)) {
		//区间没有跨过车站，啥都不用干
		return;
	}
	//以下依赖于纵坐标！
	double y0 = rs0->y_value.value(), yn = rsn->y_value.value();
	double dy = yn - y0;
	int ds = ts0->depart.secsTo(tsn->arrive);   //区间时间，秒数
	if (ds <= 0)ds += 24 * 3600;
	if (y0 == yn) {
		//这个情况很吊诡，应该不会存在。安全起见，特殊处理
		return;
	}

	for (auto p = rs0->dirNextInterval(dir()); 
		p && p->toStation() != rsn; p = p->nextInterval()) {
		auto rsi = p->toStation();
		double yi = rsi->y_value.value();
		double rate = (yi - y0) / dy;
		double dsif = ds * rate;
		int dsi = int(std::round(dsif));
		res[index].emplace(StationEvent(
			TrainEventType::CalculatedPass, ts0->depart.addSecs(dsi),
			rsi, std::nullopt, QObject::tr("推算")
		));
	}
}

void TrainLine::eventsWithSameDir(LineEventList& res, const TrainLine& another, 
	const Train& antrain) const
{
	//注意：间隔12小时的站内事件判定可能会有问题！可能要先判定交集
	if (std::max(yMin(), another.yMin()) >= std::min(yMax(), another.yMax())) {
		//提前终止条件，y范围根本不相交，不用搞
		return;
	}

	auto pme = _stations.begin(), phe = another._stations.begin();
	ConstAdaPtr mylast = _stations.begin(), hislast = another._stations.begin();   //上一站的迭代器
	int index = 0;   //自己的站序下标，用来插入结果的

	int ylast = -2, xlast = 0;    //记载上一站比较结果  -2为起始标志
	double xcond, ycond;

	//后面的站 类似merge过程
	while (pme != _stations.end() && phe != another._stations.end()) {
		ycond = yComp(pme, phe);
		if (ylast==-2) {
			//和上一次访问在同一个区间，啥都不用干，继续
			ylast = ycond;
			sameDirStep(ycond, pme, phe, mylast, hislast, index);
			continue;
		}
		//先判定上一个区间有没有发生什么事情。注意此时两个都必定是直线段
		if (pme != mylast && phe != hislast) {
			auto pint = findIntervalIntersectionSameDir(mylast, pme, hislast, phe);
			if (pint.has_value()) {
				addIntervalEvent(res, index - 1, std::get<2>(pint.value()), std::get<1>(pint.value()),
					*mylast, *pme, std::cref(antrain), std::get<0>(pint.value()),
					QObject::tr("区间越行??"));
			}
		}

		auto tme = pme->trainStation, the = phe->trainStation;
		auto rme = pme->railStation.lock(), rhe = phe->railStation.lock();
		//判断站内有没有发生什么事情 这个复杂一点
		//麻烦在：可能出现一趟车站内停车，另一趟车通过但这里没有停点的情况
		if (ycond == 0 && (index!=0||startLabel())) {
			//本站时刻表是重合的，那很简单，跟第一站写的那个一样
			if (tme->stopRangeIntersected(*the)) {
				xlast = xComp(tme->arrive, the->arrive);
				xcond = xComp(tme->depart, the->depart);
				if (xcond == xlast) {
					//到开比较情况一致，只有重合需要说明一下
					if (xcond == 0) {
						res[index].emplace(StationEvent(
							TrainEventType::Coincidence, tme->arrive, pme->railStation,
							std::cref(antrain), QObject::tr("站内共线")
						));
					}
				}
				else if (notStartOrEnd(another, pme, phe)) {
					//存在始发终到站的情况，不允许踩
					if (xcond > xlast) {
						//规定：第一次碰到0时不处理
						if (xcond > 0) {
							//比别人发车晚，被踩
							res[index].emplace(StationEvent(
								TrainEventType::Avoid, the->depart, pme->railStation,
								std::cref(antrain)
							));
						}
					}
					else {  // xcond < xlast
						if (xcond < 0) {
							//比别人发车早，踩了别人
							res[index].emplace(StationEvent(
								TrainEventType::OverTaking, tme->depart, pme->railStation,
								std::cref(antrain)
							));
						}
					}
				}
			}
			ylast = ycond;
		}
		else {
			if (ycond > 0 && dir() == Direction::Down ||
				ycond < 0 && dir() == Direction::Up) {
				//本次列车领先，推定出在前面那个站有无交叉
				if (pme != _stations.begin()) {
					int passedTime = getPrevousPassedTime(pme, rhe);
					if (the->timeInStoppedRange(passedTime)&&
						!another.isStartingOrTerminal(phe)) {
						//本次列车在上一站踩了它  改成站内事件
						//res[index - 1].emplace(IntervalEvent(
						//	TrainEventType::OverTaking,
						//	QTime::fromMSecsSinceStartOfDay(passedTime),
						//	*mylast, *pme, std::cref(antrain), rhe->mile,
						//	QObject::tr("推定")
						//));
						res[index - 1].emplace(StationEvent(
							TrainEventType::OverTaking,
							QTime::fromMSecsSinceStartOfDay(passedTime),
							rhe, std::cref(antrain), QObject::tr("推定")
						));
						//qDebug() << ycond;
						//qDebug() << "推定越行 " << mylast->trainStation->name << ", " <<
						//	pme->trainStation->name;
						//qDebug() << "HIS: " << hislast->trainStation->name << ", " <<
						//	phe->trainStation->name << ", at " << rhe->name;
					}
				}
			}
			else if (!isStartingOrTerminal(pme)) {
				//本次列车落后，推定对方在本次列车这个站
				if (phe != another._stations.begin() && (index != 0 || startLabel())) {
					int passedTime = getPrevousPassedTime(phe, rme);
					if (tme->timeInStoppedRange(passedTime)) {
						//本次列车在本站被踩
						res[index].emplace(StationEvent(
							TrainEventType::Avoid, QTime::fromMSecsSinceStartOfDay(passedTime),
							rme, std::cref(antrain), QObject::tr("推定")
						));
					}
				}
				else {
					//qDebug() << "TrainLine::eventsWithSameDir: WARNING: "
					//	<< "Invalid begin() of iterator encountered at station: "
					//	<< stationString(*pme) << Qt::endl;
				}
			}
			ylast = ycond;
		}
		sameDirStep(ycond, pme, phe, mylast, hislast, index);
	}
}

#define SHOW_INTERVALS do{\
qDebug() << "INTERVAL: [" << mylast->trainStation->name << " - "\
<< pme->trainStation->name<<"] vs. [" << hislast->trainStation->name\
<< " - " << phe->trainStation->name << "] @ " << antrain.trainName().full();\
}while(false)

void TrainLine::eventsWithCounter(LineEventList& res, const TrainLine& another, const Train& antrain) const
{
	//!!注意counter的xcond意义和同向的不同
	if (std::max(yMin(), another.yMin()) >= std::min(yMax(), another.yMax())) {
		//提前终止条件，y范围根本不相交，不用搞
		return;
	}

	//qDebug() << "event with counter: " << antrain.trainName().full() << Qt::endl;

	auto pme = _stations.begin();
	auto phe = another._stations.rbegin();  //反迭代器
	ConstAdaPtr mylast = _stations.begin();
	auto hislast = another._stations.rbegin();   //上一站的迭代器
	int index = 0;   //自己的站序下标，用来插入结果的

	int ylast = -2, xlast = 0;    //记载上一站比较结果  注意其实只有ylast有用  -2为开始标志
	int ycond;
	

	//后面的站 类似merge过程
	while (pme != _stations.end() && phe != another._stations.rend()) {
		ycond = yComp(pme, phe);
		if (ylast==-2) {
			//和上一次访问在同一个区间，或者第一次。
			//并不总是能直接跳过；还是可能会发生一些事情。
			//previous: if (ylast==-2 || ycond == ylast) && ycond != 0
			ylast = ycond;
			sameDirStep(ycond, pme, phe, mylast, hislast, index);
			continue;
		}

		//先判定上一个区间有没有发生什么事情。注意此时两个都必定是直线段
		if (pme != _stations.begin() && phe != another._stations.rbegin()) {
			auto pint = findIntervalIntersectionCounter(mylast, pme, hislast, phe);
			if (pint.has_value()) {
				addIntervalEvent(res, index - 1, std::get<2>(pint.value()), std::get<1>(pint.value()),
					*mylast, *pme, std::cref(antrain), std::get<0>(pint.value()));
			}
		}

		auto tme = pme->trainStation, the = phe->trainStation;
		auto rme = pme->railStation.lock(), rhe = phe->railStation.lock();
		//判断站内有没有发生什么事情 这个复杂一点
		//麻烦在：可能出现一趟车站内停车，另一趟车通过但这里没有停点的情况
		if (ycond == 0 && (index != 0 || startLabel())) {
			//本站时刻表是重合的，那很简单，跟第一站写的那个一样
			auto tme = pme->trainStation, the = phe->trainStation;
			xlast = xComp(tme->arrive, the->arrive);

			if (xlast == 0) {
				//同时到站，直接判定会车
				//会车时刻：this到达的时刻
				res[index].emplace(StationEvent(
					TrainEventType::Meet, tme->arrive, pme->railStation, std::cref(antrain)
				));
			}
			else if (xlast < 0) {
				//他来的时候我还没走，发生会车
				if (xComp(the->arrive, tme->depart) <= 0) {
					//会车时刻：他到达的时刻
					res[index].emplace(StationEvent(
						TrainEventType::Meet, the->arrive, pme->railStation, std::cref(antrain)
					));
				}
			}
			else {  //xlast > 0
				//我到的时候他还没走
				if (xComp(tme->arrive, the->depart) <= 0) {
					//会车时刻：我到达的时刻
					res[index].emplace(StationEvent(
						TrainEventType::Meet, tme->arrive, pme->railStation, std::cref(antrain)
					));
				}
			}
			ylast = ycond;
		}
		else {
			if (ycond > 0 && dir() == Direction::Down ||
				ycond < 0 && dir() == Direction::Up) {
				//本次列车领先，推定出在前面那个站有无交叉
				//注意单向站。判断一下对方那个站是不是本次列车方向也经过的
				if (rhe->isDirectionVia(dir())) {
					if (pme!=_stations.begin()) {
						int passedTime = getPrevousPassedTime(pme, rhe);
						if (the->timeInStoppedRange(passedTime)) {
							//新增ycond!=ylast条件，保证这个区间内前后关系变了
							//本次列车在上一站踩了它
							//res[index - 1].emplace(IntervalEvent(
							//	TrainEventType::Meet,
							//	QTime::fromMSecsSinceStartOfDay(passedTime),
							//	*mylast, *pme, std::cref(antrain), rhe->mile,
							//	QObject::tr("推定")
							//));
							res[index - 1].emplace(StationEvent(
								TrainEventType::Meet,
								QTime::fromMSecsSinceStartOfDay(passedTime),
								rhe, std::cref(antrain), QObject::tr("推定")
							));
						}
					}
					else {
						//这是不正常的
						//qDebug() << "TrainLine::eventsWithCounter: WARNING: "
						//	<< "Invalid begin() of iterator encountered at station: "
						//	<< stationString(*pme) << " with " 
						//	<< another.stationString(*phe) << Qt::endl;
					}
				}
				else { //对方这个站本次列车不通过 没啥意义
					//但似乎也不用特殊处理？
				}
			}
			else {
				//本次列车落后，推定对方在本次列车这个站
				if (rme->isDirectionVia(another.dir())&& (index != 0 || startLabel())) {
					if (phe != another._stations.rbegin()) {
						int passedTime = getPrevousPassedTime(phe, rme);
						if (tme->timeInStoppedRange(passedTime)) {
							//本次列车在本站被踩
							res[index].emplace(StationEvent(
								TrainEventType::Meet, QTime::fromMSecsSinceStartOfDay(passedTime),
								rme, std::cref(antrain), QObject::tr("推定")
							));
						}
					}
					else {
						//qDebug() << "TrainLine::eventsWithCounter: WARNING: "
						//	<< "Invalid rbegin() of iterator encountered at station: "
						//	<< another.stationString(*phe) << " with " <<
						//	stationString(*pme) << Qt::endl;
					}
				}
				
			}
			ylast = ycond;
		}
		sameDirStep(ycond, pme, phe, mylast, hislast, index);
	}
}

double TrainLine::yMin()const
{
	if (dir() == Direction::Down) {
		return firstRailStation()->y_value.value();
	}
	else {
		return lastRailStation()->y_value.value();
	}
}

double TrainLine::yMax() const
{
	if (dir() == Direction::Down) {
		return lastRailStation()->y_value.value();
	}
	else {
		return firstRailStation()->y_value.value();
	}
}


int TrainLine::xComp(const QTime& tm1, const QTime& tm2) const
{
	int x1 = tm1.msecsSinceStartOfDay(), x2 = tm2.msecsSinceStartOfDay();
	int res = 0;
	if (x1 < x2)
		res = -1;
	else if (x1 > x2)
		res = +1;
	//PBC条件：选择两者之间间隔较小的结果
	if (std::abs(x1 - x2) > 24 * 3600 * 1000 / 2) {
		return -res;
	}
	return res;
}


std::optional<std::tuple<double, QTime, TrainEventType>>
	TrainLine::findIntervalIntersectionSameDir(ConstAdaPtr mylast, ConstAdaPtr mythis, 
		ConstAdaPtr hislast, ConstAdaPtr histhis) const
{
	auto rm1 = mylast->railStation.lock(), rm2 = mythis->railStation.lock();
	auto rh1 = hislast->railStation.lock(), rh2 = histhis->railStation.lock();

	//y值：是直接确定的
	double ym1 = rm1->y_value.value(), ym2 = rm2->y_value.value();
	double yh1 = rh1->y_value.value(), yh2 = rh2->y_value.value();

	//x值，按照msecs表示
	double xm1 = mylast->trainStation->depart.msecsSinceStartOfDay(),
		xm2 = mythis->trainStation->arrive.msecsSinceStartOfDay();
	double xh1 = hislast->trainStation->depart.msecsSinceStartOfDay(),
		xh2 = histhis->trainStation->arrive.msecsSinceStartOfDay();

	static constexpr int msecsOfADay = 24 * 3600 * 1000;

	//x的表示：如果后一个时刻小于前一个时刻，加上一天。
	if (xm2 < xm1)xm2 += msecsOfADay;
	if (xh2 < xh1)xh2 += msecsOfADay;

	//剪枝：首先判定x坐标不交叉条件，直接再见
	if (std::max(xm1, xh1) > std::min(xm2, xh2)) {
		return std::nullopt;
	}

	//直接采用直线的一般方程
	// A*x + B*y + C = 0
	double a1 = 0, b1 = 0, c1 = 0, a2 = 0, b2 = 0, c2 = 0;
	//本次列车区间运行线解析式
	if (xm1 != xm2) {
		//用点斜式： k*x - y + (y0-k*x0) = 0
		double k = (ym2 - ym1) / (xm2 - xm1);   //斜率
		a1 = k; b1 = -1; c1 = ym1 - k * xm1;
	}
	else {
		//垂直于x轴，表达式为x=xm1
		a1 = 1; b1 = 0; c1 = xm1;
	}
	
	//对方列车区间运行线解析式
	if (xh1 != xh2) {
		double k = (yh2 - yh1) / (xh2 - xh1);
		a2 = k; b2 = -1; c2 = yh1 - k * xh1;
	}
	else {
		a2 = 1; b2 = 0, c2 = xh1;
	}

	//平行条件  A1*B2 - A2*B1 = 0
	if (a1 * b2 == a2 * b1) {
		//平行条件
		if (b1 * c2 == b2 * c1) {
			//重合条件 -- 标记为区间共线运行，时刻为起点时刻中较为靠后的
			int tm_msec;
			double mile;
			if (xm1 <= xh1) {
				//把当前运行线的区间起点作为标记起点
				tm_msec = xm1;
				mile = rm1->mile;
			}
			else {
				tm_msec = xh1;
				mile = rh1->mile;
			}
			return std::make_tuple(mile, QTime::fromMSecsSinceStartOfDay(tm_msec % msecsOfADay),
				TrainEventType::Coincidence);
		}
		else {
			return std::nullopt;
		}
	}

	double xinter, yinter;   //交点坐标
	//不平行，两条直线总可以找到交点  
	if (a1 == 0) {
		//很特殊的情况，方程1直接没了
		yinter = -c1 / b1;
		xinter = (-c2 - b2 * yinter) / a2;
	}
	else {
		//Gauss消元法：r2 = r1 * A2/A1
		double b2p = b2 - a2 * b1 / a1;   //不可能等于0，否则矩阵的秩就不对头了
		yinter = (-c2 + a2 * c1 / a1) / b2p;
		xinter = (-c1 - b1 * yinter) / a1;
	}

	//2021.07.06 由于12小时问题, x判定可能并不安全，因此还是采用y判定
	double yhmin = yh1, yhmax = yh2;
	if (yhmin > yhmax)std::swap(yhmin, yhmax);
	double ymmin = ym1, ymmax = ym2;
	if (ymmin > ymmax)std::swap(ymmin, ymmax);

	//判定交点是否在合理范围内
	if (ymmin <= yinter && yinter <= ymmax &&
		yhmin <= yinter && yinter <= yhmax) {
		//合法交点  在本次列车的运行线上算出里程
		double mile;
		if (b1 == 0)mile = rm1->mile;   //斜率无穷大，没得算
		else {
			mile = (rm2->mile - rm1->mile) * (yinter - ym1) / (ym2 - ym1) + rm1->mile;
		}
		QTime&& tm = QTime::fromMSecsSinceStartOfDay(int(round(xinter)) % msecsOfADay);
		if (b1 == 0 || std::abs((ym2 - ym1) / (xm2 - xm1)) >
			std::abs((yh2 - yh1) / (xh2 - xh1))) {
			//本次列车的斜率绝对值比对方来的大，是越行
			return std::make_tuple(mile, tm, TrainEventType::OverTaking);
		}
		else {
			//本次列车被踩
			return std::make_tuple(mile, tm, TrainEventType::Avoid);
		}
	}
	else 
		return std::nullopt;
}

std::optional<std::tuple<double, QTime, TrainEventType>> 
	TrainLine::findIntervalIntersectionCounter(ConstAdaPtr mylast, ConstAdaPtr mythis, 
		std::deque<AdapterStation>::const_reverse_iterator hislast, 
		std::deque<AdapterStation>::const_reverse_iterator histhis) const
{
	auto rm1 = mylast->railStation.lock(), rm2 = mythis->railStation.lock();
	auto rh1 = hislast->railStation.lock(), rh2 = histhis->railStation.lock();

	//y值：是直接确定的
	double ym1 = rm1->y_value.value(), ym2 = rm2->y_value.value();
	double yh1 = rh1->y_value.value(), yh2 = rh2->y_value.value();

	//x值，按照msecs表示
	double xm1 = mylast->trainStation->depart.msecsSinceStartOfDay(),
		xm2 = mythis->trainStation->arrive.msecsSinceStartOfDay();
	//注意：对向车用的是反迭代器！
	double xh1 = hislast->trainStation->arrive.msecsSinceStartOfDay(),
		xh2 = histhis->trainStation->depart.msecsSinceStartOfDay();

	static constexpr int msecsOfADay = 24 * 3600 * 1000;

	//x的表示：如果后一个时刻小于前一个时刻，加上一天。
	if (xm2 < xm1) {
		xm2 += msecsOfADay;
	}
		
	if (xh1 < xh2) {
		xh1 += msecsOfADay;
	}

	//以下有：xh1 >= xh2
		

	//剪枝：首先判定x坐标不交叉条件，直接再见
	//再次注意xh1和xh2的大小关系！
	if (std::max(xm1, xh2) > std::min(xm2, xh1)) {
		return std::nullopt;
	}

	//直接采用直线的一般方程
	// A*x + B*y + C = 0
	double a1 = 0, b1 = 0, c1 = 0, a2 = 0, b2 = 0, c2 = 0;
	//本次列车区间运行线解析式
	if (xm1 != xm2) {
		//用点斜式： k*x - y + (y0-k*x0) = 0
		double k = (ym2 - ym1) / (xm2 - xm1);   //斜率
		a1 = k; b1 = -1; c1 = ym1 - k * xm1;
	}
	else {
		//垂直于x轴，表达式为x=xm1
		a1 = 1; b1 = 0; c1 = xm1;
	}

	//对方列车区间运行线解析式
	if (xh1 != xh2) {
		double k = (yh2 - yh1) / (xh2 - xh1);
		a2 = k; b2 = -1; c2 = yh1 - k * xh1;
	}
	else {
		a2 = 1; b2 = 0, c2 = xh1;
	}

	//qDebug() << "Equations: " << a1 << ", " << b1 << ", " << c1 << "; "
	//	<< a2 << ", " << b2 << ", " << c2 << Qt::endl;

	//平行条件  A1*B2 - A2*B1 = 0  按道理在对向中不会发生
	if (a1 * b2 == a2 * b1) {
		//平行条件
		if (b1 * c2 == b2 * c1) {
			//重合条件 -- 标记为区间共线运行，时刻为起点时刻中较为靠后的
			int tm_msec;
			double mile;
			if (xm1 <= xh1) {
				//把当前运行线的区间起点作为标记起点
				tm_msec = xm1;
				mile = rm1->mile;
			}
			else {
				tm_msec = xh1;
				mile = rh1->mile;
			}
			return std::make_tuple(mile, QTime::fromMSecsSinceStartOfDay(tm_msec % msecsOfADay),
				TrainEventType::Coincidence);
		}
		else {
			return std::nullopt;
		}
	}

	double xinter, yinter;   //交点坐标
	//不平行，两条直线总可以找到交点  
	if (a1 == 0) {
		//很特殊的情况，方程1直接没了
		yinter = -c1 / b1;
		xinter = (-c2 - b2 * yinter) / a2;
	}
	else {
		//Gauss消元法：r2 = r1 * A2/A1
		double b2p = b2 - a2 * b1 / a1;   //不可能等于0，否则矩阵的秩就不对头了
		yinter = (-c2 + a2 * c1 / a1) / b2p;
		xinter = (-c1 - b1 * yinter) / a1;
	}

	double yhmin = yh1, yhmax = yh2;
	if (yhmin > yhmax)std::swap(yhmin, yhmax);
	double ymmin = ym1, ymmax = ym2;
	if (ymmin > ymmax)std::swap(ymmin, ymmax);

	//判定交点是否在合理范围内。用x判定，因为x的大小关系是明确的
	if (yhmin <= yinter && yinter <= yhmax &&
		ymmin <= yinter && yinter <= ymmax) {
		//合法交点  在本次列车的运行线上算出里程
		double mile;
		if (b1 == 0)mile = rm1->mile;   //斜率无穷大，没得算
		else {
			mile = (rm2->mile - rm1->mile) * (yinter - ym1) / (ym2 - ym1) + rm1->mile;
		}
		QTime&& tm = QTime::fromMSecsSinceStartOfDay(int(round(xinter)) % msecsOfADay);
		return std::make_tuple(mile, tm, TrainEventType::Meet);
	}
	else
		return std::nullopt;
}

void TrainLine::addIntervalEvent(LineEventList& res, int index, TrainEventType type,
	const QTime& time, const AdapterStation& former, 
	const AdapterStation& latter, 
	std::reference_wrapper<const Train> another, double mile, const QString& note) const
{
	auto rfor = former.railStation.lock(), rlat = latter.railStation.lock();
	if (rfor->dirAdjacent(dir()) != rlat) {
		//非相邻站，收缩区间
		//现在假定缺站情况不严重，线性地往中间搜索
		auto r1 = rfor->dirAdjacent(dir());   //临时变量
		while (r1 && mileBeforeEq(r1, mile)) {
			rfor = r1; r1 = r1->dirAdjacent(dir());
		}
		auto invdir = DirFunc::reverse(dir());
		r1 = rlat->dirAdjacent(invdir);
		while (r1 && mileAfterEq(r1, mile)) {
			rlat = r1; r1 = r1->dirAdjacent(invdir);
		}
		//现在：rfor, rlat是收缩后的区间界限
	}
	decltype(rfor) rin{};   //如果站内事件，用这个
	if (rfor->mile == mile)
		rin = rfor;
	else if (rlat->mile == mile)
		rin = rlat;
	if (rin) {
		//判定为站内事件
		res[index].emplace(StationEvent(type, time, rin, another, note));
	}
	else {
		res[index].emplace(IntervalEvent(type, time, rfor, rlat, another, mile, note));
	}
}

bool TrainLine::mileBeforeEq(std::shared_ptr<const RailStation> st, double mile) const
{
	return dir() == Direction::Down ?
		st->mile <= mile :
		st->mile >= mile;
}

bool TrainLine::mileAfterEq(std::shared_ptr<const RailStation> st, double mile) const
{
	return dir() == Direction::Down ?
		st->mile >= mile :
		st->mile <= mile;
}

QString TrainLine::stationString(const AdapterStation& st) const
{
	return st.trainStation->name.toSingleLiteral() + " [" +
		train()->trainName().full() + " @ " + _adapter.railway().name() + "]";
}

bool TrainLine::notStartOrEnd(const TrainLine& another, ConstAdaPtr pme, ConstAdaPtr phe)const
{
	return !isStartingStation(pme) && !isTerminalStation(pme) &&
		!another.isStartingStation(phe) && !another.isTerminalStation(phe);
}

bool TrainLine::isStartingStation(ConstAdaPtr st) const
{
	return startAtThis() && st == _stations.begin();
}

bool TrainLine::isTerminalStation(ConstAdaPtr st) const
{
	ConstAdaPtr last = _stations.end(); --last;
	return endAtThis() && st == last;
}

std::deque<AdapterStation>::const_iterator TrainLine::stationFromYValue(double y) const
{
	if (dir() == Direction::Down)
		return std::lower_bound(_stations.begin(), _stations.end(), y);
	else {
		return std::lower_bound(_stations.rbegin(), _stations.rend(), y).base();
	}
}

bool AdapterStation::operator<(double y) const
{
	return railStation.lock()->y_value.value() < y;
}

bool operator<(double y, const AdapterStation& adp)
{
	return y < adp.railStation.lock()->y_value.value();
}
