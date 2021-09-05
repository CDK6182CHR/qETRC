#pragma once

#include <QSplitter>
#include <memory>

#include <model/general/qemoveablemodel.h>
#include "data/common/stationname.h"

class QCheckBox;
class QEControlledTable;
class QTableView;
class RoutingDiagram;
class Routing;

class RoutingStationModel: public QEMoveableModel
{
    Q_OBJECT
public:
    enum {
        ColName=0,
        ColYValue,
        ColMAX
    };
    RoutingStationModel(QObject* parent=nullptr);
    virtual void setupNewRow(int row) override;
    void setupModel(const QMap<StationName, double>& yvalues);
    /**
     * 将表中数据写到yvalues中
     */
    void getData(QMap<StationName,double>& yvalues);
};


/**
 * @brief The CircuitDiagramDialog class
 * pyETRC.circuitWidgets.CircuitDiagramWidget
 * 交路图窗口，改为QWidget，用Dock实现
 */
class RoutingDiagramWidget : public QSplitter
{
    Q_OBJECT
    std::shared_ptr<Routing> routing;
    RoutingDiagram*const routingDiagram;
    RoutingStationModel* const model;
    QEControlledTable* ctab;
    QTableView* table;

    QCheckBox* ckTimetable;
public:
    RoutingDiagramWidget(std::shared_ptr<Routing> routing_, QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    void _updateTable();
    void _repaint();
    void _autoPaint();
    void _outPdf();
    void _outPng();
};

