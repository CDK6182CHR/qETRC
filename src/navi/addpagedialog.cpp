#include "addpagedialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QModelIndex>

#include "navi/navitree.h"

AddPageDialog::AddPageDialog(Diagram &diagram_, QWidget *parent):
    QDialog(parent),diagram(diagram_),model(new RailTableModel(diagram, this)),
    table(new QTableView),editName(new QLineEdit),editPrev(new QLineEdit)
{
    setWindowTitle(tr("添加运行图视窗"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    initUI();
    resize(800, 600);
}

AddPageDialog::AddPageDialog(Diagram& diagram_, std::shared_ptr<DiagramPage> page_, QWidget* parent):
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

void AddPageDialog::initUI()
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
        this, &AddPageDialog::preview);

    setLayout(vlay);
}

void AddPageDialog::setData()
{
    editName->setText(page->name());
    edNote->setText(page->note());
}

void AddPageDialog::preview(const QItemSelection& )
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

void AddPageDialog::okClicked()
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
    diagram(diagram_),page(diagram_.pages().at(index_)),index(index_), navi(navi_)
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
