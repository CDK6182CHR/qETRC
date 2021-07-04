#pragma once

#include <QObject>
#include "SARibbonContextCategory.h"
#include "data/rail/rail.h"
#include "data/diagram/diagram.h"

class MainWindow;
/**
 * @brief The RailContext class
 * 代理线路的ContextCategory操作
 */
class RailContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    SARibbonContextCategory*const cont;
    MainWindow* const mw;
    std::shared_ptr<Railway> railway;
public:
    explicit RailContext(Diagram& diagram_, SARibbonContextCategory* context,
        MainWindow* mw_,
        QObject* parent = nullptr);

    inline auto context() { return cont; }

    void setRailway(std::shared_ptr<Railway> rail);

private:
    void initUI();

signals:

private slots:
    void actOpenStationWidget();

};

