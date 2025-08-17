#include "adjacentlistwidget.h"
#include "edgedatamodels.h"
#include "model/delegate/qedelegate.h"
#include "adjacentlistmodel.h"

#include "QLabel"
#include "QTableView"
#include "QVBoxLayout"
#include "QHeaderView"
#include "data/common/qesystem.h"
#include "model/delegate/generaldoublespindelegate.h"
#include "model/delegate/traintimedelegate.h"
#include "model/delegate/timeintervaldelegate.h"
#include "railnet/raildb/default_options.h"


AdjacentListWidget::AdjacentListWidget(const RailNet &net, QWidget *parent):
    QWidget(parent),mdIn(new AdjacentListModel(net,this)),
    mdOut(new AdjacentListModel(net,this)),
    mdRuler(new RulerNodesModel(this)),
    mdForbid(new ForbidNodesModel(defaultDiagramOptionsForDB, this))
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
    tbOut->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    tbOut->setEditTriggers(QTableView::NoEditTriggers);
    tbOut->setModel(mdOut);
    connect(tbOut->selectionModel(),&QItemSelectionModel::currentRowChanged,
            this,&AdjacentListWidget::onOutRowChanged);
    vlay->addWidget(tbOut,2);

    tbIn=new QTableView;
    tbIn->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    tbIn->setEditTriggers(QTableView::NoEditTriggers);
    tbIn->setModel(mdIn);
    connect(tbIn->selectionModel(),&QItemSelectionModel::currentRowChanged,
            this,&AdjacentListWidget::onInRowChanged);
    vlay->addWidget(new QLabel(tr("逆邻接表 （入边表）: ")));
    vlay->addWidget(tbIn,2);

    tbRuler=new QTableView;
    tbRuler->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
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
    tbForbid->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    tbForbid->setEditTriggers(QTableView::NoEditTriggers);
    tbForbid->setModel(mdForbid);
    auto* dele2=new TrainTimeDelegate(defaultDiagramOptionsForDB, this, TrainTime::HM);
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
