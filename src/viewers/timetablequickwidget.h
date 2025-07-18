#pragma once

#include <QWidget>
#include <memory>
#include "data/diagram/stationbinding.h"

class LocateBoundingDialog;
class QTableView;
class QCheckBox;
class QLineEdit;
class TimetableQuickModel;
class TimetableQuickEditableModel;
class Train;
class QUndoStack;
class Railway;
class RailStation;
class QMenu;
struct DiagramOptions;
class TrainTime;

/**
 * @brief The TimetableQuickWidget class
 * 快速查看的只读列车时刻表  pyETRC的Ctrl+Y面板
 */
class TimetableQuickWidget : public QWidget
{
    Q_OBJECT
    const DiagramOptions& _ops;
    std::shared_ptr<Train> train;
    TimetableQuickEditableModel* const model;

    QLineEdit* edName;
    QCheckBox* ckStopOnly, * ckEdit;
    QTableView* table;
    LocateBoundingDialog* dlgLocate=nullptr;
    QMenu* meContext;
public:
    explicit TimetableQuickWidget(const DiagramOptions& ops, QUndoStack* undo_, QWidget *parent = nullptr);
    auto getTrain(){return train;}
    auto getModel() { return model; }
    void setReadonly();
private:
    void initUI();
signals:

    /**
     * 因为本类没有Diagram引用，无法处理定位问题。
     * 只能发送信号出去，暂定给TrainContext处理
     */
    void locateToBoundStation(const TrainStationBoundingList& bound,
        const TrainTime& time);

private slots:
    void onStopOnlyChanged(bool on);
    void onEditCheckChanged(bool on);
    void locateArrive();
    void locateDepart();
    void onTableContext(const QPoint& pos);
public slots:
    void setTrain(std::shared_ptr<Train> train);
    void resetTrain();
    void refreshData();
    
};

