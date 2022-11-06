#pragma once

#include <QDialog>
#include <memory>
#include <list>
#include <vector>

class SelectTrainCombo;
class TrainCollection;

class TrainStation;
class TimetableStdModel;
class QTableView;
class Train;
/**
 * @brief The SelectTrainStationDialog class
 * 2022.11.06  选择时刻表中信息，用于导入停站时长。
 */
class SelectTrainStationsDialog : public QDialog
{
    Q_OBJECT
    TrainCollection& coll;
    SelectTrainCombo* cbTrain;
    QTableView* table;
    TimetableStdModel* const model;

public:
    SelectTrainStationsDialog(TrainCollection& coll, QWidget* parent=nullptr);
    std::vector<std::list<TrainStation>::iterator>
        getSelection();
    void refreshData();

    static std::vector<std::list<TrainStation>::iterator>
        dlgGetStation(TrainCollection& coll, QWidget* parent);
private:
    void initUI();
};

