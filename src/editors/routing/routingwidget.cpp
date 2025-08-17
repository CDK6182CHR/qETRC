#include "routingwidget.h"


#include "data/train/traincollection.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "data/train/routing.h"

#include <QTableView>
#include <QHeaderView>

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
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
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

void RoutingWidget::removeRouting(int row, std::shared_ptr<Routing> routing)
{
    if (_undo) {
        _undo->push(new qecmd::RemoveRouting(row, routing, this));
    }
    else {
        qDebug() << "RoutingWidget::actRemove: commit inplace." << Qt::endl;
        commitRemoveRouting(row, routing);
    }
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

void RoutingWidget::onRemoveRoutingFromContext(std::shared_ptr<Routing> routing)
{
    int i = coll.getRoutingIndex(routing);
    if (i == -1)return;
    if (_undo) {
        _undo->push(new qecmd::RemoveRouting(i, routing, this));
    }
    else {
        qDebug() << "RoutingWidget::onRemoveRoutingFromContext: INFO: " <<
            "commit in-place. " << Qt::endl;
        commitRemoveRouting(i, routing);
    }
}

void RoutingWidget::onRemoveRoutingFromNavi(int row)
{
    auto t = coll.routingAt(row);
    removeRouting(row, t);
}

void RoutingWidget::onRoutingsAdded(const QList<std::shared_ptr<Routing>> routings)
{
    if (_undo) {
        _undo->push(new qecmd::BatchAddRoutings(routings, this));
    }
    else {
        qDebug() << "RoutingWidget::onRoutingsAdded: INFO: commit in-place" << Qt::endl;
        commitAppendRoutings(routings);
    }
}

void RoutingWidget::commitAppendRoutings(const QList<std::shared_ptr<Routing>> routings)
{
    model->appendRoutings(routings);
    //暂定一个个发射信号
    foreach(auto p, routings) {
        if (p->anyValidTrains())
            emit nonEmptyRoutingAdded(p);
    }
}

void RoutingWidget::undoAppendRoutings(const QList<std::shared_ptr<Routing>> routings)
{
    model->removeTailRoutings(routings.size());
    foreach(auto p, routings) {
        emit routingRemoved(p);
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

qecmd::BatchAddRoutings::BatchAddRoutings(const QList<std::shared_ptr<Routing>> routings_, 
    RoutingWidget* rw_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("批量添加%1个交路").arg(routings_.size()),parent),
    routings(routings_),rw(rw_)
{
}

void qecmd::BatchAddRoutings::undo()
{
    rw->undoAppendRoutings(routings);
}

void qecmd::BatchAddRoutings::redo()
{
    rw->commitAppendRoutings(routings);
}
