﻿#pragma once
#include <QWidget>
#include <memory>

class Routing;
class QPushButton;
class QDoubleSpinBox;
class PenStyleCombo;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QTableView;
class QEControlledTable;
class TimetableStdModel;
class Train;
class TrainCollection;
class TimetableWidget;
class QToolButton;

/**
 * @brief The EditTrainWidget class
 * pyETRC.CurrentWidget
 * 列车全部数据编辑
 * 2024.04.15 Add synchronize option: to synchronize with the current selected train.
 */

class EditTrainWidget : public QWidget
{
    Q_OBJECT
    TrainCollection& coll;
    std::shared_ptr<Train> _train;

    QLineEdit* edTrainName,*edDownName,*edUpName;
    QLineEdit* edStaring,*edTerminal;
    QComboBox* cbType;
    QCheckBox* ckPassenger;

    QCheckBox* ckAutoPen;
    QDoubleSpinBox* spWidth;
    PenStyleCombo* cbLs;
    QPushButton* btnColor;
    QLineEdit* edRouting;

    QPushButton* btnToRouting;
    QToolButton* btnSync;

    TimetableWidget* ctable;
    QTableView* table;

    QColor tmpColor;
public:
    explicit EditTrainWidget(TrainCollection& coll, std::shared_ptr<Train> train,
                             QWidget *parent = nullptr);
    auto train(){return _train;}
    void setTrain(const std::shared_ptr<Train>& train);
    void resetTrain();   // 2024.04.16  equivalent to setTrain(nullptr)
    void refreshData();
    void refreshBasicData();
    TimetableStdModel* getModel();
    bool isSynchronized()const;

private:
    void initUI();

signals:
    void trainInfoChanged(std::shared_ptr<Train> train,
                          std::shared_ptr<Train> info);
    void switchToRouting(std::shared_ptr<Routing> routing);
    void removeTrain(std::shared_ptr<Train> train);
    void synchronizationChanged(bool on);
private slots:
    void actApply();
    void actCancel();
    void actRemove();

    void onAutoPenToggled(bool on);
    void setLineColor();
    void actSwitchToRouting();

    void setupTypeCombo();
    void onTrainFullNameUpdate();
};

