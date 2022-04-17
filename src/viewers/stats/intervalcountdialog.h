#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <data/analysis/inttrains/intervalcounter.h>
#include <util/buttongroup.hpp>

class IntervalTrainTable;
class QTableView;
class Diagram;
class TrainFilter;
class QCheckBox;

class IntervalCountModel: public QStandardItemModel
{
    Q_OBJECT
    RailIntervalCount _data;
public:
    enum {
        ColFrom=0,
        ColTo,
        ColTotal,
        ColStart,
        ColEnd,
        ColStartEnd,
        ColMAX
    };
    IntervalCountModel(QObject* parent=nullptr);
    void resetData(RailIntervalCount&& data_);
    IntervalCountInfo& dataForRow(int i)const;
    std::shared_ptr<const RailStation> stationForRow(int i)const;
public slots:
    void refreshData();
    void setupModel();
};



class RailStationCombo;
class IntervalCountDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    RailStationCombo* cbStation;
    RadioButtonGroup<2>* rdStart;
    QCheckBox* ckPassenger,*ckFreight;
    QCheckBox* ckBusiness,*ckStop;
    TrainFilter* const filter;

    IntervalCountModel* const model;
    QTableView* table;
    IntervalTrainTable* detailTable=nullptr;

    IntervalCounter counter;

public:
    IntervalCountDialog(Diagram& diagram,QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    void refreshData();
    void onDoubleClicked();
    void toCsv();
};

