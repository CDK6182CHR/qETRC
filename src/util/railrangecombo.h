#pragma once

#include <QWidget>
#include <memory>

class QComboBox;
class RailStation;
class Railway;
/**
 * @brief The RailRangeCombo class
 * 选择线路范围的Combo。采用QWidget，是为了应用setEnabled()的slot
 * 包含起始站和结束站；可设置当前的线路（初始化时不设置）；
 * 暂定空线路不执行任何操作，不发射任何信号。
 */
class RailRangeCombo : public QWidget
{
    Q_OBJECT
    std::shared_ptr<Railway> railway{};
    std::shared_ptr<RailStation> _start{},_end{};

    QComboBox* cbStart,*cbEnd;
public:
    explicit RailRangeCombo(QWidget *parent = nullptr);
    auto start(){return _start;}
    auto end(){return _end;}
    auto getRailway(){return railway;}

    /**
     * @brief refreshData
     * 等同于设置线路后进行的刷新操作；将Combo置为第一、最后。
     */
    void refreshData();
private:
    void initUI();

signals:
    void startStationChanged(std::shared_ptr<RailStation>);
    void endStationChanged(std::shared_ptr<RailStation>);
    void stationRangeChanged(std::shared_ptr<RailStation> start,
                             std::shared_ptr<RailStation> end);
public slots:
    void setRailway(std::shared_ptr<Railway> railway);
private slots:

    /**
     * 起始站下标变更，重新设置终到站的Combo范围。
     * 如果终到站仍然合法，则保持终到站不变。
     * 注意发射信号前后进行检查。
     * 注意如果终到站变为非法，则不发射RangeChanged信号
     */
    void onStartIndexChanged(int i);
    void onEndIndexChanged(int i);
};

