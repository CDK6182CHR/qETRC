#include "rulerwidget.h"
#include "util/buttongroup.hpp"
#include "model/delegate/postivespindelegate.h"
#include "data/diagram/diagram.h"

#include <QtWidgets>

RulerWidget::RulerWidget(std::shared_ptr<Ruler> ruler_,bool commitInPlace_, QWidget *parent) :
    QWidget(parent),ruler(ruler_),model(new RulerModel(ruler_,this)),commitInPlace(commitInPlace_)
{
    initUI();
    refreshBasicData();
}

void RulerWidget::refreshData()
{
    updating = true;
    if (ruler) {
        model->refreshData();
        edName->setText(ruler->name());
        ckDiff->setChecked(ruler->different());
    }
    updating = false;
}

void RulerWidget::refreshBasicData()
{
    updating = true;
    if (ruler) {
        edName->setText(ruler->name());
        ckDiff->setChecked(ruler->different());
    }
    updating = false;
}


//void RulerWidget::setRuler(std::shared_ptr<Ruler> ruler)
//{
//    this->ruler = ruler;
//    refreshData();
//}

bool RulerWidget::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate) {
        if(ruler)
            emit focusInRuler(ruler);
        return true;
    }
    return QWidget::event(e);
}

void RulerWidget::initUI()
{
    auto* vlay=new QVBoxLayout;
    auto* form=new QFormLayout;
    edName=new QLineEdit;
    form->addRow(tr("标尺名称"),edName);
    ckDiff=new QCheckBox;
    if(ruler)ckDiff->setChecked(ruler->different());
    form->addRow(tr("上下行分设"),ckDiff);
    vlay->addLayout(form);
    connect(ckDiff,SIGNAL(toggled(bool)),this,SLOT(onDiffChanged(bool)));

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnsToContents();
    table->setEditTriggers(QTableView::CurrentChanged);

    table->setItemDelegateForColumn(RulerModel::ColMinute,
        new PostiveSpinDelegate(1, this));
    table->setItemDelegateForColumn(RulerModel::ColSeconds,
        new PostiveSpinDelegate(10, this));
    table->setItemDelegateForColumn(RulerModel::ColStart,
        new PostiveSpinDelegate(10, this));
    table->setItemDelegateForColumn(RulerModel::ColStop,
        new PostiveSpinDelegate(10, this));

    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"确定","还原"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(actCancel())});

    setLayout(vlay);
}

void RulerWidget::actCancel()
{
    refreshData();
}

void RulerWidget::onDiffChanged(bool on)
{
    if (updating)
        return;
    if (!ruler)
        return;
    //立即生效  提交给rulerContext执行
    updating = true;
    if (!on && ruler->railway().isSplitted()) {
        QMessageBox::warning(this, tr("错误"),
            tr("本线路存在上下行分设车站，标尺必须设置为上下行分设。"));
        ckDiff->setChecked(true);
        updating = false;
        return;
    }
    auto flag = QMessageBox::question(this, tr("标尺编辑"),
        tr("更改标尺上下行分设：请注意此操作立即生效。虽然此操作本身支持撤销，"
            "但由此导致未提交的标尺数据变更将丢失且不能恢复。是否继续？"));
    if (flag == QMessageBox::Yes) {
        auto t = ruler->clone();
        t->getRuler(0)->setDifferent(!ruler->different());
        if (commitInPlace) {
            qDebug() << "RulerWidget::onDiffChanged: INFO: change applied in-place.";
            ruler->swap(*(t->getRuler(0)));
        }
        else {
            emit actChangeRulerData(ruler, t);
        }
        
    }
       

    updating = false;
}

void RulerWidget::actApply()
{
    const QString& name = edName->text();
    if (name != ruler->name()) {
        //更改名称！
        if (commitInPlace) {
            ruler->setName(name);
        }
        else {
            emit actChangeRulerName(ruler, name);
        }
    }


    auto nr = model->appliedRuler();
    if (commitInPlace) {
        qDebug() << "RulerWidget::actApply: INFO: change applied in-place.";
        ruler->swap(*(nr->getRuler(0)));
    }
    else {
        emit actChangeRulerData(ruler, nr);
    }
    
}
