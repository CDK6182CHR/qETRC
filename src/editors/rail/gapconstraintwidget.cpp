#include "gapconstraintwidget.h"

#include <model/rail/gapconstraintmodel.h>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <data/common/qesystem.h>
#include <data/gapset/gapsetabstract.h>
#include <data/calculation/gapconstraints.h>
#include <util/buttongroup.hpp>

GapConstraintWidget::GapConstraintWidget(GapConstraints &constraints,QWidget *parent):
    QWidget(parent), _constraints(constraints),
    _model(new GapConstraintModel(this))
{
    resize(600,600);
    setWindowTitle(tr("列车间隔编辑"));
    initUI();
}

void GapConstraintWidget::refreshData()
{
    //_model->gapSet()->setSingleLine(_constraints.isSingleLine());
    _model->gapSet()->buildSet();
}

void GapConstraintWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    table=new QTableView;
    vlay->addWidget(table);

    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setModel(_model);

    auto* g=new ButtonGroup<2>({"确定","关闭"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(onApply()),SLOT(close())});
}

void GapConstraintWidget::onApply()
{
    _constraints.clear();
    auto* gs=_model->gapSet();
    for(const auto& gapgroup:*gs){
        for(const auto& t:*gapgroup){
            _constraints[t] = gapgroup->limit();
        }
    }

    for(auto t:gs->remainTypes()){
        _constraints[t]=0;
    }
}
