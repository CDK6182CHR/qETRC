#pragma once

#include <QAbstractTableModel>

class TrainCollection;
class Routing;

/**
 * @brief The RoutingCollectionModel class
 * pyETRC风格的交路编辑面板Model，采用TrainList的模式（只读，不保存数据），
 * 基于TrainCollection。
 * 原则上，所有交路的变更都经由这里。
 */
class RoutingCollectionModel : public QAbstractTableModel
{
    Q_OBJECT;
    TrainCollection& coll;
public:
    enum {
        ColName=0,
        ColOrder,
        ColModel,
        ColOwner,
        ColNote,
        ColMAX
    };
    explicit RoutingCollectionModel(TrainCollection& coll_, QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void refreshData();

signals:

    /**
     * 高亮运行线：通告context去处理
     */
    void routingHighlightChanged(std::shared_ptr<Routing> routing, bool on);

public slots:
    /**
     * context已经处理好了，这里维护状态。
     * 包含线性查找序号的操作
     */
    void onHighlightChangedByContext(std::shared_ptr<Routing> routing);
    void onRoutingInfoChanged(std::shared_ptr<Routing> routing);

    /**
     * 插入交路到指定位置，同时与train进行关联。
     */
    void addRoutingAt(std::shared_ptr<Routing> routing, int i);

    /**
     * 删除指定行的交路（实际从coll中删除），并且解除与train的关联。
     */
    void removeRoutingAt(int i);

    /**
     * 将新的交路添加到最后面
     */
    void appendRoutings(const QList<std::shared_ptr<Routing>>& routings);

    /**
     * 删除尾部的最后几个交路，即撤销添加操作
     */
    void removeTailRoutings(int count);
};

