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

void RulerContext::actChangeRulerName(std::shared_ptr<Ruler> ruler, const QString& name)
{
    mw->getUndoStack()->push(new qecmd::ChangeRulerName(ruler, name, this));
}

void RulerContext::commitChangeRulerName(std::shared_ptr<Ruler> ruler)
{
    if (ruler == this->ruler) {
        refreshData();
    }
    mw->getRailContext()->onRulerNameChanged(ruler);
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

void qecmd::ChangeRulerName::undo()
{
    std::swap(ruler->nameRef(), name);
    cont->commitChangeRulerName(ruler);
}

void qecmd::ChangeRulerName::redo()
{
    std::swap(ruler->nameRef(), name);
    cont->commitChangeRulerName(ruler);
}
