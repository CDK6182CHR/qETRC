#include "selectpathdialog.h"
#include "model/trainpath/pathlistreadmodel.h"

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <algorithm>

#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"

SelectPathDialog::SelectPathDialog(const QString &prompt, QWidget *parent):
    QDialog(parent), model(new PathListReadModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI(prompt);
}

std::vector<TrainPath *> SelectPathDialog::selectedPaths()
{
    std::vector<TrainPath*> res{};
    auto* m=table->selectionModel();
    const auto& lst = m->selectedRows();
    foreach(const auto& idx, lst){
        res.emplace_back(model->paths().at(idx.row()));
    }
    return res;
}

std::vector<int> SelectPathDialog::selectedIndexes()
{
    std::vector<int> res{  };
    auto* m = table->selectionModel();
    const auto& lst = m->selectedRows();
    foreach(const auto & idx, lst) {
        res.emplace_back(idx.row());
    }
    std::sort(res.begin(), res.end());
    return res;
}

std::vector<TrainPath *> SelectPathDialog::getPaths(QWidget *parent, const QString &prompt, std::vector<TrainPath *> paths, bool single)
{
    SelectPathDialog d{prompt, parent};
    d.setAttribute(Qt::WA_DeleteOnClose, false);
    d.getModel()->resetData(paths);
    if (single) {
        d.table->setSelectionMode(QTableView::SingleSelection);
    }

    auto res = d.exec();

    if (!res) return {};
    auto selpaths = d.selectedPaths();
    d.setParent(nullptr);
    return selpaths;
}

std::vector<int> SelectPathDialog::getPathIndexes(QWidget* parent, const QString& prompt, std::vector<TrainPath*> paths, 
    bool multi)
{
    SelectPathDialog d{ prompt, parent };
    if (!multi) {
        d.table->setSelectionMode(QTableView::SingleSelection);
    }
    d.setAttribute(Qt::WA_DeleteOnClose, false);
    d.getModel()->resetData(paths);
    auto res = d.exec();

    if (!res) return {};
    auto selpaths = d.selectedIndexes();
    d.setParent(nullptr);
    return selpaths;
}

void SelectPathDialog::initUI(const QString &prompt)
{
    setWindowTitle(tr("选择列车径路"));
    resize(400,500);

    auto* vlay=new QVBoxLayout(this);
    auto* label=new QLabel(prompt);
    label->setWordWrap(true);
    vlay->addWidget(label);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    table->setModel(model);
    {
        int c=0;
        for(int w: {120,40,80,80}){
            table->setColumnWidth(c++, w);
        }
    }
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setSelectionMode(QTableView::MultiSelection);

    vlay->addWidget(table);

    auto* g=new ButtonGroup<3>({"确定", "全选", "取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this,
                  {SLOT(accept()), SLOT(actSelectAll()), SLOT(reject())});
}

void SelectPathDialog::actSelectAll()
{
    auto* selmodel=table->selectionModel();
    selmodel->select(
        QItemSelection(model->index(0,0),
                       model->index(model->rowCount({})-1, model->columnCount({})-1)),
        QItemSelectionModel::Select);
}
