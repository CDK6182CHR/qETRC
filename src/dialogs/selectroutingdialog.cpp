#include "selectroutingdialog.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include "data/common/qesystem.h"
#include "data/train/traincollection.h"
#include "model/train/routingcollectionmodel.h"


SelectRoutingDialog::SelectRoutingDialog(TrainCollection &coll, bool allowNew_, QWidget *parent):
    QDialog(parent), coll(coll), allowNew(allowNew_), model(new RoutingCollectionModel(coll, this))
{
    setWindowTitle(tr("选择交路"));
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
    resize(700, 500);
}

typename SelectRoutingDialog::SelectReturned
SelectRoutingDialog::selectRouting(TrainCollection &coll, bool allowNew, QWidget *parent)
{
    SelectRoutingDialog dlg(coll, allowNew, parent);
    dlg.setAttribute(Qt::WA_DeleteOnClose, false);

    auto flag = dlg.exec();
    if (flag){
        if (dlg.isNewSelected){
            // C++20 aggregation initialization
            return {.isAccepted=true, .createNew=true};
        }else{
            return {.isAccepted=true, .createNew=false, .routing=dlg.currentRouting()};
        }
    }else{
        return {.isAccepted=false};
    }
}

void SelectRoutingDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel;
    if (allowNew){
        lab->setText(tr("请在下表选择交路，或新建交路："));
    }else{
        lab->setText(tr("请在下表选择交路："));
    }
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setSelectionMode(QTableView::SingleSelection);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setModel(model);
    {
        int c = 0;
        for (int w : {80, 200, 80, 60, 60}) {
            table->setColumnWidth(c++, w);
        }
    }

    vlay->addWidget(table);

    auto* hlay=new QHBoxLayout;
    auto* btn=new QPushButton(tr("确定"));
    connect(btn, &QPushButton::clicked, this, &SelectRoutingDialog::actApply);
    hlay->addWidget(btn);

    if (allowNew){
        btn=new QPushButton(tr("新建"));
        connect(btn, &QPushButton::clicked, this, &SelectRoutingDialog::actNew);
        hlay->addWidget(btn);
    }

    btn=new QPushButton(tr("取消"));
    connect(btn, &QPushButton::clicked, this, &QDialog::reject);
    hlay->addWidget(btn);

    vlay->addLayout(hlay);
}

std::shared_ptr<Routing> SelectRoutingDialog::currentRouting()
{
    const auto& cur=table->currentIndex();
    if (cur.isValid() && 0 <= cur.row() && cur.row() < coll.routingCount()){
        return coll.routingAt(cur.row());
    }else return {};
}

void SelectRoutingDialog::actApply()
{
    auto r=currentRouting();
    if (r) {
        emit routingSelected(r);
        accept();
    }else{
        QMessageBox::warning(this, tr("错误"), tr("没有选择交路！"));
    }
}

void SelectRoutingDialog::actNew()
{
    isNewSelected=true;
    emit createNewRouting();
    accept();
}
