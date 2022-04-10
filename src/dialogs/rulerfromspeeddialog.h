#pragma once

#include <QDialog>
#include <memory>

class QDoubleSpinBox;
class QCheckBox;
class Railway;
class QComboBox;
class QSpinBox;
class Ruler;

/**
 * @brief 2022.04.10
 * 从运行速度生成计算标尺。
 * 框架参照：RulerFromTrainDialog
 */
class RulerFromSpeedDialog : public QDialog
{
    Q_OBJECT
    std::shared_ptr<Ruler> ruler;
    QDoubleSpinBox* spSpeed;
    QSpinBox *spStart,*spStop;
    QComboBox* cbPrec;
    QCheckBox* ckAsMax;
public:
    RulerFromSpeedDialog(std::shared_ptr<Ruler> ruler, QWidget* parent=nullptr);
signals:
    void rulerUpdated(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> newruler);
private:
    void initUI();
private slots:
    void onApply();
};

