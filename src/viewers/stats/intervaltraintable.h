#pragma once

#include <QTableView>
#include <QStandardItemModel>
#include <data/analysis/inttrains/intervaltraininfo.h>


class QTableView;
class QCheckBox;
class QLineEdit;
class Diagram;
class IntervalTrainModel: public QStandardItemModel
{
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
    IntervalTrainModel(QWidget* parent=nullptr);
    void refreshData();
    void setupModel();
    void resetData(IntervalTrainList&& data_);
    const auto& getData()const{return _data;}
};



/**
 * @brief The IntervalTrainTable class
 * Model/View合流
 * 区间列车表
 */
class IntervalTrainTable : public QTableView
{
    IntervalTrainModel* const model;
public:
    IntervalTrainTable(QWidget* parent=nullptr);
    auto* getModel(){return model;}
private:
    void initUI();
};

