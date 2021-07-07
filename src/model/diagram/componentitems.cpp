#include "componentitems.h"

navi::DiagramItem::DiagramItem(Diagram& diagram) :
	AbstractComponentItem(0), _diagram(diagram),
	railList(std::make_unique<RailwayListItem>(diagram, this)),
	pageList(std::make_unique<PageListItem>(diagram, this)),
	trainList(std::make_unique<TrainListItem>(diagram.trainCollection(), this))
{
}

navi::AbstractComponentItem* navi::DiagramItem::child(int i)
{
	switch (i) {
	case RowRailways:return railList.get();
	case RowPages:return pageList.get();
	case RowTrains:return trainList.get();
	default:return nullptr;
	}
}

navi::RailwayListItem::RailwayListItem(Diagram& diagram, DiagramItem* parent):
	AbstractComponentItem(0,parent), _diagram(diagram)
{
	for (int i = 0; i < _diagram.railwayCount();i++) {
		auto p = _diagram.railways().at(i);
		_rails.emplace_back(std::make_unique<RailwayItem>(p, i, this));
	}
}

navi::AbstractComponentItem* navi::RailwayListItem::child(int i)
{
	if (i >= 0 && i < _rails.size())
		return _rails.at(i).get();
	return nullptr;
}

QString navi::RailwayListItem::data(int i) const
{
	switch (i) {
	case ColItemName:return QObject::tr("基线数据");
	case ColItemNumber:return QString::number(_diagram.railways().size());
	default:return "";
	}
}

void navi::RailwayListItem::appendRailways(const QList<std::shared_ptr<Railway>>& rails) 
{
	int row = _diagram.railwayCount();
	for (auto p = rails.begin(); p != rails.end(); ++p) {
		_diagram.railways().append(*p);
		_rails.emplace_back(std::make_unique<RailwayItem>(*p, row++, this));
	}
}

void navi::RailwayListItem::removeTailRailways(int cnt)
{
	for (int i = 0; i < cnt; i++) {
		_rails.pop_back();
		_diagram.railways().pop_back();
	}
}

navi::PageListItem::PageListItem(Diagram& diagram, DiagramItem* parent):
	AbstractComponentItem(1,parent),_diagram(diagram)
{
	int row = 0;
	for (auto p: _diagram.pages()) {
		_pages.emplace_back(std::make_unique<PageItem>(p, row++, this));
	}
}

navi::AbstractComponentItem* navi::PageListItem::child(int i)
{
	if (i >= 0 && i < _pages.size())
		return _pages.at(i).get();
	return nullptr;
}

QString navi::PageListItem::data(int i)const
{
	switch (i) {
	case ColItemName:return QObject::tr("运行图视窗");
	case ColItemNumber:return QString::number(_diagram.pages().size());
	default:return "";
	}
}

navi::AbstractComponentItem* navi::RailwayItem::child(int i)
{
	return nullptr;
}

navi::RailwayItem::RailwayItem(std::shared_ptr<Railway> rail, int row, RailwayListItem* parent):
	AbstractComponentItem(row,parent),_railway(rail)
{

}

QString navi::RailwayItem::data(int i) const
{
	switch (i) {
	case ColItemName:return _railway->name();
	default:return "";
	}
}

navi::PageItem::PageItem(std::shared_ptr<DiagramPage> page, int row, PageListItem* parent):
	AbstractComponentItem(row,parent),_page(page)
{

}

QString navi::PageItem::data(int i) const
{
	switch (i) {
	case ColItemName:return _page->name();
	case ColItemNumber:return QString::number(_page->railwayCount());
	case ColDescription:return _page->railNameString();
	default:return "";
	}
}

navi::TrainListItem::TrainListItem(TrainCollection& coll, DiagramItem* parent):
	AbstractComponentItem(2,parent),_coll(coll)
{
	int row = 0;
	for (auto p : _coll.trains()) {
		_trains.emplace_back(std::make_unique<TrainModelItem>(p, row++, this));
	}
}

QString navi::TrainListItem::data(int i) const
{
	switch (i) {
	case ColItemName:return QObject::tr("列车");
	case ColItemNumber:return QString::number(_trains.size());  
	default:return "";
	}
}

void navi::TrainListItem::resetChildren()
{
	_trains.clear();   //删除child
	int row = 0;
	for (auto p : _coll.trains()) {
		_trains.emplace_back(std::make_unique<TrainModelItem>(p, row++, this));
	}
}

navi::AbstractComponentItem* navi::TrainListItem::child(int i)
{
	if (i >= 0 && i < _trains.size())
		return _trains[i].get();
	return nullptr;
}

navi::TrainModelItem::TrainModelItem(std::shared_ptr<Train> train, int row, TrainListItem* parent):
	AbstractComponentItem(row,parent), _train(train)
{
}

QString navi::TrainModelItem::data(int i) const
{
	switch (i) {
	case ColItemName:return _train->trainName().full();
	case ColDescription:return _train->startEndString();
	default:return {};
	}
}
