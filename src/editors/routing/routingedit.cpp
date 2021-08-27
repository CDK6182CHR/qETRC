#include "routingedit.h"

#include <QtWidgets>
#include <set>

#include "data/train/routing.h"
#include "util/buttongroup.hpp"
#include "data/common/qesystem.h"
#include "data/train/traincollection.h"

RoutingEdit::RoutingEdit(TrainCollection& coll_, std::shared_ptr<Routing> routing_, QWidget *parent) :
    QWidget(parent),coll(coll_), routing(routing_),model(new RoutingEditModel(routing,this))
{
    initUI();
    refreshData();
}

void RoutingEdit::refreshData()
{
    if(!routing)return;
    refreshBasicData();
    model->refreshData();
}

void RoutingEdit::refreshBasicData()
{
    edName->setText(routing->name());
    edModel->setText(routing->model());
    edOwner->setText(routing->owner());
    edNote->setText(routing->note());
}

bool RoutingEdit::event(QEvent* e)
{
    if (e->type() == QEvent::WindowActivate && routing) {
        emit focusInRouting(routing);
        return true;
    }
    return QWidget::event(e);
}

void RoutingEdit::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;
    edName=new QLineEdit;
    flay->addRow(tr("交路名称"),edName);
    edModel=new QLineEdit;
    flay->addRow(tr("车底型号"),edModel);
    edOwner=new QLineEdit;
    flay->addRow(tr("担当局段"),edOwner);
    vlay->addLayout(flay);

    auto* g=new ButtonGroup<5>({"前插","后插","删除","上移","下移"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actAddBefore()),SLOT(actAddAfter()),
      SLOT(actRemove()),SLOT(actMoveUp()),SLOT(actMoveDown()) });

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    vlay->addWidget(table);
    connect(model, &RoutingEditModel::routingInserted,
        this, &RoutingEdit::rowInserted);

    // 列宽手动给定
    int r = 0;
    for (int i : {140, 40, 80, 80, 40}) {
        table->setColumnWidth(r++, i);
    }

    vlay->addWidget(new QLabel(tr("备注或说明")));
    edNote=new QTextEdit;
    edNote->setMaximumHeight(100);
    vlay->addWidget(edNote);

    auto* h=new ButtonGroup<3>({"确定","还原","关闭"});
    vlay->addLayout(h);
    h->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(actCancel()),
                                          SIGNAL(closeDock())});
}

void RoutingEdit::actAddBefore()
{
    insertAt(table->selectionModel()->currentIndex().row());
}

void RoutingEdit::actAddAfter()
{
    insertAt(table->selectionModel()->currentIndex().row()+1);
}

void RoutingEdit::insertAt(int row)
{
    if (row<0 || row > model->rowCount())
        return;
    if (!dlgAdd) {
        dlgAdd = new AddRoutingNodeDialog(coll, this);
        connect(dlgAdd, &AddRoutingNodeDialog::virtualTrainAdded,
            model, &RoutingEditModel::insertVirtualRow);
        connect(dlgAdd, &AddRoutingNodeDialog::realTrainAdded,
            model, &RoutingEditModel::insertRealRow);
    }
    dlgAdd->openForRow(row);
}

void RoutingEdit::actRemove()
{
    auto&& idx=table->selectionModel()->currentIndex();
    if(idx.isValid())
        model->removeRow(idx.row());
}

void RoutingEdit::actMoveUp()
{
    auto idx = table->currentIndex();
    if (idx.isValid()&&idx.row()!=0) {
        model->moveUp(idx.row());
        table->setCurrentIndex(model->index(idx.row() - 1, idx.column()));
    }
}

void RoutingEdit::actMoveDown()
{
    //下移这一行，等价于把它下面一行上移
    auto idx = table->currentIndex();
    if (idx.isValid() && idx.row() != model->rowCount() - 1) {
        model->moveDown(idx.row());
        table->setCurrentIndex(model->index(idx.row() + 1, idx.column()));
    }
}

void RoutingEdit::actParse()
{
    // todo..
}

void RoutingEdit::actDetect()
{
    // todo..
}


void RoutingEdit::actApply()
{
    // 先处理基本信息部分
    auto&& name = edName->text();
    if (name.isEmpty() || coll.routingNameExisted(name,routing)) {
        QMessageBox::warning(this, tr("错误"), tr("交路名称无效：交路名称不能为空或与既有重复"));
        return;
    }
    auto r = routing->copyBase();
    r->setName(edName->text());
    r->setModel(edModel->text());
    r->setOwner(edOwner->text());
    r->setNote(edNote->toPlainText());

    bool infoEquiv = routing->baseEqual(*r);
    bool flag = model->getAppliedOrder(r);
    if (!flag) {
        //设置有错，再见
        return;
    }
    bool orderEquiv = routing->orderEqual(*r);
    if (!orderEquiv) {
        emit routingOrderChanged(routing, r);
    }
    else if (!infoEquiv) {
        emit routingInfoChanged(routing, r);
    }
}

void RoutingEdit::actCancel()
{
    refreshData();
}

void RoutingEdit::rowInserted(int row)
{
    table->setCurrentIndex(model->index(row, 0));
}


