#pragma once
#include "intervaldatamodel.h"

#include "data/rail/forbid.h"

class ForbidModel : public IntervalDataModel
{
    Q_OBJECT;
    const std::shared_ptr<Forbid> forbid;
public:
    enum{
        ColInterval=0,
        ColStart,
        ColEnd,
        ColLength,
        ColMAX
    };
    explicit ForbidModel(std::shared_ptr<Forbid> forbid_, QObject *parent = nullptr);
    void refreshData();
    std::shared_ptr<Railway> appliedForbid();
protected:
    virtual void setupModel() override;
    virtual void setupRow(int row, std::shared_ptr<RailInterval> railint) override;
    virtual void copyRowData(int from, int to) override;
private slots:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void updateDuration(int row);
public slots:
    void copyToNextRow(int row);
    void calculateBegin(int row, int mins);
    void calculateEnd(int row, int mins);
    void calculateAllBegin(int mins);
    void calculateAllEnd(int mins);
};

