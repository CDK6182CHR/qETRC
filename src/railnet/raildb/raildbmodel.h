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
private:
    /**
     * @brief getParentItem
     * @param parent
     * 获取parent对应的节点指针。如果是invalid，则理解为根指针。
     */
    pACI getParentItem(const QModelIndex& parent)const;
public slots:
    void resetModel();
};

