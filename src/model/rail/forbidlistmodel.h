#pragma once

#include <memory>
#include <QAbstractListModel>
#include <QBitArray>
#include <vector>

class Forbid;
class Railway;
/**
 * @brief The SelectForbidModel class
 * 2022.03.13
 * 选择天窗的Model
 * 选择信息存在BitSet里面。
 * 注意目前的实现中，天窗数量基本不会变，故采用这个设计
 */
class SelectForbidModel: public QAbstractListModel
{
    Q_OBJECT
    using Base=QAbstractListModel;
    std::shared_ptr<Railway> railway{};
    QBitArray _selected{};
public:
    SelectForbidModel(QObject* parent=nullptr);
    std::vector<std::shared_ptr<Forbid>>
        selectedForbids();

public slots:
    void setRailway(std::shared_ptr<Railway> _railway);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};
