#include "predeftrainfilterwidget.h"
#include "trainfilterbasicwidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QToolButton>
#include <QApplication>
#include <QStyle>

#include "data/train/predeftrainfiltercore.h"
#include "util/buttongroup.hpp"
#include "data/train/traincollection.h"
#include "defines/icon_specs.h"

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
    auto* hlay=new QHBoxLayout;

    edName=new QLineEdit;
    hlay->addWidget(edName);
    auto* btn=new QToolButton;
    btn->setIcon(QEICN_train_filter_info_widget);
    hlay->addWidget(btn);
    connect(btn,&QToolButton::clicked,this,
            &PredefTrainFilterWidget::informPredef);

    flay->addRow(tr("预设名称"), hlay);
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
        setEnabled(false);
        return;
    }
    setEnabled(true);
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

void PredefTrainFilterWidget::informPredef()
{
    QMessageBox::information(this,tr("提示"),
                             tr("自1.3.0版本起，本系统支持保存列车筛选器预设。在需要使用列车筛选器"
                                "的功能场景，可以直接使用预设，或者从已有预设编辑。"
                                "此处编辑的预设将保存到当前运行图文件。"));

}
