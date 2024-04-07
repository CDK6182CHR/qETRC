#pragma once

#include <QHBoxLayout>
#include <QComboBox>
#include <memory>

class RailCategory;
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
    bool withEmptyRuler;
    QString emptyName;
public:
    RailRulerCombo(RailCategory& cat_, QWidget* parent=nullptr );
    RailRulerCombo(RailCategory& cat_, const QString& emptyName_,
        QWidget* parent = nullptr);
    auto railway(){return _railway;}
    auto ruler(){return _ruler;}

    /**
     * 弹出对话框，选择标尺。如果没有选择，那么返回空
     */
    static std::shared_ptr<Ruler>
        dialogGetRuler(RailCategory& cat_, QWidget* parent, 
            const QString& title = tr("选择标尺"),const QString& prompt=tr(""));

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

    void refreshAll();

    void setRailwayIndex(int idx);
};


/**
 * 2022.02.07  对指定Railway中的Ruler的Combo
 */
class RulerCombo :public QComboBox
{
    Q_OBJECT
    std::shared_ptr<Railway> _railway;
    std::shared_ptr<Ruler> _ruler{};
public:
    RulerCombo(std::shared_ptr<Railway> railway, QWidget* parent = nullptr);
    auto ruler() { return _ruler; }
private:
    void refreshData();
private slots:
    void onCurrentChanged(int i);
};
