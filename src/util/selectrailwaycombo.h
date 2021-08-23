#pragma once
#include <QComboBox>

#include "data/rail/railcategory.h"

/**
 * @brief The SelectRailwayCombo class
 * 一个简单的选择线路的Combo，其实可有可无。。
 */
class SelectRailwayCombo : public QComboBox
{
    Q_OBJECT;
    RailCategory& cat;
    std::shared_ptr<Railway> _railway;
public:
    SelectRailwayCombo(RailCategory& cat_, QWidget* parent=nullptr);
    void refresh();
    auto railway(){return _railway;}
    static std::shared_ptr<Railway> dialogGetRailway(RailCategory& cat, QWidget* parent,
        const QString& title = tr("选择线路"), const QString& prompt = "");
signals:
    void currentRailwayChanged(std::shared_ptr<Railway>);
private slots:
    void onIndexChanged(int i);
};

