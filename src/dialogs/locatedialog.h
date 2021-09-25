#pragma once
#include <QDialog>
#include "data/diagram/stationbinding.h"

struct TrainStationBounding;
class QDoubleSpinBox;
class RailStation;
class QTimeEdit;
class Railway;
class PageComboForRail;
class Diagram;
class QComboBox;
class QRadioButton;
class SelectRailwayCombo;


/**
 * @brief The LocateDialog class
 * 运行图定位对话框。
 */
class LocateDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    SelectRailwayCombo* cbRailway;
    QRadioButton* rdStation;
    QComboBox* cbStation;
    PageComboForRail* cbPage;
    QDoubleSpinBox* spMile;
    QTimeEdit* edTime;
public:
    LocateDialog(Diagram& diagram, QWidget* parent=nullptr);
    void refreshData();   // 刷新线路表，车站表，运行图表
private:
    void initUI();
    using QDialog::show;
signals:
    void locateOnMile(int pageIndex, std::shared_ptr<Railway>, double mile, const QTime& time);
    void locateOnStation(int pageIndex, std::shared_ptr<Railway>,
                         std::shared_ptr<RailStation>, const QTime& time);
private slots:
    void onRailwayChanged(std::shared_ptr<Railway> railway);
    void onRdStationToggled(bool on);
    void onApplied();
public slots:
    /**
     * @brief showDialog  刷新数据后显示
     */
    void showDialog();
};


/**
 * @brief The LocateBindingDialog class
 * 从TimetableQuickWidget调起的定位对话框。
 * 近似全局单例的作法；构造时，不给出数据。
 */
class LocateBoundingDialog:
       public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    QVector<TrainStationBounding> boudingList{};
    QComboBox* cbBound;
    PageComboForRail* cbPage;
    QTimeEdit* edTime;
public:
    LocateBoundingDialog(Diagram& diagram, QWidget* parent=nullptr);

    /**
     * 设置好数据，然后显示对话框。
     */
    void showForStation(const QVector<TrainStationBounding>& boudingList,
                        const QTime& tm);
private:
    void initUI();
    using QDialog::show;
signals:
    void locateOnStation(int pageIndex, std::shared_ptr<const Railway>,
                         std::shared_ptr<const RailStation>, const QTime& time);
private slots:
    void onApply();
    void onBoundComboChanged(int i);
};

