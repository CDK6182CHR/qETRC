#include "trainline.h"
#include "data/common/stationname.h"
#include "data/train/train.h"

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
