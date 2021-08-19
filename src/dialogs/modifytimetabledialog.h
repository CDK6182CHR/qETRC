#pragma once
#include <QDialog>
#include <memory>
#include "util/buttongroup.hpp"
#include "model/train/timetablestdmodel.h"
#include "viewers/traintimetableplane.h"

class Train;
class QSpinBox;
class QTableView;
class QCheckBox;

/**
 * @brief The ModifyTimetableDialog class
 * 微调当前列车时刻  pyETRC传承功能
 *
 */
class ModifyTimetableDialog : public QDialog
{
    Q_OBJECT;
    const std::shared_ptr<Train> train;
    TrainTimetablePlane*const table;

    RadioButtonGroup<2>* gpDir;
    QSpinBox *spMin,*spSec;
    QCheckBox* ckFirst,*ckLast;

public:
    ModifyTimetableDialog(std::shared_ptr<Train> train_, QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void trainUpdated(std::shared_ptr<Train> train,std::shared_ptr<Train> table);
private slots:
    void onApply();
};

