#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <data/analysis/inttrains/intervalcounter.h>
#include <util/buttongroup.hpp>

class IntervalTrainTable;
class QTableView;
class Diagram;
class TrainFilterSelector;
class QCheckBox;

/**
 * Model: 每个站都设置做好数据，但只显示需要的部分
 * （显示由table来控制）
 */
class IntervalCountModel: public QStandardItemModel
{
    Q_OBJECT
    RailIntervalCount _data;
    IntervalCounter& counter;
    std::shared_ptr<const RailStation> center;
    std::shared_ptr<const Railway> railway;
    bool isStart = false;
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
    IntervalCountModel(IntervalCounter& counter, QObject* parent=nullptr);
    void resetData(RailIntervalCount&& data_, std::shared_ptr<const RailStation> center, bool isStart, 
        std::shared_ptr<const Railway> rail);
    const auto& getData()const { return _data; }
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
    TrainFilterSelector* const filter;

    QTableView* table;
    IntervalTrainTable* detailTable=nullptr;

    IntervalCounter counter;
    IntervalCountModel* const model;

public:
    IntervalCountDialog(Diagram& diagram,QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    void refreshData();
    void refreshShow();
    void onDoubleClicked();
    void toCsv();
};

