#ifndef QETRC_MOBILE_2
#include "pagecontext.h"
#include "mainwindow.h"

#include "dialogs/printdiagramdialog.h"
#include "data/diagram/diagrampage.h"
#include "editors/configdialog.h"
#include "viewcategory.h"
#include "util/selectrailwaycombo.h"
#include "navi/addpagedialog.h"
#include "defines/icon_specs.h"
#include "util/combos/selectpagecombo.h"

#include <DockWidget.h>
#include <QLabel>
#include <QApplication>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLineEdit>
#include <SARibbonContextCategory.h>
#include <SARibbonMenu.h>
#include <QTextEdit>


PageContext::PageContext(Diagram &diagram_, SARibbonContextCategory *context,
                         MainWindow *mw_):
    QObject(mw_),diagram(diagram_),mw(mw_),cont(context)
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
    if (page) {
        edName->setText(page->name());
    }
}

void PageContext::refreshAllData()
{
    refreshData();
}

void PageContext::initUI()
{
    auto* page = cont->addCategoryPage(tr("运行图(&5)"));
    auto* panel = page->addPanel(tr(""));

    //基本信息部分
    if constexpr (true) {
        auto* ed = new QLineEdit;
        edName = ed;
        ed->setFocusPolicy(Qt::NoFocus);
        ed->setAlignment(Qt::AlignCenter);
        ed->setFixedWidth(120);

        auto* w = new QWidget;
        auto* vlay = new QVBoxLayout;
        auto* hlay = new QHBoxLayout;
        auto* label = new QLabel(tr("当前运行图"));
        label->setAlignment(Qt::AlignCenter);
        hlay->addWidget(label);
        auto* btn = new SARibbonToolButton;
        btn->setIcon(QEICN_change_page);
        btn->setText(tr("切换"));
        hlay->addWidget(btn);
        vlay->addLayout(hlay);
        connect(btn, &QToolButton::clicked, this, &PageContext::actChangePage);

        vlay->addWidget(ed);

        w->setLayout(vlay);
        w->setWindowTitle(tr("运行图页面名"));
        w->setObjectName(tr("页面名面板"));
        panel->addWidget(w, SARibbonPanelItem::Large);
    }

    panel = page->addPanel(tr("基本"));

    auto* act = mw->makeAction(QEICN_edit_page, tr("编辑"), tr("编辑运行图页面"));
    connect(act, SIGNAL(triggered()), this, SLOT(actEdit()));

    auto* me = new SARibbonMenu(mw);
    me->addAction(tr("重设页面"), this, &PageContext::actResetPage);
    act->setMenu(me);

    panel->addLargeAction(act, QToolButton::MenuButtonPopup);
  

    act = mw->makeAction(QEICN_export_page, tr("导出"), tr("导出运行图页面"));
    panel->addLargeAction(act);
    //btn->setMinimumWidth(70);
    connect(act, SIGNAL(triggered()), this, SLOT(actPrint()));

    act = mw->makeAction(QEICN_appearance_config_page, tr("显示设置"), tr("页面显示设置"));
    connect(act, &QAction::triggered, this, &PageContext::actConfig);
    me = new SARibbonMenu(mw);
    me->addAction(tr("将运行图显示设置应用到当前页面"), this,
        &PageContext::actUseDiagramConfig);
    act->setMenu(me);
    panel->addLargeAction(act, QToolButton::MenuButtonPopup);
    //btn->setMinimumWidth(80);

    act = mw->makeAction(QEICN_auto_top_bottom_margin, tr("自动上下边距"));
    connect(act, &QAction::triggered, this, &PageContext::actAutoTopDownMargin);
    panel->addLargeAction(act);

    act->setToolTip(tr("显示设置\n设置当前运行图页面的显示参数，不影响其他运行图页面。"));

    panel = page->addPanel("导航");

    act = mw->makeAction(QEICN_switch_to_page, tr("转到..."), tr("转到页面"));
    act->setToolTip(tr("转到运行图页面\n打开或者切换到当前运行图页面。"));
    connect(act, &QAction::triggered, this, &PageContext::actActivatePage);
    panel->addLargeAction(act);
    //btn->setMinimumWidth(70);

    act = mw->makeAction(QEICN_edit_rail_page, tr("编辑线路"), tr("编辑页面线路"));
    act->setToolTip(tr("编辑线路\n编辑当前运行图页面中的线路数据"));
    connect(act, &QAction::triggered, this, &PageContext::actEditRailway);
    me = new SARibbonMenu(mw);
    me->addAction(tr("转到线路"), this, &PageContext::actSwitchToRailway);
    act->setMenu(me);
    panel->addLargeAction(act, QToolButton::MenuButtonPopup);
    //btn->setMinimumWidth(80);

    panel = page->addPanel(tr("比例控制"));
    act = mw->makeAction(QEICN_h_expand, tr("水平放大"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, this, &PageContext::hExpand);

    act = mw->makeAction(QEICN_h_shrink, tr("水平缩小"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, this, &PageContext::hShrink);

    act = mw->makeAction(QEICN_v_expand, tr("垂直放大"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, this, &PageContext::vExpand);

    act = mw->makeAction(QEICN_v_shrink, tr("垂直缩小"));
    panel->addMediumAction(act);
    connect(act, &QAction::triggered, this, &PageContext::vShrink);

    panel = page->addPanel("");

    act = mw->makeAction(QEICN_page_copy, tr("副本"), tr("页面副本"));
    act->setToolTip(tr("创建运行图页面副本\n创建当前运行图页面的副本。注意页面中的基线并不会被复制。"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &PageContext::actDulplicate);

    act = mw->makeAction(QEICN_del_page, tr("删除"), tr("删除页面"));
    act->setToolTip(tr("删除运行图\n删除当前运行图页面，"
        "但线路、车次信息不会被删除。"));
    panel->addLargeAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemovePage()));

    act = mw->makeAction(QEICN_close_page_context, tr("关闭面板"), tr("关闭页面面板"));
    act->setToolTip(tr("关闭面板\n关闭当前的运行图页面上下文工具栏页面"));
    connect(act, &QAction::triggered, mw, &MainWindow::focusOutPage);
    panel->addLargeAction(act);
    //btn->setMinimumWidth(80);
}

std::shared_ptr<Railway> PageContext::selectRailway()
{
    if (!page)return nullptr;
    auto rail = SelectRailwayCombo::dialogGetRailway(page->railways(), mw, tr("选择线路"),
        tr("本运行图包含多条线路，请选择一条线路"));
    return rail;
}

void PageContext::actEdit()
{
    auto* dialog = new EditPageDialog(page,diagram, mw);
    connect(dialog, &EditPageDialog::editApplied, this, &PageContext::onEditApplied);
    dialog->open();
}

void PageContext::actPrint()
{
    int idx = diagram.getPageIndex(page);
    if (idx != -1) {
        auto* dw = mw->diagramWidgets.at(idx);
        auto* dialog = new PrintDiagramDialog(dw, mw);
        dialog->open();
    }
}

void PageContext::actConfig()
{
    auto* dlg = new ConfigDialog(page->configRef(), diagram.options().period_hours, page, mw);
    connect(dlg, &ConfigDialog::onPageConfigApplied,
        mw->getViewCategory(), &ViewCategory::onActPageConfigApplied);
    dlg->show();
}

void PageContext::actActivatePage()
{
    if (!page)return;
    mw->activatePageWidget(diagram.getPageIndex(page));
}

void PageContext::actUseDiagramConfig()
{
    if (!page)return;
    auto flag = QMessageBox::question(mw, tr("提示"), tr("此操作将当前运行图文件的显示设置信息覆盖到"
        "当前运行图页面[%1]。是否确认？\n "
        "要修改运行图文件的显示设置、或将其应用到所有运行图页面，请转到工具栏[显示]页面。")
        .arg(page->name()));
    if (flag != QMessageBox::Yes)
        return;
    mw->getUndoStack()->push(new qecmd::ChangePageConfig(page->configRef(), diagram.config(),
        true, page, mw->getViewCategory()));
}

void PageContext::commitResetPage(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo)
{
    page->swap(*newinfo);
    if (page->name() != newinfo->name()) {
        int i = mw->_diagram.getPageIndex(page);
        mw->diagramDocks.at(i)->setWindowTitle(page->name());
        emit pageNameChanged(i);
    }
    mw->updatePageDiagram(page);
    refreshData();
}

void PageContext::actEditRailway()
{
    if (auto rail = selectRailway()) {
        mw->actOpenRailStationWidget(rail);
    }
}

void PageContext::actSwitchToRailway()
{
    if (auto rail = selectRailway()) {
        mw->focusInRailway(rail);
    }
}

void PageContext::actAutoTopDownMargin()
{
    if (!page) return;
    Config cfg_new = page->config();  // copy construct

    // 35 for the top axis of time, which is fixed in current impl.
    // the extra 5 makes it clearer. 
    cfg_new.margins.up = page->topLabelMaxHeight() + 35 + 5;

    // here the constant 10 comes from the literal 27 in updateTimeAxis.
    // that is: 35 - 27 = 8, and we get a "ceil" of it.
    cfg_new.margins.down = page->bottomLabelMaxHeight() + 35 + 10 + 5;

    mw->undoStack->push(new qecmd::ChangePageConfig(page->configRef(), cfg_new, true, page, mw->catView));
}

void PageContext::actResetPage()
{
    if (!page)return;
    auto* dlg = new AddPageDialog(diagram, page, mw);
    connect(dlg, &AddPageDialog::modificationDone,
        this, &PageContext::onResetApplied);
    dlg->show();
}

void PageContext::onResetApplied(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> data)
{
    mw->getUndoStack()->push(new qecmd::ResetPage(page, data, this));
}

void PageContext::actDulplicate()
{
    if (page)
        emit dulplicatePage(page);
}

void PageContext::hExpand()
{
    auto& cfg = page->configRef();
    if (cfg.seconds_per_pix > 1) {
        Config newcfg(cfg);    // default copy ctor
        newcfg.seconds_per_pix -= 1;
        mw->getViewCategory()->onActPageScaleApplied(cfg, newcfg, true, page);
    }
}

void PageContext::hShrink()
{
    auto& cfg = page->configRef();
    Config newcfg(cfg);    // default copy ctor
    newcfg.seconds_per_pix += 1;
    mw->getViewCategory()->onActPageScaleApplied(cfg, newcfg, true, page);
}

void PageContext::vExpand()
{
    auto& cfg = page->configRef();
    Config newcfg(cfg);
    if (cfg.seconds_per_pix_y>1) {
        newcfg.seconds_per_pix_y -= 1;
    }
    newcfg.pixels_per_km += 1;
    mw->getViewCategory()->onActPageScaleApplied(cfg, newcfg, true, page);
}

void PageContext::vShrink()
{
    auto& cfg = page->configRef();
    Config newcfg(cfg);
    newcfg.seconds_per_pix_y += 1;
    if (cfg.pixels_per_km > 1) {
        newcfg.pixels_per_km -= 1;
    }

    mw->getViewCategory()->onActPageScaleApplied(cfg, newcfg, true, page);
}

void PageContext::actChangePage()
{
    auto page = SelectPageCombo::dialogGetPage(diagram.pages(), mw, tr("选择页面"),
        tr("请选择要切换到的运行图页面"));
    if (page) {
        mw->focusInPage(page);
    }
}

void PageContext::commitEditInfo(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo)
{
    page->swapBaseInfo(*newinfo);
    if (page->name() != newinfo->name()) {
        int i = mw->_diagram.getPageIndex(page);
        mw->diagramDocks.at(i)->setWindowTitle(page->name());
        emit pageNameChanged(i);
    }
        
    refreshData();
}

void PageContext::onEditApplied(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo)
{
    mw->getUndoStack()->push(new qecmd::EditPageInfo(page, newinfo, this));
}

void PageContext::actRemovePage()
{
    emit pageRemoved(diagram.getPageIndex(page));
}

EditPageDialog::EditPageDialog(std::shared_ptr<DiagramPage> page_,Diagram& dia, QWidget* parent):
    QDialog(parent),page(page_),diagram(dia)
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void EditPageDialog::initUI()
{
    setWindowTitle(tr("运行图页面 - 基本信息"));
    resize(400, 500);

    auto* vlay = new QVBoxLayout;
    auto* form = new QFormLayout;
    edName = new QLineEdit;
    edName->setText(page->name());
    form->addRow(tr("页面名称"), edName);
    vlay->addLayout(form);
    vlay->addWidget(new QLabel(tr("备注：")));
    edNote = new QTextEdit;
    edNote->setPlainText(page->note());
    vlay->addWidget(edNote);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, SIGNAL(accepted()), this, SLOT(actApply()));
    connect(box, SIGNAL(rejected()), this, SLOT(close()));
    vlay->addWidget(box);
    setLayout(vlay);
}

void EditPageDialog::actApply()
{
    const QString& name = edName->text();
    if (!diagram.pageNameIsValid(name, page)) {
        QMessageBox::warning(this, tr("错误"), tr("运行图名称不能为空或与其他运行图名称冲突。"));
        return;
    }
    const QString& note = edNote->toPlainText();
    if (name != page->name() || note!=page->note()) {
        auto n = std::make_shared<DiagramPage>(page->config(),
            QList<std::shared_ptr<Railway>>{}, name, note);
        emit editApplied(page, n);
    }
    done(QDialog::Accepted);
}

qecmd::EditPageInfo::EditPageInfo(std::shared_ptr<DiagramPage> page_, 
    std::shared_ptr<DiagramPage> newpage_, PageContext* context_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("编辑运行图信息: ")+newpage_->name(),parent),
    page(page_),newpage(newpage_),cont(context_)
{
}

void qecmd::EditPageInfo::undo()
{
    cont->commitEditInfo(page, newpage);
}

void qecmd::EditPageInfo::redo()
{
    cont->commitEditInfo(page, newpage);
}

qecmd::ResetPage::ResetPage(std::shared_ptr<DiagramPage> page, 
    std::shared_ptr<DiagramPage> newpage, PageContext* context, QUndoCommand* parent):
    QUndoCommand(QObject::tr("修改运行图: %1").arg(newpage->name()),parent),
    page(page),data(newpage),cont(context)
{
}

void qecmd::ResetPage::undo()
{
    cont->commitResetPage(page, data);
}

void qecmd::ResetPage::redo()
{
    cont->commitResetPage(page, data);
}
#endif
