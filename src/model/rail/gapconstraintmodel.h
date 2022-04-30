#pragma once

#include <QAbstractTableModel>
#include <memory>
#include <data/diagram/traingap.h>

namespace gapset{
class GapSetAbstract;
}

/**
 * @brief The GapConstraintModel class
 * 分类别的列车间隔限制编辑器
 * 构造方式什么的问题，一会再想想。
 * 2022.03.14：不再持有GapSet的所有权；这个所有权交给外面来搞。
 */
class GapConstraintModel : public QAbstractTableModel
{
    //std::unique_ptr<gapset::GapSetAbstract> _gapSet{};
    gapset::GapSetAbstract* _gapSet{};
public:
    enum {
        ColName=0,
        ColLimit=1,
        ColMAX
    };
    explicit GapConstraintModel(QObject *parent = nullptr);

    auto* gapSet(){return _gapSet;}

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:
    void setSingleLine(bool singleLine);
    void setGapSet(gapset::GapSetAbstract* gapSet, bool singleLine);
    void setConstrainFromCurrent(const std::map<TrainGapTypePair, int>& mingap,
        int minSecs, int maxSecs);
};

