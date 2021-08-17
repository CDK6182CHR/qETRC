#pragma once

#include <QDialog>
#include <memory>

#include "util/selecttraincombo.h"

class Ruler;
class TrainCollection;
class QSpinBox;
class Railway;


/**
 * @brief The RulerFromTrainDialog class
 * 从单个车次读取标尺信息。
 */
class RulerFromTrainDialog : public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    std::shared_ptr<Ruler> ruler;

    SelectTrainCombo* cbTrain;
    QSpinBox* spStart,*spStop;
public:
    RulerFromTrainDialog(TrainCollection& coll_, std::shared_ptr<Ruler> ruler_,
                         QWidget* parent=nullptr);
signals:
    void rulerUpdated(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> newruler);
private:
    void initUI();
private slots:
    void onApply();
};

