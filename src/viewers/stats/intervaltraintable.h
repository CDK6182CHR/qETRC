#pragma once

#include <QTableView>
#include <QStandardItemModel>
#include "data/analysis/inttrains/intervaltraininfo.h"
#include "data/analysis/runstat/trainintervalstatresult.h"


class QTableView;
class QCheckBox;
class QLineEdit;
class Diagram;
struct DiagramOptions;


/**
 * 2026.02.12: Add the information for total mile and speeds.
 * The corresponding data will be computed when the model is updated.
 */
class IntervalTrainModel: public QStandardItemModel
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    IntervalTrainList _data;
	std::vector<TrainIntervalStatResult> _statData;
public:
    enum {
        ColTrainName=0,
        ColType,
        ColFromStation,
        ColFromTime,
        ColToStation,
        ColToTime,
        ColTime,
        ColStarting,
        ColTerminal,
        ColTotalMile,
        ColTravSpeed,
        ColTechSpeed,
        ColMAX
    };
    IntervalTrainModel(const DiagramOptions& ops, QWidget* parent=nullptr);
    void refreshData();
    void setupModel();
    void resetData(IntervalTrainList&& data_);
    void resetData(const IntervalTrainList& data_);
    const auto& getData()const{return _data;}

private:
    void updateTrainStatData();
};



/**
 * @brief The IntervalTrainTable class
 * Model/View合流
 * 区间列车表
 */
class IntervalTrainTable : public QTableView
{
    const DiagramOptions& _ops;
    IntervalTrainModel* const model;
public:
    IntervalTrainTable(const DiagramOptions& ops, QWidget* parent=nullptr);
    auto* getModel(){return model;}
private:
    void initUI();
};

