#include "routingwidget.h"

#include <QtWidgets>
#include "data/train/traincollection.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"


RoutingWidget::RoutingWidget(TrainCollection &coll_,QUndoStack* undo, QWidget *parent):
    QWidget(parent),coll(coll_),_undo(undo), model(new RoutingCollectionModel(coll_,this))
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

    table=new QTableView;
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
    for (int w: {120,200,100,80,80})
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
    auto r = std::make_shared<Routing>();
    r->setName(coll.validRoutingName(tr("新交路")));
    if (_undo) {
        _undo->push(new qecmd::AddRouting(r, this));
    }
    else {
        qDebug() << "RoutingWidget::actAdd: commit inplace." << Qt::endl;
        commitAddRouting(r);
    }
}

void RoutingWidget::actRemove()
{
    auto&& idx = table->currentIndex();
    if (!idx.isValid())return;
    auto r = coll.routingAt(idx.row());
    if (_undo) {
        _undo->push(new qecmd::RemoveRouting(idx.row(), r, this));
    }
    else {
        qDebug() << "RoutingWidget::actRemove: commit inplace." << Qt::endl;
        commitRemoveRouting(idx.row(), r);
    }
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

void RoutingWidget::undoAddRouting(std::shared_ptr<Routing> routing)
{
    model->removeRoutingAt(coll.routings().size() - 1);
    emit routingRemoved(routing);
}


void RoutingWidget::commitRemoveRouting(int row, std::shared_ptr<Routing> routing)
{
    model->removeRoutingAt(row);
    emit routingRemoved(routing);
}

void RoutingWidget::undoRemoveRouting(int row, std::shared_ptr<Routing> routing)
{
    model->addRoutingAt(routing, row);
    if (routing->empty()) {
        emit editRouting(routing);
    }
    else if (routing->anyValidTrains()) {
        emit nonEmptyRoutingAdded(routing);
    }
}


void RoutingWidget::commitAddRouting(std::shared_ptr<Routing> routing)
{
    model->addRoutingAt(routing, coll.routings().size());
    if (routing->empty()) {
        emit editRouting(routing);
    }
    else if (routing->anyValidTrains()) {
        emit nonEmptyRoutingAdded(routing);
    }
}

qecmd::AddRouting::AddRouting(std::shared_ptr<Routing> routing_, 
    RoutingWidget* const rw_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("添加交路: %1").arg(routing_->name()),parent),
    routing(routing_),rw(rw_)
{
}

void qecmd::AddRouting::undo()
{
    rw->undoAddRouting(routing);
}

void qecmd::AddRouting::redo()
{
    rw->commitAddRouting(routing);
}

qecmd::RemoveRouting::RemoveRouting(int row_, std::shared_ptr<Routing> routing_,
    RoutingWidget* const rw_, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("删除交路: %1").arg(routing_->name()), parent), row(row_),
    routing(routing_), rw(rw_)
{
}

void qecmd::RemoveRouting::undo()
{
    rw->undoRemoveRouting(row, routing);
}

void qecmd::RemoveRouting::redo()
{
    rw->commitRemoveRouting(row, routing);
}
