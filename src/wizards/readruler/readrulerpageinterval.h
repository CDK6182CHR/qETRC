#pragma once
#include <QWizardPage>
#include <QVector>

#include "util/railrulercombo.h"

class RailCategory;
class QStandardItemModel;
class QListView;
class RailInterval;

/**
 * @brief The ReadRulerPageInterval class
 * 第2页  选择区间
 * 表示区间的model不进行subclass，而是直接在这里管理。
 */
class ReadRulerPageInterval : public QWizardPage
{
    Q_OBJECT;
    RailCategory&  cat;
    RailRulerCombo* cbRuler;

    std::shared_ptr<Railway> _railway{};
    std::shared_ptr<Ruler> _ruler{};
    QStandardItemModel *mdDown,*mdUp;
    QListView* lsDown,*lsUp;
    QVector<std::shared_ptr<RailInterval>> railints;
public:
    ReadRulerPageInterval(RailCategory& cat_, QWidget* parent=nullptr);
    auto railway(){return _railway;}
    auto ruler(){return _ruler;}
    const auto& getIntervals()const { return railints; }
    virtual bool validatePage()override;
private:
    void initUI();
private slots:
    void setRailway(std::shared_ptr<Railway> railway);
    void selectAllDown();
    void deselectAllDown();
    void selectAllUp();
    void deselectAllUp();
};

