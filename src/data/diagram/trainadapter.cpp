#include "trainadapter.h"

TrainAdapter::TrainAdapter(std::shared_ptr<Train> train, 
	std::weak_ptr<Railway> railway, const Config& config):
	_train(train),_railway(railway)
{
	autoLines(config);
}

void TrainAdapter::print() const
{
	qDebug() << "TrainAdapter: " << _train->trainName().full() << " @ " <<
		_railway.lock()->name() << ", lines: " << _lines.size() << Qt::endl;
	for (const auto& p : _lines) {
		p->print();
	}
}

void TrainAdapter::autoLines(const Config& config)
{
	//命名规则：前缀r表示rail，t表示train
	auto& table = _train->timetable();
	std::shared_ptr<Railway> rail = _railway.lock();
	//上一个绑定到的车站
	auto tlast = _train->nullStation();
	std::shared_ptr<RailStation> rlast{};
	Direction locdir = Direction::Undefined;    //当前站前区间的行别
	std::shared_ptr<TrainLine> line = std::make_shared<TrainLine>();  //当前运行线。初始空

	int loccnt = 0;   //当前运行线的计数
	int tpass = 0;    //在train但不在rail中的站数

	for (auto tcur = table.begin(); tcur != table.end(); ++tcur) {
		std::shared_ptr rcur = rail->stationByGeneralName(tcur->name);
		bool bound = false;   //本站是否成功绑定
		if (rcur) {
			if (!loccnt) {
				//第一站，这时候什么都不知道，直接绑定
				line->addStation(tcur, rcur);
				bound = true;
			}
			else {  // !rlast  此前已经有过绑定
				if (rail->stationsBetween(rlast, rcur) + tpass >
					config.max_passed_stations) {
					//跨越区间数量超限，截断运行线
					if (loccnt >= 2) {  
						line->dir = locdir;
						_lines.append(line);
					}
					else {
						//上一段运行线被舍弃；但如果再之前一段是以折返方式结束的，
						//则这里需要补上标签。
						if (!line->startLabel) {
							_lines.last()->endLabel = true;
						}
					}
					//将当前站按照新运行线的第一站进行绑定
					line = std::make_shared<TrainLine>();
					loccnt = 0;
					locdir = Direction::Undefined;
					line->addStation(tcur, rcur);
					bound = true;
				}
				else {  //不是跨越区间数截断的情况
					Direction dir1 = rail->gapDirection(rlast, rcur);
					if (DirFunc::isValid(locdir) && DirFunc::isValid(dir1) &&
						locdir != dir1 && rcur->isDirectionVia(dir1)) {  //行别变化
						//注意新一段运行线要包含上一段的最后一个
						//注意必须是合法经过当前站 （rcur->isDirectionVia），否则直接忽略处理
						line->endLabel = false;
						_lines.append(line);
						//创建新的运行线
						line = std::make_shared<TrainLine>();
						line->startLabel = false;
						line->stations.push_back(_lines.last()->stations.back());   //copy construct
						loccnt = 1;  //这是上一站
						locdir = dir1;
						line->dir = locdir;
						line->addStation(tcur, rcur);
						bound = true;
					}
					else {  //其他情况
						//既没有跨越站数，也没有行别变化，正常绑定就好
						locdir = dir1;
						//这里有一个特殊情况  应当极少发生
						if (!rlast->isDirectionVia(locdir)) {
							//第一个站是非法绑定，即它的行别不匹配 （例如上行经过下行单向站）
							//此时应当撤销上一站的绑定，当前站的绑定按照第一站的规则进行；但行别已经确定
							qDebug() << "TrainAdapter::autoItem: WARNING: Invalid direction bound encountered" <<
								rlast->name << Qt::endl;
							line->stations.pop_back();
							loccnt--;
						}
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
