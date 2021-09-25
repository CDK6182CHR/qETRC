#pragma once

#include <QAbstractItemModel>
#include <memory>

#include "raildbitems.h"
class RailDB;

class RailDBModel : public QAbstractItemModel
{
    Q_OBJECT;
    std::shared_ptr<RailDB> _raildb;
    std::unique_ptr<navi::RailCategoryItem> _root;

    using ACI=navi::AbstractComponentItem;
    using pACI=ACI*;

public:
    explicit RailDBModel(std::shared_ptr<RailDB> raildb, QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    pACI getItem(const QModelIndex& idx)const;
    QModelIndex indexByPath(const std::deque<int>& path);

    /**
     * 返回path所示的父节点。如果为空，直接返回空。（这表示根节点）
     * 用在插入时，插入的index可能是个无效的。
     */
    QModelIndex parentIndexByPath(const std::deque<int>& path);

    /**
     * @brief railIndexBrute
     * 暴力查找 （DFS）指定线路。一般用不到；只在Path失效的情况紧急替代。
     */
    QModelIndex railIndexBrute(std::shared_ptr<Railway> railway);
    QModelIndex categoryIndexBrute(std::shared_ptr<RailCategory> category);

    std::deque<navi::path_t> searchFullName(const QString& name);
    std::deque<navi::path_t> searchPartName(const QString& name);
    std::deque<navi::path_t> searchRailName(const QString& name);

    std::shared_ptr<Railway> railwayByPath(const navi::path_t& path);

private:
    /**
     * @brief getParentItem
     * @param parent
     * 获取parent对应的节点指针。如果是invalid，则理解为根指针。
     */
    pACI getParentItem(const QModelIndex& parent)const;
    QModelIndex railIndexBruteFrom(std::shared_ptr<Railway> railway,
                                   const QModelIndex& idx);
    QModelIndex categoryIndexBruteFrom(std::shared_ptr<RailCategory> cat,
        const QModelIndex& idx);
    std::shared_ptr<Railway> railwayByIndex(const QModelIndex& idx);
    std::shared_ptr<RailCategory> categoryByIndex(const QModelIndex& idx);



public slots:
    void resetModel();

    /**
     * 指定线路变更后，更新数据。
     */
    void onRailInfoChanged(std::shared_ptr<Railway> railway, const std::deque<int>& path);

    void onCategoryInfoChanged(std::shared_ptr<RailCategory> cat, const std::deque<int>& path);

    /**
     * 执行删除：railway参数用来检查PATH对应的路径是否合理。
     * 调用相关Item完成确实的操作。
     */
    void commitRemoveRailwayAt(std::shared_ptr<Railway> railway,
        const std::deque<int>& path);

    /**
     * 执行添加，或撤销删除。path给出的是插入后的位置
     */
    void commitInsertRailwayAt(std::shared_ptr<Railway> railway,
        const std::deque<int>& path);

    /**
     * 执行添加一组线路  不做检查
     */
    void commitInsertRailwaysAt(const QList<std::shared_ptr<Railway>>& rails,
        const std::deque<int>& path);

    /**
     * 删除一组线路。检查第一个的path是否正确。
     */
    void commitRemoveRailwaysAt(const QList<std::shared_ptr<Railway>>& rails,
        const std::deque<int>& path);

    void commitInsertCategoryAt(std::shared_ptr<RailCategory> cat,
        const std::deque<int>& path);

    void commitRemoveCategoryAt(std::shared_ptr<RailCategory> cat,
        const std::deque<int>& path);
};

