#include "adjacentlistwidget.h"
#include "edgedatamodels.h"
#include "model/delegate/qedelegate.h"

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <data/common/qesystem.h>
#include <model/delegate/generaldoublespindelegate.h>
#include <model/delegate/qetimedelegate.h>
#include <model/delegate/timeintervaldelegate.h>

AdjacentListModel::AdjacentListModel(const RailNet &net, QObject *parent):
    QStandardItemModel(parent), net(net)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("线名"),tr("自"),tr("至"),tr("方向"),tr("里程"),
                               tr("标尺数")});
}

std::shared_ptr<RailNet::edge> AdjacentListModel::edgeFromRow(int row)
{
    return qvariant_cast<std::shared_ptr<RailNet::edge>>(
                             item(row,ColRailName)->data(qeutil::GraphEdgeRole));
}

void AdjacentListModel::setupForInAdj(std::shared_ptr<RailNet::vertex> v)
{
    ve=v;
    if(!ve){
        setRowCount(0);
        return;
    }
    setRowCount(v->in_degree());
    int row=0;
    for(auto e=v->in_edge;e;e=e->next_in){
        setupRow(e,row++);
    }
}

void AdjacentListModel::setupForOutAdj(std::shared_ptr<RailNet::vertex> v)
{
    ve=v;
    if(!ve){
        setRowCount(0);
        return;
    }
    setRowCount(v->out_degree());
    int row=0;
    for(auto e=v->out_edge;e;e=e->next_out){
        setupRow(e,row++);
    }
}

void AdjacentListModel::setupRow(std::shared_ptr<RailNet::edge> e, int row)
{
    using SI=QStandardItem;

    auto* it=new SI(e->data.railName);
    QVariant v;
    v.setValue(e);
    it->setData(v,qeutil::GraphEdgeRole);
    setItem(row,ColRailName,it);

    setItem(row,ColFrom,new SI(e->from.lock()->data.name.toSingleLiteral()));
    setItem(row,ColTo,new SI(e->to.lock()->data.name.toSingleLiteral()));
    setItem(row,ColDir,new SI(DirFunc::dirToString(e->data.dir)));

    it=new SI;
    it->setData(e->data.mile, Qt::EditRole);
    setItem(row,ColMile,it);
    it=new SI;
    it->setData(e->data.rulerNodes.size(),Qt::EditRole);
    setItem(row,ColRulerCount,it);
}


AdjacentListWidget::AdjacentListWidget(const RailNet &net, QWidget *parent):
    QWidget(parent),mdIn(new AdjacentListModel(net,this)),
    mdOut(new AdjacentListModel(net,this)),
    mdRuler(new RulerNodesModel(this)),
    mdForbid(new ForbidNodesModel(this))
{
    initUI();
    connect(this,&AdjacentListWidget::currentEdgeChanged,
            this,&AdjacentListWidget::onCurrentEdgeChanged);
}

void AdjacentListWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    vlay->addWidget(new QLabel(tr("邻接表 （出边表）: ")));
    tbOut=new QTableView;
    tbOut->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbOut->setEditTriggers(QTableView::NoEditTriggers);
    tbOut->setModel(mdOut);
    connect(tbOut->selectionModel(),&QItemSelectionModel::currentRowChanged,
            this,&AdjacentListWidget::onOutRowChanged);
    vlay->addWidget(tbOut,2);

    tbIn=new QTableView;
    tbIn->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbIn->setEditTriggers(QTableView::NoEditTriggers);
    tbIn->setModel(mdIn);
    connect(tbIn->selectionModel(),&QItemSelectionModel::currentRowChanged,
            this,&AdjacentListWidget::onInRowChanged);
    vlay->addWidget(new QLabel(tr("逆邻接表 （入边表）: ")));
    vlay->addWidget(tbIn,2);

    tbRuler=new QTableView;
    tbRuler->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbRuler->setEditTriggers(QTableView::NoEditTriggers);
    tbRuler->setModel(mdRuler);
    tbRuler->setItemDelegateForColumn(RulerNodesModel::ColSpeed,
                                      new GeneralDoubleSpinDelegate(this));
    auto* dele=new TimeIntervalDelegate(this);
    tbRuler->setItemDelegateForColumn(RulerNodesModel::ColPass,dele);
    tbRuler->setItemDelegateForColumn(RulerNodesModel::ColStart,dele);
    tbRuler->setItemDelegateForColumn(RulerNodesModel::ColStop,dele);

    vlay->addWidget(new QLabel(tr("当前边标尺表：")));
    vlay->addWidget(tbRuler,1);

    tbForbid=new QTableView;
    tbForbid->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbForbid->setEditTriggers(QTableView::NoEditTriggers);
    tbForbid->setModel(mdForbid);
    auto* dele2=new QETimeDelegate(this,"hh:mm");
    tbForbid->setItemDelegateForColumn(ForbidNodesModel::ColBegin,dele2);
    tbForbid->setItemDelegateForColumn(ForbidNodesModel::ColEnd,dele2);
    vlay->addWidget(new QLabel(tr("当前边天窗表：")));
    vlay->addWidget(tbForbid,1);
}

void AdjacentListWidget::setVertex(std::shared_ptr<RailNet::vertex> ve)
{
    mdIn->setupForInAdj(ve);
    mdOut->setupForOutAdj(ve);
    tbIn->resizeColumnsToContents();
    tbOut->resizeColumnsToContents();
}

void AdjacentListWidget::onInRowChanged(const QModelIndex &idx)
{
    if(!idx.isValid()) return;
    emit currentEdgeChanged(mdIn->edgeFromRow(idx.row()));
}

void AdjacentListWidget::onOutRowChanged(const QModelIndex &idx)
{
    if(!idx.isValid()) return;
    emit currentEdgeChanged(mdOut->edgeFromRow(idx.row()));
}

void AdjacentListWidget::onCurrentEdgeChanged(std::shared_ptr<RailNet::edge> ed)
{
    mdRuler->setupModel(ed);
    mdForbid->setupModel(ed);

    tbRuler->resizeColumnsToContents();
    tbForbid->resizeColumnsToContents();
}
