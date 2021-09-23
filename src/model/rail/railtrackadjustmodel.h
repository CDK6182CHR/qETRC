#pragma once
#include <QAbstractTableModel>

class Track;

/**
 * @brief The RailTrackAdjustModel class
 * 管理股道表，进行排序、重命名的Model。使用AbstractModel, 透明操作，
 * 所有修改直接应用。
 * 注意不是编辑股道顺序那个；那个直接用简单的QEMoveableModel就行了。
 */
class RailTrackAdjustModel : public QAbstractTableModel
{
    Q_OBJECT;
    QVector<std::shared_ptr<Track>>& order;
public:
    explicit RailTrackAdjustModel(QVector<std::shared_ptr<Track>>& order,
                                  QObject *parent = nullptr);

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * 将指定行上移一行。如果row==0，不进行任何操作。
     */
    void moveUp(int row);
    void moveDown(int row);

    Qt::ItemFlags flags(const QModelIndex& idx)const override;
};

