#pragma once

#include <QDialog>

#include "data/common/traintime.h"
#include "model/general/qemoveablemodel.h"

class TrainCollection;
class QTableView;
class Train;
class QBitArray;
class TrainStation;
struct DiagramOptions;


class CorrectTimetableModel: public QEMoveableModel
{
    Q_OBJECT
    const DiagramOptions& _ops;
    std::shared_ptr<Train> train;
public:
    enum {
        ColName = 0,
        ColArrive,
        ColDepart,
        ColStopDuration,
        ColIntervalDuration,
        ColBusiness,
        ColTrack,
        ColNote,
        ColMAX
    };
    CorrectTimetableModel(const DiagramOptions& ops, std::shared_ptr<Train> train, QObject* parent=nullptr);
    void refreshData();
    //void setTrain(std::shared_ptr<Train> train);

    /**
     * @brief appliedTrain
     * 将当前表格数据整理成Train对象返回
     */
    std::shared_ptr<Train> appliedTrain();

public slots:

    /**
     * @brief doExchange 交换到发
     * 以do开头的方法，读取选择那一列的选择情况，一次性全部执行
     */
    void doExchange();

    /**
     * @brief doInverse
     * 区间反排
     */
    void doReverse();

    void doMoveUp();
    void doMoveDown();
    void doToTop();
    void doToBottom();

    void doPartialSortArrive();
    void doPartialSortDepart();

    void selectAll();
    void deselectAll();
    void selectInverse();

    /**
     * 2022.05.28  将train引用指向参数，用于提供自动更正的接口。
     * 我们其实并不要求this->train与实际的train是一致的。
     * 这里给的train实际上是一个副本。
     */
    void setTrain(std::shared_ptr<Train> train);
private:
    void setupModel();

    /**
     * 计算间隔时长。初始化和变动时都可调用。
     * 计算row所给行的数据；依赖这一行和前一行。
     */
    void calculateDurations(int row);

    TrainTime rowArrive(int row)const;
    TrainTime rowDepart(int row)const;

    void calculateSelectedRows(const QBitArray& rows);
    void calculateAllRows();

    int firstSelectedRow();
    int lastSelectedRow();

    void partialSort(bool (*comp)(const TrainStation&, const TrainStation&));

private slots:
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
        const QVector<int>& roles);

};

class SelectTrainCombo;
class QCheckBox;

/**
 * @brief The CorrectTimetableDialog class
 * pyETRC.CorrectionWidget
 * 时刻表微调
 */
class CorrectTimetableDialog : public QDialog
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    std::shared_ptr<Train> train;
    SelectTrainCombo* cbTrain;
    CorrectTimetableModel* const model;
    QTableView* table;
    QCheckBox* ckEdit;
    bool informEdit = true;
public:
    CorrectTimetableDialog(
         const DiagramOptions& ops, std::shared_ptr<Train> train, QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void correctionApplied(std::shared_ptr<Train> train, std::shared_ptr<Train> table);
private slots:
    //void setTrain(std::shared_ptr<Train> train);
    void refreshData();
    void batchSelect();
    void actApply();
    void onEditToggled(bool on);

    void autoCorrect();
};
