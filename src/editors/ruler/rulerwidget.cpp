﻿#include "rulerwidget.h"
#include "util/buttongroup.hpp"
#include "model/delegate/postivespindelegate.h"
#include "data/common/qesystem.h"
#include "data/rail/rulernode.h"
#include "data/rail/ruler.h"

#include <QFormLayout>
#include <QTableView>
#include <QHeaderView>
#include <QAction>
#include <QMessageBox>
#include <QLineEdit>
#include <QEvent>
#include <QInputDialog>

#include "model/rail/rulermodel.h"
#include "util/helper_templates.hpp"
#include "util/utilfunc.h"

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
    }
    updating = false;
}

void RulerWidget::refreshBasicData()
{
    updating = true;
    if (ruler) {
        edName->setText(ruler->name());
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
    vlay->addLayout(form);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    //table->resizeColumnsToContents();
    table->setEditTriggers(QTableView::AllEditTriggers);

    table->setItemDelegateForColumn(RulerModel::ColMinute,
        new PostiveSpinDelegate(1, this));
    table->setItemDelegateForColumn(RulerModel::ColSeconds,
        new PostiveSpinDelegate(10, this));
    table->setItemDelegateForColumn(RulerModel::ColStart,
        new PostiveSpinDelegate(10, this));
    table->setItemDelegateForColumn(RulerModel::ColStop,
        new PostiveSpinDelegate(10, this));

    //actions...
    auto* act = new QAction(tr("将下行数据复制为上行"), table);
    connect(act, SIGNAL(triggered()), this, SLOT(copyFromDownToUp()));
    table->addAction(act);
    act = new QAction(tr("将上行数据复制为下行"), table);
    connect(act, SIGNAL(triggered()), this, SLOT(copyFromUpToDown()));
    table->addAction(act);
    table->setContextMenuPolicy(Qt::ActionsContextMenu);

    qeutil::make_action_at(table, tr("批量设置起步附加"), this, &RulerWidget::batchSetStart);
    qeutil::make_action_at(table, tr("批量设置停车附加"), this, &RulerWidget::batchSetStop);

    int c = 0;
    for (int w : {120, 50, 50, 60, 60, 80, 80}) {
        table->setColumnWidth(c++, w);
    }


    vlay->addWidget(table);

    auto* g=new ButtonGroup<3>({"确定","还原","删除"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()),SLOT(actCancel()),
        SLOT(actRemove()) });

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
    if (!on && ruler->railway()->isSplitted()) {
        QMessageBox::warning(this, tr("错误"),
            tr("本线路存在上下行分设车站，标尺必须设置为上下行分设。"));
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

void RulerWidget::copyFromDownToUp()
{
    auto t = QMessageBox::question(this, tr("标尺编辑"),
        tr("将（表中当前显示的）下行每个区间的标尺数据复制到上行对应区间，"
            "如果存在上下行分设站，则有关的区间数据不变。是否确认？"));
    if (t == QMessageBox::Yes)
        model->copyFromDownToUp();
}

void RulerWidget::copyFromUpToDown()
{
    auto t = QMessageBox::question(this, tr("标尺编辑"),
        tr("将（表中当前显示的）上行每个区间的标尺数据复制到下行对应区间，"
            "如果存在上下行分设站，则有关的区间数据不变。是否确认？"));
    if (t == QMessageBox::Yes)
        model->copyFromUpToDown();
}

void RulerWidget::actRemove()
{
    if (ruler) {
        emit actRemoveRuler(ruler);
    }
}

void RulerWidget::batchSetStart()
{
    const auto& sel = table->selectionModel()->selectedIndexes();
    auto rows = qeutil::indexRows(sel);
    
    if (sel.isEmpty())return;
    bool ok;
    int res = QInputDialog::getInt(this,
        tr("批量设置起步附加"),
        tr("将所选%1行的起步附加时分全部设置为（秒）：").arg(rows.size()), 120, 0, 100000, 10, &ok);
    if (!ok)return;

    for (int row : rows) {
        model->item(row, RulerModel::ColStart)->setData(res, Qt::EditRole);
    }
}

void RulerWidget::batchSetStop()
{
    const auto& sel = table->selectionModel()->selectedIndexes();
    auto rows = qeutil::indexRows(sel);
    if (sel.isEmpty())return;
    bool ok;
    int res = QInputDialog::getInt(this,
        tr("批量设置停车附加"),
        tr("将所选%1行的停车附加时分全部设置为（秒）：").arg(rows.size()), 120, 0, 100000, 10, &ok);
    if (!ok)return;

    for(int row:rows) {
        model->item(row, RulerModel::ColStop)->setData(res, Qt::EditRole);
    }
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
