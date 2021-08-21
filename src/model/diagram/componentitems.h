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
        inline int& rowRef(){return _row;}
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
        inline virtual int childCount()const override {return static_cast<int>( _rails.size()); }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
		
		/**
		 * 暂定在这里执行真实的添加工作
		 */
		void appendRailways(const QList<std::shared_ptr<Railway>>& rails);

        void appendRailway(std::shared_ptr<Railway> rail);

		void removeTailRailways(int cnt);

        /**
         * @brief removeRailwayAt
         * 删除指定下标的基线数据，同时触发删除既有车次、运行图中的数据，不可撤销。
         */
        void removeRailwayAt(int i);
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
        inline virtual int childCount()const override { return static_cast<int>( _pages.size()); }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
        void resetChildren();

		/**
		 * 暂定在这里实施实际的数据插入操作
		 */
		void insertPage(std::shared_ptr<DiagramPage> page, int i);
		void removePageAt(int i);
	};

    class RulerItem;
    class ForbidItem;

    /**
     * @brief The RailwayItem class
     * 标尺、天窗放在同一个级别下面。注意行数和index可能不等，所有的行数计算应该由本类完成。
     */
	class RailwayItem :public AbstractComponentItem
	{
		std::shared_ptr<Railway> _railway;
        std::deque<std::unique_ptr<RulerItem>> _rulers;
        std::deque<std::unique_ptr<ForbidItem>> _forbids;
	public:
		enum { Type = 4 };
		RailwayItem(std::shared_ptr<Railway> rail, int row, RailwayListItem* parent);
		virtual AbstractComponentItem* child(int i)override;
        virtual int childCount()const override;
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
		inline auto railway()const { return _railway; }

        /**
         * @brief onRulerInsertedAt
         * 在指定位置添加标尺；并不执行实际的添加，而仅仅是更新item
         */
        void onRulerInsertedAt(int i);

        void onRulerRemovedAt(int i);

        int getRulerRow(std::shared_ptr<const Ruler> ruler);

        int getForbidRow(std::shared_ptr<const Forbid> forbid);
	};

	class PageItem :public AbstractComponentItem
	{
		std::shared_ptr<DiagramPage> _page;
	public:
		enum { Type = 5 };
		PageItem(std::shared_ptr<DiagramPage> page, int row, PageListItem* parent);
		inline virtual AbstractComponentItem* child(int )override { return nullptr; }
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
        inline virtual int childCount()const override { return static_cast<int>( _trains.size()); }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
		void resetChildren();

        void removeTrainAt(int i);
        void undoRemoveTrainAt(std::shared_ptr<Train> train,int i);

		/**
		 * 将新列车实际添加到coll后面
		 */
		void addNewTrain(std::shared_ptr<Train> train);
		void undoAddNewTrain();
	};

	class TrainModelItem :public AbstractComponentItem
	{
		std::shared_ptr<Train> _train;
	public:
		enum{Type=7};
		TrainModelItem(std::shared_ptr<Train> train, int row, TrainListItem* parent);
		inline virtual AbstractComponentItem* child(int )override { return nullptr; }
		inline virtual int childCount()const override { return 0; }
		virtual QString data(int i)const override;
		inline virtual int type()const override { return Type; }
		inline auto train() { return _train; }
	};

    class RulerItem: public AbstractComponentItem
    {
        std::shared_ptr<Ruler> _ruler;
	public:
        enum {Type=8};
        RulerItem(std::shared_ptr<Ruler> ruler, int row, RailwayItem* parent);
        inline virtual AbstractComponentItem* child(int)override {return nullptr; }
        inline virtual int childCount()const override {return 0;}
        virtual QString data(int i)const override;
        inline virtual int type()const override {return Type;}
        inline auto ruler() {return _ruler;}
    };

    class ForbidItem: public AbstractComponentItem
    {
        std::shared_ptr<Railway> _railway;
        std::shared_ptr<Forbid> _forbid;
    public:
        enum {Type=9};
        ForbidItem(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> railway,
                   int row, RailwayItem* parent);
        inline virtual AbstractComponentItem* child(int )override{return nullptr;}
        inline virtual int childCount() const override{return 0;}
        virtual QString data(int i) const override;
        inline virtual int type()const override {return Type;}
        inline auto forbid(){return _forbid;}
        inline auto railway(){return _railway;}
    };

}

