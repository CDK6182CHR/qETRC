#include "addpagedialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QLineEdit>
#include <QTableView>
#include <QTextEdit>

#include "navi/navitree.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"
#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"

#include <model/diagram/railtablemodel.h>

#if 0

AddPageDialogV1::AddPageDialogV1(Diagram &diagram_, QWidget *parent):
    QDialog(parent),diagram(diagram_),model(new RailTableModel(diagram, this)),
    table(new QTableView),editName(new QLineEdit),editPrev(new QLineEdit)
{
    setWindowTitle(tr("添加运行图视窗"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    initUI();
    resize(800, 600);
}

AddPageDialogV1::AddPageDialogV1(Diagram& diagram_, std::shared_ptr<DiagramPage> page_, QWidget* parent):
    QDialog(parent),diagram(diagram_),page(page_),
    model(new RailTableModel(diagram,this)),
    table(new QTableView),editName(new QLineEdit),editPrev(new QLineEdit)
{
    setWindowTitle(tr("修改运行图视窗 - %1").arg(page->name()));
    setAttribute(Qt::WA_DeleteOnClose, true);
    initUI();
    resize(800, 600);
    setData();
}

void AddPageDialogV1::initUI()
{
    auto* vlay = new QVBoxLayout;
    auto* form = new QFormLayout;
    form->addRow(tr("运行图名称"), editName);
    vlay->addLayout(form);
    vlay->addWidget(new QLabel(tr("请在下表中选择要添加到运行图的线路：")));

    table->setModel(model);
    table->resizeColumnsToContents();
    table->verticalHeader()->setDefaultSectionSize(diagram.config().table_row_height);
    table->setSelectionBehavior(QTableView::SelectRows);
    vlay->addWidget(table);

    form = new QFormLayout;
    editPrev->setFocusPolicy(Qt::NoFocus);
    form->addRow(tr("线路顺序预览"), editPrev);
    vlay->addLayout(form);

    vlay->addWidget(new QLabel(tr("备注：")));
    edNote = new QTextEdit;
    vlay->addWidget(edNote);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    vlay->addWidget(box);

    connect(box, SIGNAL(accepted()), this, SLOT(okClicked()));
    connect(box, SIGNAL(rejected()), this, SLOT(close()));
    //connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&)),
    //    this, SLOT(preview(const QItemSelection&)));
    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &AddPageDialogV1::preview);

    setLayout(vlay);
}

void AddPageDialogV1::setData()
{
    editName->setText(page->name());
    edNote->setText(page->note());
}

void AddPageDialogV1::preview(const QItemSelection& )
{
    auto* sel = table->selectionModel();
    auto lst = sel->selectedRows();
    auto p = lst.begin();
    if (lst.empty()) {
        editPrev->setText("");
        return;
    }
    QString res = diagram.railwayAt(p->row())->name();
    for (++p; p != lst.end(); ++p) {
        res += " - " + diagram.railwayAt(p->row())->name();
    }
    editPrev->setText(res);
}

void AddPageDialogV1::okClicked()
{
    const QString& name = editName->text();
    if (name.isEmpty() || diagram.pageNameExisted(name, page)) {
        QMessageBox::warning(this, tr("错误"), tr("运行图名称为空或已存在，请重新设置"));
        return;
    }

    auto* sel = table->selectionModel();
    auto lst = sel->selectedRows();
    if (lst.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请至少选择一条线路!"));
        return;
    }

    QList<std::shared_ptr<Railway>> rails;
    for (const auto& p : lst) {
        rails.append(diagram.railways().at(p.row()));
    }

    auto npage = std::make_shared<DiagramPage>(diagram.config(), rails, name, edNote->toPlainText());
    if (page) {
        emit modificationDone(page, npage);
    }
    else {
        emit creationDone(npage);
    }
    done(QDialog::Accepted);
}

#endif

AddPageDialog::AddPageDialog(Diagram& diagram_, QWidget* parent) :
    QDialog(parent),diagram(diagram_),
    mdUnsel(new RailListModel(diagram_.railways(),this)),
    mdSel(new RailListModel({},this))
{
    setWindowTitle(tr("添加运行图视窗"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    initUI();
    resize(800, 600);
}

AddPageDialog::AddPageDialog(Diagram& diagram_, std::shared_ptr<DiagramPage> page_, QWidget* parent):
    QDialog(parent), diagram(diagram_),
    mdUnsel(new RailListModel(diagram_.railways(), this)),
    mdSel(new RailListModel({}, this)),page(page_)
{
    setWindowTitle(tr("修改运行图视窗 - %1").arg(page->name()));
    setAttribute(Qt::WA_DeleteOnClose, true);
    initUI();
    resize(800, 600);
    setData();
}

void AddPageDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* flay = new QFormLayout;
    edName = new QLineEdit;
    flay->addRow(tr("运行图页面名称"), edName);
    vlay->addLayout(flay);

    auto* h = new ButtonGroup<4>({ "全选","清空","上移","下移" });
    h->connectAll(SIGNAL(clicked()), this, { SLOT(selectAll()),SLOT(deselectAll()),
        SLOT(moveUp()),SLOT(moveDown()) });
    vlay->addLayout(h);

    auto* hlay = new QHBoxLayout;
    tbUnsel = new QTableView;
    tbUnsel->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbUnsel->setModel(mdUnsel);
    tbUnsel->setEditTriggers(QTableView::NoEditTriggers);
    tbUnsel->setSelectionBehavior(QTableView::SelectRows);
    tbUnsel->setSelectionMode(QTableView::ContiguousSelection);
    int c = 0;
    for (int w : {120, 80, 80, 60, 60}) {
        tbUnsel->setColumnWidth(c++, w);
    }
    hlay->addWidget(tbUnsel);

    auto* btns = new ButtonGroup<2, QVBoxLayout>({ ">","<" });
    btns->connectAll(SIGNAL(clicked()), this, { SLOT(select()),SLOT(deselect()) });
    btns->setMaximumWidth(30);
    hlay->addLayout(btns);

    tbSel = new QTableView;
    tbSel->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbSel->setModel(mdSel);
    tbSel->setEditTriggers(QTableView::NoEditTriggers);
    tbSel->setSelectionBehavior(QTableView::SelectRows);
    tbSel->setSelectionMode(QTableView::ContiguousSelection);

    c = 0;
    for (int w : {120,80,80,60,60}) {
        tbSel->setColumnWidth(c++, w);
    }

    hlay->addWidget(tbSel);
    vlay->addLayout(hlay);

    vlay->addWidget(new QLabel("备注: "));
    edNote = new QTextEdit;
    edNote->setMaximumHeight(60);
    vlay->addWidget(edNote);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    vlay->addWidget(box);

    connect(box, &QDialogButtonBox::accepted, this, &AddPageDialog::okClicked);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::close);
}

void AddPageDialog::setData()
{
    QList<std::shared_ptr<Railway>> lst;
    foreach(auto p, diagram.railways()) {
        if (! page->containsRailway(p)) {
            lst.append(p);
        }
    }
    mdUnsel->setRailways(lst);
    mdSel->setRailways(page->railways());

    edName->setText(page->name());
    edNote->setText(page->note());
}

void AddPageDialog::okClicked()
{
    const QString& name = edName->text();
    if (name.isEmpty() || diagram.pageNameExisted(name, page)) {
        QMessageBox::warning(this, tr("错误"), tr("运行图名称为空或已存在，请重新设置"));
        return;
    }

    const auto& rails = mdSel->railways();

    if (rails.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请至少选择一条线路!"));
        return;
    }

    auto npage = std::make_shared<DiagramPage>(diagram.config(), rails, name, edNote->toPlainText());
    if (page) {
        emit modificationDone(page, npage);
    }
    else {
        emit creationDone(npage);
    }
    done(QDialog::Accepted);
}

void AddPageDialog::selectAll()
{
    mdSel->append(mdUnsel->railways());
    mdUnsel->clear();
}

void AddPageDialog::deselectAll()
{
    mdUnsel->append(mdSel->railways());
    mdSel->clear();
}

void AddPageDialog::moveUp()
{
    auto sel = tbSel->selectionModel()->selectedRows();
    std::sort(sel.begin(), sel.end(), qeutil::ltIndexRow);
    tbSel->clearSelection();
    foreach(const auto & p, sel) {
        if (p.row() > 0) {
            mdSel->moveUp(p.row());
            //tbSel->selectionModel()->select(mdSel->index(p.row()-1,0), QItemSelectionModel::Select);
        }
    }
}

void AddPageDialog::moveDown()
{
    auto sel = tbSel->selectionModel()->selectedRows();
    std::sort(sel.begin(), sel.end(), qeutil::ltIndexRow);
    tbSel->clearSelection();

    for (auto p = sel.crbegin(); p != sel.crend(); ++p) {
        if (p->row() >= 0 && p->row() < mdSel->rowCount({}) - 1) {
            mdSel->moveDown(p->row());
        }
    }
}

void AddPageDialog::select()
{
    auto sel = tbUnsel->selectionModel()->selectedRows();
    std::sort(sel.begin(), sel.end(), qeutil::ltIndexRow);
    for (auto p = sel.crbegin(); p != sel.crend(); ++p) {
        mdSel->append(mdUnsel->takeAt(p->row()));
    }
}

void AddPageDialog::deselect()
{
    auto sel = tbSel->selectionModel()->selectedRows();
    std::sort(sel.begin(), sel.end(), qeutil::ltIndexRow);
    for (auto p = sel.crbegin(); p != sel.crend(); ++p) {
        mdUnsel->append(mdSel->takeAt(p->row()));
    }
}



qecmd::AddPage::AddPage(Diagram& diagram_, 
    std::shared_ptr<DiagramPage> page_, 
    NaviTree* nv, QUndoCommand* parent):
    QUndoCommand(QObject::tr("添加运行图 ")+page_->name(),parent),
    diagram(diagram_),page(page_),navi(nv)
{
}

void qecmd::AddPage::undo()
{
    navi->undoAddPage();
}

void qecmd::AddPage::redo()
{
    navi->commitAddPage(page);
}

qecmd::RemovePage::RemovePage(Diagram& diagram_, int index_,
    NaviTree* navi_, QUndoCommand* parent):
    QUndoCommand(parent),
    diagram(diagram_),page(diagram_.pages().at(index_)),navi(navi_),index(index_)
{
    setText(QObject::tr("删除运行图: ") + page->name());
}

void qecmd::RemovePage::undo()
{
    navi->undoRemovePage(page, index);
}

void qecmd::RemovePage::redo()
{
    navi->commitRemovePage(index);
}
