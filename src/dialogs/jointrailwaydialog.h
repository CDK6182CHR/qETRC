#pragma once

#include <QDialog>
#include <memory>

#include <util/buttongroup.hpp>

class QCheckBox;
class SelectRailwayCombo;
class Railway;
class RailCategory;

/**
 * @brief The JointRailwayDialog class
 * 2022.12.29 same as pyETRC, but the input railway should from
 * current diagram.
 */
class JointRailwayDialog : public QDialog
{
    Q_OBJECT
    RailCategory& railcat;
    std::shared_ptr<Railway> railway;
    SelectRailwayCombo* cbRail;

    RadioButtonGroup<2>* rdSeq;
    QCheckBox* ckReverse;
public:
    JointRailwayDialog(RailCategory& railcat, std::shared_ptr<Railway> railway,
                       QWidget* parent=nullptr);

private:
    void initUI();

signals:
    void applied(std::shared_ptr<Railway> rail, std::shared_ptr<Railway> rhs);
private slots:

    /**
     * @brief actApply
     * Only emits signal, do not actually apply.
     * This is because, it may be needed to reset ordinate before operation.
     */
    void actApply();
};

