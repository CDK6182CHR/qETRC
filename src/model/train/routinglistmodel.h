#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <memory>

class Routing;
/**
 * @brief The RoutingListModel class
 * 基于QList<Routing>的model，只读，结构和RoutingCollectionModel不同。
 * 仅用于批量解析。
 */
class RoutingListModel : public QAbstractTableModel
{
    Q_OBJECT;
    QList<std::shared_ptr<Routing>> routings;
public:
    enum{
        ColName=0,
        ColReal,
        ColVirtual,
        ColOrder,
        ColMAX
    };
    explicit RoutingListModel(QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void setRoutings(const QList<std::shared_ptr<Routing>>& routings);
};

