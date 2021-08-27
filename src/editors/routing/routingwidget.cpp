#include "routingwidget.h"

#include <QtWidgets>
#include "data/train/traincollection.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"


RoutingWidget::RoutingWidget(TrainCollection &coll_, QWidget *parent):
    QWidget(parent),coll(coll_),model(new RoutingCollectionModel(coll_,this))
{
    initUI();
    refreshData();
}

void RoutingWidget::refreshData()
{
    model->refreshData();
}

void RoutingWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    auto* table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setModel(model);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setSelectionMode(QTableView::SingleSelection);
    connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged,
        this, &RoutingWidget::onCurrentRowChanged);

    connect(table, &QTableView::doubleClicked, this, &RoutingWidget::actDoubleClicked);

    // 列宽 手动指定
    int c=0;
    for (int w: {120,200,80,80,80})
        table->setColumnWidth(c++,w);
    vlay->addWidget(table);

    auto* g=new ButtonGroup<3>({"编辑(&E)","添加(&N)","删除(&D)"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actEdit()),SLOT(actAdd()),SLOT(actRemove())});
    setLayout(vlay);
}

void RoutingWidget::actEdit()
{
    actDoubleClicked(table->currentIndex());
}

void RoutingWidget::actAdd()
{
    // todo ..
}

void RoutingWidget::actRemove()
{
    // todo ..
}


void RoutingWidget::actDoubleClicked(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;
    emit editRouting(coll.routingAt(idx.row()));
}

void RoutingWidget::onCurrentRowChanged(const QModelIndex& idx)
{
    if (!idx.isValid())return;
    emit focusInRouting(coll.routingAt(idx.row()));
}
