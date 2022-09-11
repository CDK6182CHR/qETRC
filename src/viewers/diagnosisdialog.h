#pragma once

#include <QDialog>
#include <QStandardItemModel>

#include "data/diagram/trainevents.h"

class Railway;
class RailRangeCombo;
class SelectRailwayCombo;
class QTableView;
class QCheckBox;
class QRadioButton;

class Diagram;

/**
 * @brief The DiagnosisModel class
 * 只读的错误列表model
 */
class DiagnosisModel: public QStandardItemModel
{
    Q_OBJECT;
    Diagram& diagram;
    DiagnosisList lst;
public:
    enum {
        ColTrainName=0,
        ColRailway,
        ColPos,
        ColLevel,
        ColType,
        ColDescription,
        ColMAX
    };
    DiagnosisModel(Diagram& diagram_, QObject* parent=nullptr);
private:
    void setupModel();
public slots:
    void setupForTrain(std::shared_ptr<Train> train, 
                       std::shared_ptr<Railway> railway,
                       std::shared_ptr<RailStation> start,
                       std::shared_ptr<RailStation> end);
    void setupForAll(
                     std::shared_ptr<Railway> railway,
                     std::shared_ptr<RailStation> start,
                     std::shared_ptr<RailStation> end);
};

class SelectTrainCombo;

/**
 * @brief The DiagnosisDialog class
 * 列车时刻表问题检查对话框
 */
class DiagnosisDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    DiagnosisModel* const model;

    SelectTrainCombo* cbTrain;
    QRadioButton* rdSingle;
    //QCheckBox* ckIntMeet;
    QTableView* table;

    SelectRailwayCombo* cbRail;
    RailRangeCombo* cbRange;
    QCheckBox* ckFiltRail,*ckFiltRange;
public:
    DiagnosisDialog(Diagram& diagram_, QWidget* parent = nullptr);
    DiagnosisDialog(Diagram& diagram_, std::shared_ptr<Train> train, 
        QWidget* parent = nullptr);
private:
    void initUI();

    /**
     * 如果勾选了筛选线路，返回线路；否则返回空
     */
    std::shared_ptr<Railway> getFilterRailway();
    std::pair<std::shared_ptr<RailStation>,std::shared_ptr<RailStation>>
        getFilterRange();
signals:
    void showStatus(const QString&);
private slots:
    void actApply();
    void onSingleToggled(bool on);
    void actHelp();

    void onFiltRailChanged(bool on);
    void onFiltRangeChanged(bool on);
};

