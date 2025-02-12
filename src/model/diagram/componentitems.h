#pragma once


#include <memory>
#include <deque>
#include <QString>
#include <QObject>

class Diagram;
class TrainCollection;
class TrainPathCollection;

/**
 * 用于做导航窗口的ModelItem类
 * 由于名字太容易冲突，用namespace 封装一下
 */

class Railway;
class DiagramPage;
class Ruler;
class Forbid;
class Train;
class Routing;
class TrainPath;
class PathRuler;


namespace navi {

    /**
     * 定位到一个Item的路径。从front()至back()，是从顶层至底层的行号。
     */
    using path_t=std::deque<int>;

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

        /**
         * @brief The DBColumns enum
         * 用于线路数据库的列数描述，前缀DB以防止冲突
         */
        enum DBColumns {
            DBColName=0,
            DBColMile,
            DBColStart,
            DBColEnd,
            DBColAuthor,
            DBColVersion,
            DBColMAX
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

        /**
         * @brief path
         * @return 当前Item从根节点开始的每一层行号。
         */
        path_t path()const;

        /**
         * 根据path，从当前结点开始查找，返回对应结点
         */
        AbstractComponentItem* itemByPath(const path_t& path);

        virtual ~AbstractComponentItem()=default;
	};

	class RailwayListItem;
	class PageListItem;
	class TrainListItem;
    class RoutingListItem;
	class PathListItem;

	/**
	 * @brief The DiagramItem class 运行图结点，也就是根节点 （可能不会显示）
	 */
	class DiagramItem :public AbstractComponentItem
	{
		Diagram& _diagram;

		std::unique_ptr<RailwayListItem> railList;
		std::unique_ptr<PageListItem> pageList;
		std::unique_ptr<TrainListItem> trainList;
        std::unique_ptr<RoutingListItem> routingList;
		std::unique_ptr<PathListItem> pathList;

	public:
		enum { Type = 1 };
		enum DiagramRows {
			RowRailways = 0,
			RowPages,
			RowTrains,
            RowRoutings,
			RowPaths,
			MaxDiagramRows
		};
		DiagramItem(Diagram& diagram);
		virtual AbstractComponentItem* child(int i)override;
		inline virtual int childCount()const override { return MaxDiagramRows; }
		virtual QString data(int i)const override { return i == 0 ? QObject::tr("运行图") : ""; }
		inline virtual int type()const override { return Type; }
        /**
         * @brief refreshRoutingList
         * 重新加载Routing
         */
        void refreshRoutingList();
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

		/**
		 * 2021.10.15  撤销删除线路
		 */
		void insertRailwayAt(int i, std::shared_ptr<Railway> rail);

		void removeTailRailways(int cnt);

#if 0
        /**
         * @brief removeRailwayAt
         * 删除指定下标的基线数据，同时触发删除既有车次、运行图中的数据，不可撤销。
         */
		[[deprecated]]
        void removeRailwayAt(int i);
#endif

		void removeRailwayAtU(int i);
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

    class RoutingItem;
    class RoutingListItem: public AbstractComponentItem
    {
        TrainCollection& coll;
        std::deque<std::unique_ptr<RoutingItem>> _routings;
    public:
        enum {Type=10};
        RoutingListItem(TrainCollection& coll_, DiagramItem* parent);
        virtual AbstractComponentItem* child(int i)override;
        virtual int childCount()const override {return static_cast<int>(_routings.size()) ;}
        virtual QString data(int i)const override;
        virtual int type()const override {return Type;}
        void onRoutingInsertedAt(int first,int last);
        void onRoutingRemovedAt(int first,int last);
    };

    class RoutingItem: public AbstractComponentItem
    {
        std::shared_ptr<Routing> _routing;
    public:
        enum {Type=11};
        RoutingItem(std::shared_ptr<Routing> routing, int row, RoutingListItem* parent);
        virtual AbstractComponentItem* child(int)override {return nullptr;}
        virtual int childCount()const override{return 0;}
        virtual QString data(int i)const override;
        virtual int type()const override {return Type;}
        auto routing()const{return _routing;}
    };

	class PathItem;
	class PathListItem :public AbstractComponentItem
	{
		TrainPathCollection& pathcoll;
		std::deque<std::unique_ptr<PathItem>> _paths;
	public:
		enum { Type = 12 };
		PathListItem(TrainPathCollection& pathcoll, DiagramItem* parent);

		virtual AbstractComponentItem* child(int i) override;
		virtual int childCount() const override { return _paths.size(); }
		virtual QString data(int i) const override;
		virtual int type() const override { return Type; }

		/**
		 * These functions are called in DiagramNaviModel
		 */
		void onPathInsertedAt(int first, int last);
		void onPathRemovedAt(int first, int last);

		auto* pathItemAt(int idx) { return _paths.at(idx).get(); }
	};


    class PathRulerItem;
	class PathItem : public AbstractComponentItem
	{
		TrainPath* _path;
        std::deque<std::unique_ptr<PathRulerItem>> _rulerItems;
	public:
		enum { Type = 13 };
		PathItem(TrainPath* path, int row, PathListItem* parent);

		// 通过 AbstractComponentItem 继承
        virtual AbstractComponentItem* child(int i) override;
        virtual int childCount() const override;
		virtual QString data(int i) const override;
		virtual int type() const override { return Type; }

        /**
         * 2025.02.12  Insert ruler at given place.
         * No actual insertion taken place here; only for post-processing after insertion
         */
        void onRulerInsertedAt(int place);

        void onRulerRemovedAt(int place);

		auto* getPath() { return _path; }
	};


	class PathRulerItem : public AbstractComponentItem
	{
		std::shared_ptr<PathRuler> _ruler;
	public:
		enum { Type = 14 };
		PathRulerItem(std::shared_ptr<PathRuler> ruler, int row, PathItem* parent);

        // AbstractComponentItem interface
    public:
        AbstractComponentItem *child([[maybe_unused]] int i) override {return nullptr;};
        int childCount() const override {return 0;}
        QString data(int i) const override;
        int type() const override {return Type;}

        const auto& ruler() {return _ruler;}
    };
}

