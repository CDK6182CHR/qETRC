#include "pagecontext.h"
#include "mainwindow.h"

PageContext::PageContext(Diagram &diagram_, SARibbonContextCategory *context,
                         MainWindow *mw_):
    QObject(mw_),diagram(diagram_),cont(context),mw(mw_)
{
    initUI();
}

void PageContext::setPage(std::shared_ptr<DiagramPage> page)
{
    this->page = page;
    refreshData();
}

void PageContext::resetPage()
{
    this->page.reset();
    refreshData();
}

void PageContext::refreshData()
{
    //小心nullptr...
}

void PageContext::initUI()
{
    auto* page = cont->addCategoryPage(tr("管理"));
    auto* panel = page->addPannel(tr(""));

    QAction* act = new QAction(QIcon(":/icons/close.png"), tr("删除"), this);
    panel->addLargeAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemovePage()));
}

void PageContext::actRemovePage()
{
    emit pageRemoved(diagram.getPageIndex(page));
}
