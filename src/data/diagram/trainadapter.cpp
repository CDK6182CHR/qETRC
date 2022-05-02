#include "trainadapter.h"
#include <cassert>

#include "kernel/trainitem.h"
#include "util/utilfunc.h"
#include "config.h"
#include "data/rail/rulernode.h"


TrainAdapter::TrainAdapter(std::weak_ptr<Train> train,
    std::weak_ptr<Railway> railway, const Config& config):
    _railway(railway),_train(train)
{
	autoLines(config);
}


TrainAdapter& TrainAdapter::operator=(TrainAdapter&& another)noexcept
{
	assert(&_railway == &(another._railway));
	assert(&_train == &(another._train));
	_lines = std::move(another._lines);
	return *this;
}

void TrainAdapter::print() const
{
    qDebug() << "TrainAdapter: " << train()->trainName().full() << " @ " <<
        _railway.lock()->name() << ", lines: " << _lines.size() << Qt::endl;
	for (const auto& p : _lines) {
		p->print();
	}
}


AdapterEventList TrainAdapter::listAdapterEvents(const TrainCollection& coll) const
{
	AdapterEventList res;
	for (auto p : _lines) {
		//res.append(p->listLineEvents(coll));
		auto&& t = p->listLineEvents(coll);
		res.append(t);
	}
	return res;
}

const AdapterStation* TrainAdapter::lastStation() const
{
	if (_lines.empty())
		return nullptr;
	return _lines.last()->lastStation();
}

const AdapterStation* TrainAdapter::firstStation() const
{
	if (_lines.empty())
		return nullptr;
	return _lines.first()->firstStation();
}

int TrainAdapter::totalSecs() const
{
	int res = 0;
	for (auto p : _lines)
		res += p->totalSecs();
	return res;
}

std::pair<int,int> TrainAdapter::runStaySecs() const
{
	int run = 0, stay = 0;
	for (auto p : _lines) {
		auto&& d = p->runStaySecs();
		run += d.first;
		stay += d.second;
	}
	return std::make_pair(run, stay);
}

double TrainAdapter::totalMile() const
{
	double res = 0;
	for (auto p : _lines)
		res += p->totalMile();
	return res;
}

void TrainAdapter::setIsShow(bool on)
{
	for (auto p : _lines) {
		p->setIsShow(on);
	}
}

void TrainAdapter::setIsShowLineWise(Direction dir, bool on)
{
	for (auto p : _lines) {
		if (p->dir() == dir)
			p->setIsShow(on);
	}
}

Direction TrainAdapter::firstDirection() const
{
	return isNull() ? Direction::Undefined : _lines.first()->dir();
}

Direction TrainAdapter::lastDirection() const
{
	return isNull() ? Direction::Undefined : _lines.last()->dir();
}

bool TrainAdapter::isFirstStation(const AdapterStation* st) const
{
	if (isNull())return false;
	auto p = _lines.first();
	if (p->isNull())return false;
	return st->trainStation->name == train()->starting() &&
		st == &(p->stations().front());
}

bool TrainAdapter::isLastStation(const AdapterStation* st) const
{
	if (isNull())return false;
	auto p = _lines.last();
	if (p->isNull())return false;
	return st->trainStation->name == train()->terminal() &&
		st == &(p->stations().back());
}

std::pair<const AdapterStation*, std::shared_ptr<TrainLine>>
	TrainAdapter::stationByTrainLinear(Train::ConstStationPtr st) const
{
	const AdapterStation* p = nullptr;
	for (auto line : _lines) {
		if ((p = line->stationByTrainLinear(st)))
			return std::make_pair(p, line);
	}
    return std::make_pair(nullptr, nullptr);
}

int TrainAdapter::adapterStationCount() const
{
    int res=0;
    foreach(auto line,_lines){
        res+=line->stations().size();
    }
    return res;
}

void TrainAdapter::timetableInterpolation(std::shared_ptr<const Ruler> ruler, 
	bool toRailStart, bool toRailEnd, int prec)
{
	for (int i = 0; i < _lines.size(); i++) {
		auto line = _lines.at(i);
		bool toBegin = false, toEnd = false;
		if (i == 0) {
            toBegin = ((line->dir() == Direction::Down && toRailStart) ||
                (line->dir() == Direction::Up && toRailEnd));
		}
		if (i == _lines.size() - 1) {
            toEnd = ((line->dir() == Direction::Down && toRailEnd) ||
                (line->dir() == Direction::Up && toRailStart));
		}
		line->timetaleInterpolation(ruler, toBegin, toEnd, prec);
	}
}

double TrainAdapter::relativeError(std::shared_ptr<const Ruler> ruler) const
{
	int this_time = 0, error_time = 0;
	foreach(auto line, _lines) {
		if (line->isNull())continue;
		auto pr = line->stations().begin();
		auto p = std::next(pr);
		for (; p != line->stations().end(); pr = p, ++p) {
			int secs_real = qeutil::secsTo(pr->trainStation->depart, 
				p->trainStation->arrive);
			int secs_std = ruler->totalInterval(pr->railStation.lock(), p->railStation.lock(),
				line->dir());
			if (secs_std == -1)continue;
			if (line->hasStartAppend(pr)) {
				secs_std -= pr->railStation.lock()->dirNextInterval(line->dir())
					->getRulerNode(*ruler)->start;
			}
			if (line->hasStopAppend(p)) {
				secs_std -= p->railStation.lock()->dirPrevInterval(line->dir())
					->getRulerNode(*ruler)->stop;
			}
			this_time += secs_real;
			error_time += std::abs(secs_real - secs_std);
		}
	}
	return static_cast<double>(error_time) / this_time;
}

