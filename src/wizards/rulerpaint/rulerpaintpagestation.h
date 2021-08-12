#pragma once

#include <QWizardPage>
#include <memory>
#include "util/buttongroup.hpp"
#include "data/common/direction.h"

class QComboBox;
class RailCategory;
class Railway;
class Ruler;
class RailStationModel;
class QTableView;
class RailRulerCombo;
class RailStation;

/**
 * @brief The RulerPaintPageStation class
 * 第2页，即选择线路、标尺、起始站的那一页
 * pyETRC中的起始站这里改成参考站，可以从任意车站作为锚点开始铺画
 */
class RulerPaintPageStation:
        public QWizardPage
{
    Q_OBJECT;
    RailCategory& cat;

    RailStationModel* const model;
    std::shared_ptr<Railway> _railway{};
    std::shared_ptr<Ruler> _ruler{};
    std::shared_ptr<const RailStation> _anchorStation;

    RailRulerCombo* cbRuler;
    RadioButtonGroup<2>* gpDir;
    QTableView* table;
public:
    RulerPaintPageStation(RailCategory& cat_, QWidget* parent=nullptr);
    Direction getDir()const;
    auto railway()const{return _railway;}
    auto ruler()const{return _ruler;}
    auto anchorStation()const{return _anchorStation;}
    virtual bool validatePage()override;
private:
    void initUI();
private slots:
    void onRailwayChanged(std::shared_ptr<Railway> railway);
    void onDirChanged();
};

