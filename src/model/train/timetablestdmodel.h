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

    explicit TimetableStdModel(bool inplace, QObject *parent = nullptr);

    auto train(){return _train;}
    void setTrain(std::shared_ptr<Train> train);

    void refreshData();

protected:
    virtual void setupNewRow(int row) override;

private:
    void setupModel();

private slots:
    void actApply();
    void actCancel();
    void actRemove();


};

