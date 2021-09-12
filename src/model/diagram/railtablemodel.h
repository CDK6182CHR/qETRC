#pragma once


#include <QAbstractTableModel>
#include <QObject>

class Diagram;

/**
 * 线路列表，做成表格形式，与导航栏中的不一样。
 * 主要用在各种选择线路的场合。
 */
class RailTableModel : public QAbstractTableModel
{
    Q_OBJECT
    Diagram& diagram;
    enum Columns {
        ColName = 0,
        ColFirst,
        ColLast,
        ColMile,
        ColStations,
        ColMAX
    };
public:
    explicit RailTableModel(Diagram& diagram_, QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex& ) const override;
    virtual int columnCount(const QModelIndex&) const override { return ColMAX; }
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

