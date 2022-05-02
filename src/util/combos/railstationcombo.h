#pragma once

#include <memory>
#include <QHBoxLayout>

class RailStation;
class Railway;
class QComboBox;
class RailCategory;

/**
 * @brief The RailStationCombo class
 * 2022.04.17  用于选择线路中车站的控件
 * 当线路或车站改变时发射信号
 * 注意初始化时【会发送信号】
 */
class RailStationCombo : public QHBoxLayout
{
    Q_OBJECT
    RailCategory& _railcat;
    QComboBox* cbRail,*cbStation;
    std::shared_ptr<Railway> _railway;
    std::shared_ptr<RailStation> _station;

public:
    RailStationCombo(RailCategory& railcat, QWidget* parent=nullptr);
    std::shared_ptr<Railway> railway()const {return _railway;}
    std::shared_ptr<RailStation> station()const {return _station;}
private:
    void initUI();
signals:
    void railwayChanged(std::shared_ptr<Railway>);
    void stationChanged(std::shared_ptr<RailStation>);
private slots:
    void onRailwayChanged(int i);
    void onStationChanged(int i);
    void setupStationCombo();
public slots:
    void refreshData();
};

