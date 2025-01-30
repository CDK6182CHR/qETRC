#pragma once
#include <QDialog>
#include <QVector>
#include <QStandardItemModel>

class Train;
class QTableView;
class TrainCollection;
class TrainName;
class QCheckBox;

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

    /**
     * Process the data in the table, and generate the new trains.
     * The data will be checked here.
     */
    QVector<std::shared_ptr<Train>> acceptedData(TrainCollection& coll, QWidget* report_parent, bool check_terminal);

private:
    void setupModel();
    void refreshData();

    /**
     * Create a train object based on the given data. Range: [start_row, end_row]
     * The train name is checked here. Returns nullptr on failed.
     */
    std::shared_ptr<Train> createTrain(TrainCollection& coll, QWidget* report_parent,
        const TrainName& name, int start_row, int end_row, bool first_starting, bool last_terminal);
};


/**
 * @brief 2024.12.08 The SplitTrainDialog class
 * Split the train into several trains.
 * The new trains are added to the same routing (if available) by default.
 */
class SplitTrainDialog : public QDialog
{
    Q_OBJECT;
    TrainCollection& m_coll;
    std::shared_ptr<Train> m_train;
    SplitTrainModel* m_model;
    QTableView* m_table;
    QCheckBox* m_ckCrossTerminal;
public:
    SplitTrainDialog(TrainCollection& coll, std::shared_ptr<Train> train, QWidget* parent=nullptr);

    void accept()override;
    void reject()override;

private:
    void initUI();


signals:
    void splitApplied(std::shared_ptr<Train> train, QVector<std::shared_ptr<Train>> newTrains);

};


