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
struct DiagramOptions;


/**
 * @brief The SelectTrainStationDialog class
 * 2022.11.06  选择时刻表中信息，用于导入停站时长。
 *
 * Currently, the apply slot is NOT implemented; the only
 * valid usage is through the static function dlgGetStation.
 * For this reason, the constructor is declared as private.
 */
class SelectTrainStationsDialog : public QDialog
{
    Q_OBJECT
    TrainCollection& coll;
    const DiagramOptions& _ops;
    SelectTrainCombo* cbTrain;
    QTableView* table;
    TimetableStdModel* const model;

    SelectTrainStationsDialog(TrainCollection& coll, const DiagramOptions& ops, QWidget* parent=nullptr);

public:
    using result_type=std::vector<std::list<TrainStation>::iterator>;

    result_type getSelection();
    void refreshData();

    static result_type
        dlgGetStation(TrainCollection& coll, const DiagramOptions& ops, QWidget* parent);

signals:
//    void applied(const result_type&);
private:
    void initUI();
private slots:
//    void actApply();
};

