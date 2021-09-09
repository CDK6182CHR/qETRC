#pragma once

#include <QDialog>
#include "data/rail/railway.h"
#include <memory>
#include <QTableView>
#include "model/rail/railstationmodel.h"

/**
 * @brief The SelectRailStationDialog class
 * 选择指定线路中的车站的对话框。用于输出车站时刻表等
 */
class SelectRailStationDialog : public QDialog
{
    Q_OBJECT
    const std::shared_ptr<Railway> railway;

    RailStationModel* const model;
    QTableView* table;
    std::shared_ptr<RailStation> _station;
public:
    SelectRailStationDialog(std::shared_ptr<Railway> rail,QWidget* parent=nullptr);
    static std::shared_ptr<RailStation> getStation(std::shared_ptr<Railway> rail,
        QWidget* parent);
    auto station() { return _station; }
private:
    void initUI();
signals:
    void stationSelected(std::shared_ptr<RailStation> station);
private slots:
    void actApply();
    void onItemDoubleClicked(const QModelIndex& index);
};

