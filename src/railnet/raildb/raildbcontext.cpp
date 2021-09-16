#include "raildbcontext.h"
#include "raildbnavi.h"
#include "raildbwindow.h"
#include "raildb.h"
#include "data/common/qesystem.h"
#include "mainwindow/mainwindow.h"
#include <DockWidget.h>
#include <DockManager.h>
#include <SARibbonContextCategory.h>
#include <SARibbonBar.h>

RailDBContext::RailDBContext(SARibbonContextCategory *cont, MainWindow *mw_):
    QObject(mw_), cont(cont),mw(mw_),
    window(new RailDBWindow()),_raildb(window->railDB())
{
    connect(window->getNavi(), &RailDBNavi::deactivated,
        this, &RailDBContext::onWindowDeactivated);
    initUI();
}

RailDBNavi* RailDBContext::getNavi()
{
    return window->getNavi();
}

void RailDBContext::initUI()
{
    auto* page=cont->addCategoryPage(tr("线路数据库"));
}

bool RailDBContext::deactiveOnClose()
{
    return window->getNavi()->deactiveOnClose();
}

void RailDBContext::onWindowDeactivated()
{
    mw->ribbonBar()->hideContextCategory(cont);
    if(dock)
        dock->closeDockWidget();
}

void RailDBContext::activateDB()
{
    if (dock == nullptr) {
        dock = new ads::CDockWidget(tr("线路数据库"));
        mw->getManager()->addDockWidgetTab(ads::CenterDockWidgetArea, dock);
        dock->setWidget(window);
    }
    if (dock->isClosed()) {
        dock->toggleView();
    }
    else {
        dock->setAsCurrentTab();
    }
    if (!_active) {
        window->getNavi()->openDB(SystemJson::instance.default_raildb_file);
        _active = true;
    }
    mw->ribbonBar()->showContextCategory(cont);
}

void RailDBContext::deactivateDB()
{
    window->deactive();
}
