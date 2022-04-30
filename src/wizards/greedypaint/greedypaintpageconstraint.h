#pragma once
#include <QWidget>
#include <util/buttongroup.hpp>
#include <array>


class SelectForbidModel;
class QListView;
class Diagram;
class GapConstraintModel;
class QTableView;
class GreedyPainter;
class QCheckBox;
class QSpinBox;
class TrainFilter;
class RailRulerCombo;
namespace gapset {
class GapSetAbstract;
}
/**
 * @brief The GreedyPaintPageConstraint class
 */
class GreedyPaintPageConstraint : public QWidget
{
    Q_OBJECT;
    RailRulerCombo* cbRuler;
    QSpinBox* spBack;
    QCheckBox* ckSingle;

    Diagram& diagram;
    GreedyPainter& painter;
    QTableView* table;
    QListView* lstForbid;
    GapConstraintModel*  _model;
    SelectForbidModel* const _mdForbid;

    static constexpr const int GAP_SET_COUNT=2;
    RadioButtonGroup<GAP_SET_COUNT>* gpGapSet;
    std::array<std::unique_ptr<gapset::GapSetAbstract>,GAP_SET_COUNT> _availableGapSets;

    TrainFilter* filter;
    QSpinBox* spMinGap, * spMaxGap;
public:
    explicit GreedyPaintPageConstraint(
            Diagram& _diagram,
            GreedyPainter& _painter,
            QWidget *parent = nullptr);
private:
    void initUI();
    void initGapSets();

signals:

    /**
     * @brief onApplied
     * 排图参数确认，提交给上级，转到下一页
     */
    void constraintChanged();

    void actClose();

private slots:
    void onApply();
    void onSingleLineChanged(bool on);
    void onGapSetToggled(int id, bool on);
    void onGetGapFromCurrent();
};

