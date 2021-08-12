#pragma once

#include <QHBoxLayout>
#include <memory>

class RailCategory;
class QComboBox;
class Railway;
class Ruler;
/**
 * @brief The RailRulerCombo class
 * 两个并排的ComboBox，选择线路以及标尺
 */
class RailRulerCombo : public QHBoxLayout
{
    Q_OBJECT;
    RailCategory& cat;
    QComboBox* cbRail,*cbRuler;
    std::shared_ptr<Railway> _railway;
    std::shared_ptr<Ruler> _ruler;
public:
    RailRulerCombo(RailCategory& cat_, QWidget* parent=nullptr );
    auto railway(){return _railway;}
    auto ruler(){return _ruler;}

private:
    void initUI();

signals:
    void railwayChagned(std::shared_ptr<Railway>);
    void rulerChanged(std::shared_ptr<Ruler>);
private slots:
    void onRailIndexChanged(int i);
    void onRulerIndexChanged(int i);
public slots:
    /**
     * @brief refreshRulerList
     * 重新加载标尺列表，可能导致选择项变化
     * precondition: 当前线路不变且有效
     */
    void refreshRulerList();

    /**
     * @brief refreshRailwayList
     * 重新加载线路列表，可能导致选择的线路、标尺都变化
     */
    void refreshRailwayList();
};

