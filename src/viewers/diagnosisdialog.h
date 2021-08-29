#pragma once

#include <QDialog>
#include <QStandardItemModel>

#include "data/diagram/trainevents.h"

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
    void setupForTrain(std::shared_ptr<Train> train, bool withIntMeet);
    void setupForAll(bool withIntMeet);
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
    QCheckBox* ckIntMeet;
    QTableView* table;
public:
    DiagnosisDialog(Diagram& diagram_, QWidget* parent = nullptr);
    DiagnosisDialog(Diagram& diagram_, std::shared_ptr<Train> train, 
        QWidget* parent = nullptr);
private:
    void initUI();
private slots:
    void actApply();
    void onSingleToggled(bool on);
    void actHelp();
};

