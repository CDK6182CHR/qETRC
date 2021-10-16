#pragma once


#include <QAbstractTableModel>
#include <memory>

class Diagram;

/**
 * 线路列表，做成表格形式，与导航栏中的不一样。
 * 主要用在各种选择线路的场合。
 */
QT_DEPRECATED
class  RailTableModel : public QAbstractTableModel
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

class Railway;

/**
 * 线路列表的Model；用QList维护数据，可以增删。
 * 用于新版的运行图页面设置。
 */
class RailListModel : public QAbstractTableModel
{
    Q_OBJECT;
    QList<std::shared_ptr<Railway>> _railways;
public:
    enum Columns {
        ColName = 0,
        ColFirst,
        ColLast,
        ColMile,
        ColStations,
        ColMAX
    };
    RailListModel(const QList<std::shared_ptr<Railway>>& railways, QObject* parent = nullptr);
    virtual int rowCount(const QModelIndex&) const override;
    virtual int columnCount(const QModelIndex&) const override { return ColMAX; }
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    std::shared_ptr<Railway> takeAt(int row);
    void append(std::shared_ptr<Railway> railway);
    void append(const QList<std::shared_ptr<Railway>>& railways);
    void clear();

    const auto& railways()const { return _railways; }

    /**
     * 重设列表
     */
    void setRailways(const QList<std::shared_ptr<Railway>>& lst);

    void moveUp(int row);
    void moveDown(int row);
};

