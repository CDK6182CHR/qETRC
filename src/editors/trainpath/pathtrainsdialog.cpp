#include "pathtrainsdialog.h"
#include "data/common/qesystem.h"
#include "data/trainpath/trainpath.h"
#include "model/train/trainlistreadmodel.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"

#include <algorithm>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>

PathTrainsDialog::PathTrainsDialog(TrainCollection &coll, TrainPath *path, QWidget *parent):
    QDialog(parent), coll(coll), path(path), model(new TrainListReadModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("径路列车表 - %1").arg(path->name()));
    initUI();
    refreshData();
}

void PathTrainsDialog::refreshData()
{
    auto trains_=path->trainsShared();
    QList<std::shared_ptr<Train>> trainsQ(trains_.begin(), trains_.end());
    model->resetList(std::move(trainsQ));
}

void PathTrainsDialog::initUI()
{
    resize(600, 600);
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("以下为包含径路 [%1] 的列车。").arg(path->name()));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setSelectionMode(QTableView::MultiSelection);

    {
        int c=0;
        for(int w:{100,100,100,80,40,80,80}){
            table->setColumnWidth(c++, w);
        }
    }
    vlay->addWidget(table);

    auto* g=new ButtonGroup<3>({"关闭", "删除选中", "新增.."});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this,
                  { SLOT(close()), SLOT(actRemove()), SIGNAL(actAdd())});
}

void PathTrainsDialog::actRemove()
{
    const auto& lst=table->selectionModel()->selectedRows();
    auto rows=qeutil::indexRows(lst);
    emit removeTrains(path, rows);

    // the dialog is in modal state, so just simply remove the rows are OK
    // the actual operations are done by emitting the signal above
    for(auto itr=rows.rbegin();itr!=rows.rend();++itr){
        model->removeTrainAt(*itr);
    }
}