void TrainAdapter::autoLines(const Config& config)
{
	//命名规则：前缀r表示rail，t表示train
    auto& table = train()->timetable();
    auto rail = _railway.lock();    //alias
	//上一个绑定到的车站
    auto tlast = train()->nullStation();
	std::shared_ptr<RailStation> rlast{};
	Direction locdir = Direction::Undefined;    //当前站前区间的行别
	std::shared_ptr<TrainLine> line = std::make_shared<TrainLine>(*this);  //当前运行线。初始空

	int loccnt = 0;   //当前运行线的计数
	int tpass = 0;    //在train但不在rail中的站数

	//qDebug() << "TrainAdapter::autoLines: INFO: binding " << train().trainName().full() <<
	//	" @ " << rail.name() << Qt::endl;

	for (auto tcur = table.begin(); tcur != table.end(); ++tcur) {
        std::shared_ptr rcur = rail->stationByGeneralName(tcur->name);
		bool bound = false;   //本站是否成功绑定
		if (rcur) {
			if (!loccnt) {
				//第一站，这时候什么都不知道，直接绑定
				line->addStation(tcur, rcur);
				bound = true;
			}
			else if (loccnt == 1 && (!qeutil::directionIntersected(rlast->direction,rcur->direction) ||
				!rlast->isDirectionVia(rail->gapDirection(rlast, rcur))) ){
				//首先判断是否要撤销第一站的绑定 （应当很少见的特殊情况）
				//第一个站是非法绑定，即它的行别不匹配 （例如上行经过下行单向站）
				//此时应当撤销上一站的绑定，当前站的绑定按照第一站的规则进行；但行别已经确定
				//注意这也是唯一一种允许locdir不匹配时绑定的情况
				//2021.10.23注意：这里的行别判断只依据里程，并不可靠。
				//现在增加方向不交的判定条件
				qDebug() << "TrainAdapter::autoItem: WARNING: Invalid direction bound encountered" <<
					rlast->name << ", unbound previous station "
					<< line->_stations.back().trainStation->name.toSingleLiteral()
					<< ", for train " << train()->trainName().full() << Qt::endl;
				line->_stations.pop_back();
				loccnt--;
			}
			else if (rcur->isDirectionVia(locdir = rail->gapDirection(rlast, rcur))) {  
				if (rcur == rlast) {
					//如果本站和上一站是同一站...直接绑定
					//此时不对方向有任何判断
					line->addStation(tcur, rcur);
					bound = true;
				}
				// !rlast  此前已经有过绑定
                else if (rail->stationsBetween(rlast, rcur) + tpass >
					config.max_passed_stations) {
					//跨越区间数量超限，截断运行线
					if (loccnt >= 2) {  
						_lines.append(line);
					}
					else {
						//上一段运行线被舍弃；但如果再之前一段是以折返方式结束的，
						//则这里需要补上标签。
						if (!line->_startLabel) {
							_lines.last()->_endLabel = true;
						}
					}
					//将当前站按照新运行线的第一站进行绑定
					line = std::make_shared<TrainLine>(*this);
					loccnt = 0;
					locdir = Direction::Undefined;
					line->addStation(tcur, rcur);
					bound = true;
				}
				else {  //不是跨越区间数截断的情况
					
					if (DirFunc::isValid(line->_dir) && DirFunc::isValid(locdir) &&
						!qeutil::directionIntersected(rlast->direction, rcur->direction)) {
						// 2021.10.23新增：如果本站和上站的行别不交叉（例如下行单向站和上行单向站）
						// 则按照行别翻转处理，但新运行线不包含上一运行线终点。
						_lines.append(line);
						line = std::make_shared<TrainLine>(*this);
						// 因为是单向站，所以运行线方向可以直接断定
						locdir = qeutil::passedDirToDir(rcur->direction);
						loccnt = 0;   
						line->addStation(tcur, rcur);
						line->_dir = locdir;
						bound = true;
					}
					else if (DirFunc::isValid(line->_dir) && DirFunc::isValid(locdir) &&
						line->_dir != locdir && rcur->isDirectionVia(locdir)) {  //行别变化
						//注意新一段运行线要包含上一段的最后一个
						//注意必须是合法经过当前站 （rcur->isDirectionVia），否则直接忽略处理
						line->_endLabel = false;
						_lines.append(line);
						//创建新的运行线
						line = std::make_shared<TrainLine>(*this);
						line->_startLabel = false;
						line->_stations.push_back(_lines.last()->_stations.back());   //copy construct
						loccnt = 1;  //这是上一站
						line->_dir = locdir;
						line->addStation(tcur, rcur);
						bound = true;
					}
					else {  //其他情况
						//既没有跨越站数，也没有行别变化，正常绑定就好
						line->_dir = locdir;
						
						if (rcur->isDirectionVia(locdir)) {
							line->addStation(tcur, rcur);
							bound = true;
						}
					}
				}
			}
		}
		if (bound) {
			tlast = tcur;
			rlast = rcur;
			tpass = 0;
			loccnt++;
		}
		else {
			tpass++;
		}
	}
	//最后一段运行线
	if (loccnt >= 2) {
		_lines.append(line);
	}
}
