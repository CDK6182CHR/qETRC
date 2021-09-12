﻿#pragma once

#include <QWidget>
#include <memory>

class QTableView;
class QCheckBox;
class QLineEdit;
class TimetableQuickModel;
class TimetableQuickEditableModel;
class Train;
class QUndoStack;
class Railway;
class RailStation;

/**
 * @brief The TimetableQuickWidget class
 * 快速查看的只读列车时刻表  pyETRC的Ctrl+Y面板
 */
class TimetableQuickWidget : public QWidget
{
    Q_OBJECT
    std::shared_ptr<Train> train;
    TimetableQuickEditableModel* const model;

    QLineEdit* edName;
    QCheckBox* ckStopOnly, * ckEdit;
    QTableView* table;
public:
    explicit TimetableQuickWidget(QUndoStack* undo_, QWidget *parent = nullptr);
    auto getTrain(){return train;}
    auto getModel() { return model; }
private:
    void initUI();
signals:
    void locateToEvent(int pageIndex, std::shared_ptr<const Railway>,
        std::shared_ptr<const RailStation>, const QTime&);
private slots:
    void onStopOnlyChanged(bool on);
    void onEditCheckChanged(bool on);
    void locateArrive();
    void locateDepart();
public slots:
    void setTrain(std::shared_ptr<Train> train);
    void refreshData();

};

