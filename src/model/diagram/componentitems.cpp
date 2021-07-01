#include "componentitems.h"

DiagramItem::DiagramItem(Diagram& diagram):
	AbstractComponentItem(0),_diagram(diagram),
	railList(std::make_unique<RailwayListItem>(diagram,this)),
		pageList(std::make_unique<PageListItem>(diagram,this))
{
}

AbstractComponentItem* DiagramItem::child(int i)
{
	switch (i) {
	case RowRailways:return railList.get();
	case RowPages:return pageList.get();
	default:return nullptr;
	}
}

RailwayListItem::RailwayListItem(Diagram& diagram, DiagramItem* parent):
	AbstractComponentItem(0,parent), _diagram(diagram)
{
	for (int i = 0; i < _diagram.railwayCount();i++) {
		auto p = _diagram.railways().at(i);
		_rails.emplace_back(std::make_unique<RailwayItem>(p, i, this));
	}
}

AbstractComponentItem* RailwayListItem::child(int i)
{
	if (i >= 0 && i < _rails.size())
		return _rails.at(i).get();
	return nullptr;
}

QString RailwayListItem::data(int i) const
{
	switch (i) {
	case ColItemName:return QObject::tr("基线数据");
	case ColItemNumber:return QString::number(_diagram.railways().size());
	default:return "";
	}
}

PageListItem::PageListItem(Diagram& diagram, DiagramItem* parent):
	AbstractComponentItem(1,parent),_diagram(diagram)
{
	int row = 0;
	for (auto p: _diagram.pages()) {
		_pages.emplace_back(std::make_unique<PageItem>(p, row++, this));
	}
}

AbstractComponentItem* PageListItem::child(int i)
{
	if (i >= 0 && i < _pages.size())
		return _pages.at(i).get();
	return nullptr;
}

QString PageListItem::data(int i)const 
{
	switch (i) {
	case ColItemName:return QObject::tr("运行图视窗");
	case ColItemNumber:return QString::number(_diagram.pages().size());
	default:return "";
	}
}

AbstractComponentItem* RailwayItem::child(int i)
{
	return nullptr;
}

RailwayItem::RailwayItem(std::shared_ptr<Railway> rail, int row, RailwayListItem* parent):
	AbstractComponentItem(row,parent),_railway(rail)
{

}

QString RailwayItem::data(int i) const
{
	switch (i) {
	case ColItemName:return _railway->name();
	default:return "";
	}
}

PageItem::PageItem(std::shared_ptr<DiagramPage> page, int row, PageListItem* parent):
	AbstractComponentItem(row,parent),_page(page)
{

}

QString PageItem::data(int i) const
{
	switch (i) {
	case ColItemName:return _page->name();
	case ColItemNumber:return QString::number(_page->railwayCount());
	case ColDescription:return _page->railNameString();
	default:return "";
	}
}
