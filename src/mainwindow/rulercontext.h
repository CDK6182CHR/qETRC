#pragma once

#include <QObject>

#include <SARibbonContextCategory.h>

#include "data/rail/rail.h"
#include "data/diagram/diagram.h"

class MainWindow;

/**
 * @brief The RulerContext class
 * 当前标尺的Context
 */
class RulerContext : public QObject
{
    Q_OBJECT;
    std::shared_ptr<Ruler> ruler{};
    Diagram& diagram;
    SARibbonContextCategory*const cont;
    MainWindow* const mw;
public:
    explicit RulerContext(Diagram& diagram, SARibbonContextCategory* context, MainWindow* mw_);

    void setRuler(std::shared_ptr<Ruler> ruler);
    auto getRuler(){return ruler;}
    void refreshData();
    auto* context() { return cont; }

private:
    void initUI();


signals:

};

