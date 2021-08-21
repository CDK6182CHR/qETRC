#include "forbidwidget.h"

#include <QtWidgets>
#include "model/delegate/qetimedelegate.h"
#include "data/diagram/diagram.h"
#include "util/buttongroup.hpp"

ForbidWidget::ForbidWidget(std::shared_ptr<Forbid> forbid_,bool commitInPlace, QWidget *parent):
    QWidget(parent),forbid(forbid_),model(new ForbidModel(forbid,this)),inplace(commitInPlace)
{
    initUI();
    refreshBasicData();
}

void ForbidWidget::refreshBasicData()
{
    updating=true;
    ckDown->setChecked(forbid->isDownShow());
    ckUp->setChecked(forbid->isUpShow());
    updating=false;
}

void ForbidWidget::refreshData()
{
    refreshBasicData();
    model->refreshData();
}

void ForbidWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* hlay=new QHBoxLayout;
    ckDown=new QCheckBox(tr("下行"));
    ckUp=new QCheckBox(tr("上行"));
    hlay->addWidget(ckDown);
    hlay->addWidget(ckUp);
    flay->addRow(tr("显示天窗"),hlay);
    //todo: connection of forbid show

    spLength=new QSpinBox;
    spLength->setRange(0,24*60);
    spLength->setSuffix(tr(" 分 (min)"));
    spLength->setSingleStep(10);
    spLength->setValue(120);
    flay->addRow(tr("默认时长"),spLength);
    vlay->addLayout(flay);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnsToContents();
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setItemDelegateForColumn(ForbidModel::ColStart,
                                    new QETimeDelegate(this,"hh:mm"));
    table->setItemDelegateForColumn(ForbidModel::ColEnd,
                                    new QETimeDelegate(this,"hh:mm"));
    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"确定","还原"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(onApply()),SLOT(refreshData())});
    vlay->addLayout(g);
}

void ForbidWidget::onApply()
{
    auto nr = model->appliedForbid();
    if (inplace) {
        qDebug() << "ForbidWidget::onApply: INFO: commit in place. " << Qt::endl;
        forbid->swap(*(nr->getForbid(0)));
    }
    else {
        emit forbidChanged(forbid, nr);
    }
}

ForbidTabWidget::ForbidTabWidget(std::shared_ptr<Railway> railway_, bool commitInPlace,
                                 QWidget *parent):
    QTabWidget(parent), railway(railway_), inplace(commitInPlace)
{
    for(int i=0;i<Forbid::FORBID_COUNT;i++){
        addForbidTab(railway->getForbid(i));
    }
}

void ForbidTabWidget::addForbidTab(std::shared_ptr<Forbid> forbid)
{
    auto* w=new ForbidWidget(forbid, inplace);
    addTab(w, forbid->name());
    connect(w, &ForbidWidget::forbidChanged,
            this, &ForbidTabWidget::forbidChanged);
}

void ForbidTabWidget::refreshData(std::shared_ptr<Forbid> forbid)
{
    static_cast<ForbidWidget*>(widget(forbid->index()))->refreshData();
}

void ForbidTabWidget::refreshBasicData(std::shared_ptr<Forbid> forbid)
{
    static_cast<ForbidWidget*>(widget(forbid->index()))->refreshBasicData();
}
