#pragma once

#include <QStandardItemModel>
#include <QUndoStack>
#include <memory>

#include "data/train/train.h"
#include "model/general/qemoveablemodel.h"

/**
 * @brief The TimetableStdModel class
 * 列车时刻表的模型；用于原来的当前列车编辑页面。
 * 这个实现采用StandardItemModel暂存数据。
 * 暂定Undo, Redo等操作交给TrainContext去处理。
 * （执行操作后、Undo/Redo之前，本Model可能已经无了）
 */
class TimetableStdModel : public QEMoveableModel
{
    Q_OBJECT;
    std::shared_ptr<Train> _train{};

    /**
     * @brief commitInPlace
     * 如果启用，则不考虑撤销问题，提交时直接更新数据到列车；
     * 如果不启用，则为目前的主程序设计，以信号的形式，由第三方来处理undo/redo的操作。
     */
    const bool commitInPlace;
public:
    enum Columns {
        ColName=0,
        ColArrive,
        ColDepart,
        ColBusiness,
        ColTrack,
        ColNote,
        ColStopTime,
        ColMAX
    };

    explicit TimetableStdModel(bool inplace, QWidget *parent = nullptr);

    auto train(){return _train;}
    void setTrain(std::shared_ptr<Train> train);

    void refreshData();

protected:
    virtual void setupNewRow(int row) override;

signals:
    void timetableChanged(std::shared_ptr<Train> train, std::shared_ptr<Train> newTable);

private:
    void setupModel();

    /**
     * 由当前时刻表生成列车对象。如果有问题，直接报告，然后返回空
     */
    std::shared_ptr<Train> getTimetableTrain();

public slots:

    /**
     * 执行更改。
     * 如果commitInPlace==false，那么用信号通告负责方面（实现中是contextTrain）来执行操作。
     * 创建一个新的Train对象，包含这里的时刻表信息，但没有别的信息，发送过去。
     */
    void actApply();
    void actCancel();

    /**
     * 当时刻数据变化，实时计算停时
     */
    void onDataChanged(const QModelIndex& leftTop, const QModelIndex& rightBottom);


};

