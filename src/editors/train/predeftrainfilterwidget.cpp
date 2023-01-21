#include "predeftrainfilterwidget.h"
#include "trainfilterbasicwidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>

#include "data/train/predeftrainfiltercore.h"
#include "util/buttongroup.hpp"
#include "data/train/traincollection.h"

PredefTrainFilterWidget::PredefTrainFilterWidget(TrainCollection &coll, QWidget *parent)
    : QWidget{parent}, coll(coll)
{
    initUI();
}

void PredefTrainFilterWidget::setCore(PredefTrainFilterCore* core)
{
    this->core = core;
    basic->setCoreSimple(core);   // to be refreshed below
    refreshData();
}

void PredefTrainFilterWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    edName=new QLineEdit;

    flay->addRow(tr("预设名称"), edName);
    vlay->addLayout(flay);

    basic=new TrainFilterBasicWidget(coll,core);
    basic->layout()->setContentsMargins(0,0,0,0);
    vlay->addWidget(basic);

    edNote=new QTextEdit;
    vlay->addWidget(new QLabel(tr("备注:")));
    vlay->addWidget(edNote);

    auto* g=new ButtonGroup<3>({"确定","刷新","清空"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(actApply()),SLOT(refreshData()),SLOT(actClear())});
}

void PredefTrainFilterWidget::refreshData()
{
    if (!core){
        clearNotChecked();
        return;
    }
    basic->refreshData();
    edName->setText(core->name());
    edNote->setText(core->note());
}

void PredefTrainFilterWidget::actApply()
{
    // first check name, if not valid, do nothing.
    if (!coll.filterNameIsValid(edName->text(), core)) {
        QMessageBox::warning(this, tr("错误"), tr("非法列车筛选器名称：列车筛选器名称应非空且不互相重复。"));
        return;
    }

    std::unique_ptr<PredefTrainFilterCore> data(new PredefTrainFilterCore);
    data->setName(edName->text());
    data->setNote(edNote->toPlainText());

    basic->appliedData(data.get());
    emit changeApplied(core, data);
}

void PredefTrainFilterWidget::actClear()
{
    bool flag=basic->clearFilter();
    if (flag){
        edNote->clear();
    }
}

void PredefTrainFilterWidget::clearNotChecked()
{
    basic->clearNotChecked();
    edName->clear();
    edNote->clear();
}
