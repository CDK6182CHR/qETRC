#include "rulercontext.h"
#include "mainwindow.h"

RulerContext::RulerContext(Diagram& diagram_, SARibbonContextCategory *context, MainWindow *mw_):
    QObject(mw_),diagram(diagram_), cont(context),mw(mw_)
{
    initUI();
}

void RulerContext::setRuler(std::shared_ptr<Ruler> ruler)
{
    this->ruler=ruler;
    refreshData();
}

void RulerContext::refreshData()
{
    auto* page = cont->addCategoryPage(tr("标尺"));
    //todo...
}

void RulerContext::initUI()
{
    //todo...
}
