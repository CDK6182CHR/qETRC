#pragma once
#include <QStandardItemModel>
#include <QDialog>
#include <QCheckBox>
#include <QTableView>
#include <vector>
#include <utility>

class Diagram;
class Railway;
class RailStation;
class TrainLine;
struct AdapterStation;

class StationTimetableSettledModel:
        public QStandardItemModel
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Railway> rail;
    std::shared_ptr<RailStation> station;
    std::vector<std::pair<std::shared_ptr<TrainLine>,const AdapterStation*>> lst;
public:
    enum{
        ColTrainName=0,
        ColStationName,
        ColArrive,
        ColDepart,
        ColType,
        ColStop,
        ColDir,
        ColStarting,
        ColTerminal,
        ColTrack,
        ColModel,
        ColOwner,
        ColNote,
        ColMAX
    };
    StationTimetableSettledModel(Diagram& dia,
                                 std::shared_ptr<Railway> rail_,
                                 std::shared_ptr<RailStation> station_,
                                 QObject* parent=nullptr);
    void setupModel();
};


/**
 * @brief The StationTimetableSettled class
 * 图定车站时刻表  仅考虑图定列车停站情况下的时刻表，与pyETRC一致
 */
class StationTimetableSettledDialog:
        public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Railway> rail;
    std::shared_ptr<RailStation> station;
    StationTimetableSettledModel* const model;

    QTableView* table;
    QCheckBox* ckHidePass;
public:
    StationTimetableSettledDialog(Diagram& diagram_, std::shared_ptr<Railway> rail_,
                                  std::shared_ptr<RailStation> station_,
                                  QWidget* parent=nullptr);

private:
    void initUI();
    void refreshData();

private slots:

    void onHidePassChanged(bool on);
    void outputCsv();
};

