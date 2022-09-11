#pragma once
#include <QComboBox>
#include <QList>

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

    /**
     * 2022.09.11 设置当前状态为rail。
     */
    void setRailway(std::shared_ptr<const Railway> rail);

    /**
     * 2021.10.11  重载版本  用QList<Railway>构造临时对象
     * 注意考虑入参。空和仅有一个的都直接返回。
     */
    static std::shared_ptr<Railway> dialogGetRailway(const QList<std::shared_ptr<Railway>>& railways,
        QWidget* parent, const QString& title = tr("选择线路"), const QString& prompt = "");
signals:
    void currentRailwayChanged(std::shared_ptr<Railway>);
private slots:
    void onIndexChanged(int i);
};

