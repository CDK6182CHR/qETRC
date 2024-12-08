#pragma once
#include <QDialog>
#include <QStandardItemModel>

class Train;

class SplitTrainModel: public QStandardItemModel
{
    Q_OBJECT
    std::shared_ptr<Train> _train;
public:
    enum Columns {
        ColName,
        ColArrive,
        ColDepart,
        ColBusiness,
        ColStopTime,
        ColNewTrainName,
        ColMAX
    };

    SplitTrainModel(std::shared_ptr<Train> train, QObject* parent=nullptr);

private:
    void setupModel();
    void refreshData();
};


/**
 * @brief 2024.12.08 The SplitTrainDialog class
 * Split the train into several trains.
 * The new trains are added to the same routing (if available) by default.
 */
class SplitTrainDialog : public QDialog
{
    Q_OBJECT
public:
    SplitTrainDialog();
};


