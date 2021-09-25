#pragma once

#include "model/diagram/componentitems.h"

class RailCategory;

namespace navi {

    class RailwayItemDB;

    /**
     * @brief The RailCategoryItem class
     * 用于线路数据库的Item。与RailListItem的区别是支持sub-cat。以及列数规定的不同。
     * 暂不确定是否要取消RailListItem。
     */
    class RailCategoryItem : public navi::AbstractComponentItem
    {
        std::shared_ptr<RailCategory> _category;
        std::deque<std::unique_ptr<RailCategoryItem>> _subcats;
        std::deque<std::unique_ptr<RailwayItemDB>> _railways;
    public:
        enum {Type=20};
        RailCategoryItem(std::shared_ptr<RailCategory> category,
                         int row,
                         RailCategoryItem* parent);
        auto category(){return _category;}

    public:
        virtual AbstractComponentItem *child(int i) override;
        virtual int childCount() const override;
        virtual QString data(int i) const override;
        virtual int type() const override {return Type;}

        int subCategoriesCount()const;

        /**
         * 删除指定位置的railway。执行实际的删除操作。
         * 注意入参i是行数，要减去前面subcat的行
         */
        void removeRailwayAt(int i);

        void removeRailwaysAt(int i, int count);

        /**
         * 在位置i处插入railway，执行实际的插入操作。
         * 注意入参i是行数。
         */
        void insertRailwayAt(std::shared_ptr<Railway> rail, int i);

        void insertRailwaysAt(const QList<std::shared_ptr<Railway>>& rails, int i);

        /**
         * 插入Category。执行实际的数据更改操作。
         * 注意所有railway的行数也应该修改
         */
        void insertCategoryAt(std::shared_ptr<RailCategory> cat, int i);

        void removeCategoryAt(int i);

        /**
         * 搜索符合pred所给条件的线路，返回符合条件的路径表。
         * 注意只搜索Railway，不搜索Category。
         * 从当前对象开始搜索。
         */
        std::deque<navi::path_t> searchBy(std::function<bool(const Railway& )> pred);
    };


    /**
     * @brief The RailwayItemDB class
     * 单个线路的Item。与RailwayItem的区别在parent类型不同。
     * 暂不确定是否合并为同一类。
     */
    class RailwayItemDB: public navi::AbstractComponentItem
    {
        std::shared_ptr<Railway> _railway;
    public:
        enum {Type=21};
        RailwayItemDB(std::shared_ptr<Railway> rail, int row,
                      RailCategoryItem* parent);
        auto railway(){return _railway;}


        // AbstractComponentItem interface
    public:
        virtual AbstractComponentItem *child(int ) override {return nullptr;}
        virtual int childCount() const override {return 0;}
        virtual QString data(int i) const override;
        virtual int type() const override {return Type;}
    };


}
