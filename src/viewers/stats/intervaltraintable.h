#pragma once

#include <QTableView>
#include <QStandardItemModel>
#include <data/analysis/inttrains/intervaltraininfo.h>


class QTableView;
class QCheckBox;
class QLineEdit;
class Diagram;
struct DiagramOptions;

class IntervalTrainModel: public QStandardItemModel
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    IntervalTrainList _data;
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
        ColMAX
    };
    IntervalTrainModel(const DiagramOptions& ops, QWidget* parent=nullptr);
    void refreshData();
    void setupModel();
    void resetData(IntervalTrainList&& data_);
    void resetData(const IntervalTrainList& data_);
    const auto& getData()const{return _data;}
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

