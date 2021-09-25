#pragma once

#include <QWidget>
#include <memory>
#include "util/buttongroup.hpp"

class Railway;
class RailNet;
class QSpinBox;
class QCheckBox;
class QEControlledTable;
class QEMoveableModel;
class QTableView;

/**
 * @brief The QuickPathSelected class
 * 基于最短路算法的经由选择器。
 */
class QuickPathSelector : public QWidget
{
    Q_OBJECT
    RailNet& net;
    QEControlledTable* ctbDown,*ctbUp;
    QTableView* tbDown,*tbUp;
    QEMoveableModel* mdDown,*mdUp;

    QCheckBox* ckRuler;
    RadioButtonGroup<3>* gpUp;
    QSpinBox* spRuler;
    bool informForce = true;
public:
    explicit QuickPathSelector(RailNet& net, QWidget *parent = nullptr);

private:
    void initUI();
    QVector<QString> pathFromModel(const QEMoveableModel* model);

signals:
    void railGenerated(std::shared_ptr<Railway>, const QString& pathString);
private slots:
    void actGenerate();
    void actForce();
    void onUpModeChanged();
};

