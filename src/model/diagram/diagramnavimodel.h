#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QWidget>
#include <QModelIndex>

#include "data/diagram/diagram.h"
#include "componentitems.h"

/**
 * @brief The DiagramNaviModel class
 * 运行图资源管理器 用TreeView展示
 * 逻辑上，是只读的
 */
class DiagramNaviModel : public QAbstractItemModel
{
    Q_OBJECT

    Diagram& _diagram;
    std::unique_ptr<navi::DiagramItem> _root;

public:
    explicit DiagramNaviModel(Diagram& diagram, QObject* parent);

    // 通过 QAbstractItemModel 继承
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    inline Diagram& diagram() { return _diagram; }

    /**
     * 发生了打开新运行图等操作，直接暴力重新加载Item
     */
    void resetModel();

    /**
     * 试验：导入线路，缀加到尾部。
     * 实际的添加由这里完成！！
     */
    void importRailways(const QList<std::shared_ptr<Railway>> rails);

    void removeTailRailways(int cnt);

private:
    //for convenient..
    using ACI = navi::AbstractComponentItem;
    using pACI = navi::AbstractComponentItem*;
    pACI getItem(const QModelIndex& idx)const;

public slots:
    void resetTrainList();
    //void resetRailwayList();
    //void resetPageList();

    void insertPage(std::shared_ptr<DiagramPage> page, int index);
    void removePageAt(int index);

    void onPageNameChanged(int i);
};


