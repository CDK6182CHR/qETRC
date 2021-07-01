#pragma once

#include <memory>
#include <deque>
#include "data/diagram/diagram.h"

/**
 * Diagram中资源的封装类 （基类）
 * 注意数据发生变化时，需要通知相关对象更新状态
 */
class AbstractComponentItem
{
    int _row;
    AbstractComponentItem* _parent;
    enum { Type = 0 };
public:
    enum Columns {
        ColItemName = 0,
        ColItemNumber,
        ColDescription,
        ColMaxNumber
    };
    inline AbstractComponentItem(int row,
        AbstractComponentItem* parent = nullptr) :
        _row(row), _parent(parent) {}
    virtual AbstractComponentItem* child(int i) = 0;
    virtual int childCount()const = 0;
    virtual QString data(int i)const = 0;
    inline AbstractComponentItem* parent() { return _parent; }
    inline int row()const { return _row; }
};

class RailwayListItem;
class PageListItem;

/**
 * @brief The DiagramItem class 运行图结点，也就是根节点 （可能不会显示）
 */
class DiagramItem :public AbstractComponentItem
{
    Diagram& _diagram;

    std::unique_ptr<RailwayListItem> railList;
    std::unique_ptr<PageListItem> pageList;

public:
    enum { Type = 1 };
    enum DiagramRows {
        RowRailways = 0,
        RowPages,
        MaxDiagramRows
    };
    DiagramItem(Diagram& diagram);
    virtual AbstractComponentItem* child(int i)override;
    inline virtual int childCount()const override { return MaxDiagramRows; }
    virtual QString data(int i)const override { return i == 0 ? QObject::tr("运行图") : ""; }
};

class RailwayItem;

/**
 * @brief The RailwayListItem class  线路列表的根节点
 */
class RailwayListItem :public AbstractComponentItem
{
    Diagram& _diagram;
    std::deque<std::unique_ptr<RailwayItem>> _rails;
public:
    enum { Type = 2 };
    RailwayListItem(Diagram& diagram, DiagramItem* parent);
    virtual AbstractComponentItem* child(int i) override;
    inline virtual int childCount()const override { return _rails.size(); }
    virtual QString data(int i)const override;
};

class PageItem;
class PageListItem :public AbstractComponentItem
{
    Diagram& _diagram;
    std::deque<std::unique_ptr<PageItem>> _pages;
public:
    enum { type = 3 };
    PageListItem(Diagram& diagram, DiagramItem* parent);
    virtual AbstractComponentItem* child(int i) override;
    inline virtual int childCount()const override { return _pages.size(); }
    virtual QString data(int i)const override;
};


class RailwayItem :public AbstractComponentItem
{
    std::shared_ptr<Railway> _railway;
public:
    enum { Type = 4 };
    RailwayItem(std::shared_ptr<Railway> rail, int row, RailwayListItem* parent);
    virtual AbstractComponentItem* child(int i)override;
    inline virtual int childCount()const override { return 0; }
    virtual QString data(int i)const override;
};

class PageItem :public AbstractComponentItem
{
    std::shared_ptr<DiagramPage> _page;
public:
    enum { Type = 5 };
    PageItem(std::shared_ptr<DiagramPage> page, int row, PageListItem* parent);
    inline virtual AbstractComponentItem* child(int i)override { return nullptr; }
    inline virtual int childCount()const override { return 0; }
    virtual QString data(int i)const override;
};


