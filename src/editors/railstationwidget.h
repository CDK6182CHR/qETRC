#pragma once

#include <QWidget>
#include <QUndoStack>

#include "util/qecontrolledtable.h"
#include "model/rail/railstationmodel.h"

/**
 * @brief The RailStationWidget class
 * 线路的里程表编辑页面
 * pyETRC.LineWidget
 */
class RailStationWidget : public QWidget
{
    Q_OBJECT
    QEControlledTable* ctable;
    RailStationModel* model;
    QUndoStack* _undo;
    std::shared_ptr<Railway> railway;
public:
    explicit RailStationWidget(QUndoStack* undo, QWidget *parent = nullptr);

    void setRailway(std::shared_ptr<Railway> rail);

private:
    void initUI();

signals:

private slots:
    void actApply();
    void actCancel();

};


