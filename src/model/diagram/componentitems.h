#pragma once


#include <memory>
#include <deque>
#include "data/diagram/diagram.h"
#include "data/train/traincollection.h"

/**
 * 用于做导航窗口的ModelItem类
 * 由于名字太容易冲突，用namespace 封装一下
 */
namespace navi {

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
		virtual int type()const = 0;
		inline AbstractComponentItem* parent() { return _parent; }
        inline int row()const { return _row; }
        inline void setRow(int r){_row=r;}
	};

	class RailwayListItem;
	class PageListItem;
	class TrainListItem;

	/**
	 * @brief The DiagramItem class 运行图结点，也就是根节点 （可能不会显示）
	 */
	class DiagramItem :public AbstractComponentItem
	{
		Diagram& _diagram;

		std::unique_ptr<RailwayListItem> railList;
		std::unique_ptr<PageListItem> pageList;
		std::unique_ptr<TrainListItem> trainList;

	public:
		enum { Type = 1 };
		enum DiagramRows {
			RowRailways = 0,
			RowPages,
			RowTrains,
			MaxDiagramRows
		};
		DiagramItem(Diagram& diagram);
		virtual AbstractComponentItem* child(int i)override;
		inline virtual int childCount()const override { return MaxDiagramRows; }
		virtual QString data(int i)const override { return i == 0 ? QObject::tr("运行图") : ""; }
		inline virtual int type()const override { return Type; }
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
		inline virtual int type()const override { return Type; }
		
		/**
		 * 暂定在这里执行真实的添加工作
		 */
		void appendRailways(const QList<std::shared_ptr<Railway>>& rails);

        void appendRailway(std::shared_ptr<Railway> rail);

		void removeTailRailways(int cnt);
	};

	class PageItem;
	class PageListItem :public AbstractComponentItem
	{
		Diagram& _diagram;
		std::deque<std::unique_ptr<PageItem>> _pages;
	public:
		enum { Type = 3 };
		PageListItem(Diagram& diagram, DiagramItem* parent);
		virtual AbstractComponentItem* child(int i) override;
		inline virtual int childCount()const override { return _pages.size(); }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }

		/**
		 * 暂定在这里实施实际的数据插入操作
		 */
		void insertPage(std::shared_ptr<DiagramPage> page, int i);
		void removePageAt(int i);
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
		inline virtual int type()const override { return Type; }
		inline auto railway()const { return _railway; }
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
		inline virtual int type()const override { return Type; }
		inline auto page() { return _page; }
	};

	class TrainModelItem;
	class TrainListItem : public AbstractComponentItem
	{
		TrainCollection& _coll;
		std::deque<std::unique_ptr<TrainModelItem>> _trains;
	public:
		enum { Type = 6 };
		TrainListItem(TrainCollection& coll, DiagramItem* parent);
		inline virtual AbstractComponentItem* child(int i)override;
		inline virtual int childCount()const override { return _trains.size(); }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
		void resetChildren();
	};

	class TrainModelItem :public AbstractComponentItem
	{
		std::shared_ptr<Train> _train;
	public:
		enum{Type=7};
		TrainModelItem(std::shared_ptr<Train> train, int row, TrainListItem* parent);
		inline virtual AbstractComponentItem* child(int i)override { return nullptr; }
		inline virtual int childCount()const override { return 0; }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
		inline auto train() { return _train; }
	};

}

