#pragma once

#include <QAbstractTableModel>
#include <QObject>
#include <memory>
#include <functional>
#include <optional>

#include "data/train/traincollection.h"

struct Config;

/**
 * @brief The TrainListModel class
 * 采用类似只读的模式实现  有关操作立即响应
 * 注意全程应当是类似单例的模式 （Diagram只有一个TrainCollection） 采用引用
 * 暂定由上层的Widget (原来的pyETRC.TrainWidget)负责删除
 */
class TrainListModel : public QAbstractTableModel
{
    Q_OBJECT

    TrainCollection& coll;
    enum Columns {
        ColTrainName=0,
        ColStarting,
        ColTerminal,
        ColType,
        ColShow,
        ColMile,
        ColSpeed,
        MAX_COLUMNS
    };
public:

    TrainListModel(TrainCollection& coll, QObject* parent);

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    //调整的接口 下次再实现
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    //virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    //virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    //virtual void sort(int column, Qt::SortOrder order) override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    void trainShowChanged(std::shared_ptr<Train> train, bool show);
};

