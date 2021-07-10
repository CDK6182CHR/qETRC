#include "pagecontext.h"
#include "mainwindow.h"

#include "dialogs/printdiagramdialog.h"

#include <QtWidgets>

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
    if (page) {
        edName->setText(page->name());
    }
}

void PageContext::initUI()
{
    auto* page = cont->addCategoryPage(tr("管理"));
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
    auto* btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, SIGNAL(triggered()), this, SLOT(actEdit()));

    act = new QAction(QIcon(":/icons/pdf.png"), tr("导出"), this);
    btn = panel->addLargeAction(act);
    btn->setMinimumWidth(70);
    connect(act, SIGNAL(triggered()), this, SLOT(actPrint()));


    act = new QAction(QIcon(":/icons/close.png"), tr("删除"), this);
    panel->addLargeAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(actRemovePage()));
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
        auto n = std::make_shared<DiagramPage>(QList<std::shared_ptr<Railway>>{}, name, note);
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
