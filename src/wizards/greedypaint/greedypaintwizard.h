﻿#pragma once

#include <QTabWidget>

#include <data/calculation/greedypainter.h>

class GreedyPaintPagePaint;
class GreedyPaintPageConstraint;
class TrainFilterSelector;
class DiagramWidget;
class Train;
struct AdapterStation;


/**
 * @brief The GreedyPaintWizard class
 * 贪心推线的完整向导。
 * 与标尺排图不同，这里打算采用QTabWidget，弱化向导的特征，
 * 但类名暂时还是叫Wizard。
 * 配置了第一页之后，可以反复使用第二页铺画车次。
 */
class GreedyPaintWizard : public QTabWidget
{
    Q_OBJECT
    Diagram& diagram;
    TrainFilterSelector* const filter;   // mind, the actually object belongs to PageConstraint
    GreedyPainter painter;
    GreedyPaintPageConstraint* pgConst;
    GreedyPaintPagePaint* pgPaint;

    bool showCloseMsg=true;
public:
    GreedyPaintWizard(Diagram& diagram_, QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void trainAdded(std::shared_ptr<Train>);
    void showStatus(const QString&);
    void removeTmpTrainLine(const Train& train);
    void paintTmpTrainLine(std::shared_ptr<Train>);
private slots:
    void onConstraintChanged();
    void onClose();

public slots:
    /**
     * 2022.06.02：如果线路站表发生非等价变更，
     * 需要重新生成标尺表格数据，调用这个。
     */
    void refreshData(std::shared_ptr<Railway> rail);

    /**
     * 2023.10.04  clean up temporary data before exiting paining mode. 
     * Currently, mainly clear temp train lines.
     */
    void cleanUpTempData();

    void onPaintingPointClicked(DiagramWidget* d, std::shared_ptr<Train> train, AdapterStation* st);

    /**
     * 2024.04.07: Check status on ruler or railway is removed to avoid dangerous cases.
     */
    void onRulerRemoved(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> rail);

    void onRailwayRemoved(std::shared_ptr<Railway> rail);
};

