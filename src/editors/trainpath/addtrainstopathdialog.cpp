#include "addtrainstopathdialog.h"
#include "data/common/qesystem.h"
#include "data/train/traincollection.h"
#include "data/trainpath/trainpath.h"
#include "model/train/trainlistreadmodel.h"
#include "util/buttongroup.hpp"

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

AddTrainsToPathDialog::AddTrainsToPathDialog(const DiagramOptions& ops, TrainCollection &coll, TrainPath *path, QWidget *parent):
    QDialog(parent), _ops(ops), coll(coll), path(path), model(new TrainListReadModel(ops, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
    refreshData();
}

void AddTrainsToPathDialog::refreshData()
{
    QList<std::shared_ptr<Train>> trains{};
    foreach (auto t, coll.trains()){
        if (!path->containsTrain(t)){
            trains.push_back(t);
        }
    }
    model->resetList(std::move(trains));
}

void AddTrainsToPathDialog::initUI()
{
    setWindowTitle(tr("添加列车到径路 - %1").arg(path->name()));
    resize(600, 600);

    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("请在下表选择要添加当前径路的列车，可多选。"));
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

    auto* g=new ButtonGroup<2>({"确定", "取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, {SLOT(accept()), SLOT(reject())});
}

void AddTrainsToPathDialog::accept()
{
    const auto& lst=table->selectionModel()->selectedRows();
    if (!lst.empty()){
        QList<std::shared_ptr<Train>> trains{};
        foreach (const auto& idx, lst){
            trains.push_back(model->trains().at(idx.row()));
        }
        emit trainsAdded(path, trains);
        QDialog::accept();
    }else{
        QMessageBox::warning(this, tr("警告"), tr("未选择任何列车"));
    }
}
