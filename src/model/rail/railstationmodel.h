#pragma once

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QUndoStack>
#include <memory>

#include "data/rail/rail.h"
#include "model/delegate/qedelegate.h"

/**
 * @brief The RailStationModel class
 * 线路的站表（里程表）的数据模型，用于编辑里程的表格。
 * 注意使用StandartItem来暂存数据。
 */
class RailStationModel : public QStandardItemModel
{
    Q_OBJECT
    std::shared_ptr<Railway> railway;
    QUndoStack* const _undo;
public:
    enum Columns{
        ColName=0,
        ColMile,
        ColCounter,
        ColLevel,
        ColShow,
        ColDir,
        ColPassenger,
        ColFreight,
        ColMAX
    };
    explicit RailStationModel(QUndoStack* undo,
                              QObject *parent = nullptr);

    void setRailway(std::shared_ptr<Railway> rail);

    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;

private:
    void setupModel();
};



