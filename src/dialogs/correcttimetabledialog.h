#pragma once

#include <QDialog>
#include "model/general/qemoveablemodel.h"

class TrainCollection;
class QTableView;
class Train;
class CorrectTimetableModel: public QEMoveableModel
{
    Q_OBJECT
    std::shared_ptr<Train> train;
public:
    enum {
        ColName = 0,
        ColArrive,
        ColDepart,
        ColStopDuration,
        ColIntervalDuration,
        ColNote,
        ColMAX
    };
    CorrectTimetableModel(std::shared_ptr<Train> train, QObject* parent=nullptr);
    void refreshData();
    //void setTrain(std::shared_ptr<Train> train);
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

    void selectAll();
    void deselectAll();
    void selectInverse();
private:
    void setupModel();

    /**
     * 计算间隔时长。初始化和变动时都可调用。
     * 计算row所给行的数据；依赖这一行和前一行。
     */
    void calculateDurations(int row);

    QTime rowArrive(int row)const;
    QTime rowDepart(int row)const;
};

class SelectTrainCombo;

/**
 * @brief The CorrectTimetableDialog class
 * pyETRC.CorrectionWidget
 * 时刻表微调
 */
class CorrectTimetableDialog : public QDialog
{
    Q_OBJECT;
    std::shared_ptr<Train> train;
    SelectTrainCombo* cbTrain;
    CorrectTimetableModel* const model;
    QTableView* table;
public:
    CorrectTimetableDialog(
         std::shared_ptr<Train> train, QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    //void setTrain(std::shared_ptr<Train> train);
    void refreshData();
    void batchSelect();
    void actApply();
};
