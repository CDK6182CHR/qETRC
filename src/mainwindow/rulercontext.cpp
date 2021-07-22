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

void RulerContext::actChangeRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> nr)
{
    mw->getUndoStack()->push(new qecmd::UpdateRuler(ruler, nr, this));
}

void RulerContext::commitRulerChange(std::shared_ptr<Ruler> ruler)
{
    mw->getRailContext()->refreshRulerTable(ruler);
    if (ruler->isOrdinateRuler()) {
        emit ordinateRulerModified(ruler->railway());
    }
}

void qecmd::UpdateRuler::undo()
{
    ruler->swap(*(nr->getRuler(0)));
    cont->commitRulerChange(ruler);
}

void qecmd::UpdateRuler::redo()
{
    ruler->swap(*(nr->getRuler(0)));
    cont->commitRulerChange(ruler);
}
