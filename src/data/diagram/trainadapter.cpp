#include "trainadapter.h"
#include <cassert>

#include "kernel/trainitem.h"

TrainAdapter::TrainAdapter(Train& train,
    Railway& railway, const Config& config):
    _railway(railway),_train(train)
{
	autoLines(config);
}


TrainAdapter& TrainAdapter::operator=(TrainAdapter&& another)
{
	assert(&_railway == &(another._railway));
	assert(&_train == &(another._train));
	_lines = std::move(another._lines);
	return *this;
}

void TrainAdapter::print() const
{
    qDebug() << "TrainAdapter: " << _train.trainName().full() << " @ " <<
        _railway.name() << ", lines: " << _lines.size() << Qt::endl;
	for (const auto& p : _lines) {
		p->print();
	}
}

void TrainAdapter::clearItems()
{
	for (auto& p : _lines) {
		p->setItem(nullptr);
	}
}

void TrainAdapter::highlightItems()
{
	for (auto p : _lines) {
		if (p->item())
			p->item()->highlight();
	}
}

void TrainAdapter::unhighlightItems()
{
	for (auto p : _lines) {
		if (p->item())
			p->item()->unhighlight();
	}
}

void TrainAdapter::autoLines(const Config& config)
{
	//命名规则：前缀r表示rail，t表示train
    auto& table = _train.timetable();
    auto& rail = _railway;    //alias
	//上一个绑定到的车站
    auto tlast = _train.nullStation();
	std::shared_ptr<RailStation> rlast{};
	Direction locdir = Direction::Undefined;    //当前站前区间的行别
	std::shared_ptr<TrainLine> line = std::make_shared<TrainLine>(*this);  //当前运行线。初始空

	int loccnt = 0;   //当前运行线的计数
	int tpass = 0;    //在train但不在rail中的站数

	//qDebug() << "TrainAdapter::autoLines: INFO: binding " << train().trainName().full() <<
	//	" @ " << rail.name() << Qt::endl;


	for (auto tcur = table.begin(); tcur != table.end(); ++tcur) {
        std::shared_ptr rcur = rail.stationByGeneralName(tcur->name);
		bool bound = false;   //本站是否成功绑定
		if (rcur) {
			if (!loccnt) {
				//第一站，这时候什么都不知道，直接绑定
				line->addStation(tcur, rcur);
				bound = true;
			}
			else if (loccnt == 1 && !rlast->isDirectionVia(rail.gapDirection(rlast, rcur))) {
				//首先判断是否要撤销第一站的绑定 （应当很少见的特殊情况）
				//第一个站是非法绑定，即它的行别不匹配 （例如上行经过下行单向站）
				//此时应当撤销上一站的绑定，当前站的绑定按照第一站的规则进行；但行别已经确定
				//注意这也是唯一一种允许locdir不匹配时绑定的情况
				qDebug() << "TrainAdapter::autoItem: WARNING: Invalid direction bound encountered" <<
					rlast->name << Qt::endl;
				line->_stations.pop_back();
				loccnt--;
			}
			else if (rcur->isDirectionVia(locdir = rail.gapDirection(rlast, rcur))) {  
				// !rlast  此前已经有过绑定
                if (rail.stationsBetween(rlast, rcur) + tpass >
					config.max_passed_stations) {
					//跨越区间数量超限，截断运行线
					if (loccnt >= 2) {  
						line->_dir = locdir;
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
