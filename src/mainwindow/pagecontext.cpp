#include "pagecontext.h"
#include "mainwindow.h"

#include "dialogs/printdiagramdialog.h"
#include "data/diagram/diagrampage.h"
#include "editors/configdialog.h"
#include "viewcategory.h"
#include "util/selectrailwaycombo.h"
#include "navi/addpagedialog.h"

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
    auto* panel = page->addPannel(tr(""));

    //基本信息部分
    if constexpr (true) {
        auto* ed = new QLineEdit;
        edName = ed;
        ed->setFocusPolicy(Qt::NoFocus);
        ed->setAlignment(Qt::AlignCenter);
        ed->setFixedWidth(150);

        auto* w = new QWidget;
        auto* vlay = new QVBoxLayout;
        auto* label = new QLabel(tr("当前运行图"));
        label->setAlignment(Qt::AlignCenter);
        vlay->addWidget(label);

        vlay->addWidget(ed);

        w->setLayout(vlay);
        panel->addWidget(w, SARibbonPannelItem::Large);
    }

    panel = page->addPannel(tr("基本"));

    auto* act = new QAction(QIcon(":/icons/edit.png"), tr("编辑"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(actEdit()));

    auto* me = new SARibbonMenu(mw);
    me->addAction(tr("重设页面"), this, &PageContext::actResetPage);
    act->setMenu(me);

    auto* btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
  

    act = new QAction(QIcon(":/icons/pdf.png"), tr("导出"), this);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, SIGNAL(triggered()), this, SLOT(actPrint()));

    act = new QAction(QIcon(":/icons/config.png"), tr("显示设置"), this);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, &QAction::triggered, this, &PageContext::actConfig);
    me = new SARibbonMenu(mw);
    me->addAction(tr("将运行图显示设置应用到当前页面"), this,
        &PageContext::actUseDiagramConfig);
    act->setMenu(me);

    act->setToolTip(tr("显示设置\n设置当前运行图页面的显示参数，不影响其他运行图页面。"));

    panel = page->addPannel("导航");

    act = new QAction(qApp->style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("转到..."), 
        this);
    act->setToolTip(tr("转到运行图页面\n打开或者切换到当前运行图页面。"));
    connect(act, &QAction::triggered, this, &PageContext::actActivatePage);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);

    act = new QAction(QIcon(":/icons/rail.png"), tr("编辑线路"), this);
    act->setToolTip(tr("编辑线路\n编辑当前运行图页面中的线路数据"));
    connect(act, &QAction::triggered, this, &PageContext::actEditRailway);
    me = new SARibbonMenu(mw);
    me->addAction(tr("转到线路"), this, &PageContext::actSwitchToRailway);
    act->setMenu(me);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);

    panel = page->addPannel("");

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
        tr("删除"), this);
    act->setToolTip(tr("删除运行图\n删除当前运行图页面，"
        "但线路、车次信息不会被删除。"));
    panel->addLargeAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemovePage()));

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("关闭面板"), this);
    act->setToolTip(tr("关闭面板\n关闭当前的运行图页面上下文工具栏页面"));
    connect(act, &QAction::triggered, mw, &MainWindow::focusOutPage);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
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
    auto* dlg = new ConfigDialog(page->configRef(), page, mw);
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
        mw->diagramDocks.at(i)->setWindowTitle(tr("运行图 - ") + page->name());
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

void PageContext::commitEditInfo(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo)
{
    page->swapBaseInfo(*newinfo);
    if (page->name() != newinfo->name()) {
        int i = mw->_diagram.getPageIndex(page);
        mw->diagramDocks.at(i)->setWindowTitle(tr("运行图 - ") + page->name());
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
