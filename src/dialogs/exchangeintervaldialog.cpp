#include "exchangeintervaldialog.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <algorithm>
#include <QMessageBox>

#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include <model/train/timetablestdmodel.h>
#include "data/train/train.h"
#include <util/selecttraincombo.h>

ExchangeIntervalDialog::ExchangeIntervalDialog(TrainCollection &coll_,
                       std::shared_ptr<Train> train1_, QWidget *parent):
    QDialog(parent),coll(coll_),train1(train1_),
    model1(new TimetableStdModel(false,this)),model2(new TimetableStdModel(false,this))
{
    setWindowTitle(tr("区间换线 - %1").arg(train1->trainName().full()));
    resize(800,800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void ExchangeIntervalDialog::initUI()
{
    auto* vlay=new QVBoxLayout;
    auto* label=new QLabel("请选择一个车次，然后选择要交换时刻的区间。"
        "当前车次和另一车次的表中选择的第一个站作为起始站，最后一个站作为终止站。"
        "交换的时刻包括起始站和终止站。"
        "你可以选择是否包含起始站的到达时刻和终止站的出发时刻。"
        "请注意，原则上只有同方向的两个车次、在两个交点或首末点之间才可以换线，"
        "但本系统不做限制，亦不对结果作保证。");
    label->setWordWrap(true);
    vlay->addWidget(label);

    auto* hlay=new QHBoxLayout;
    ckExStart=new QCheckBox(tr("交换起始站到达时刻"));
    ckExEnd=new QCheckBox(tr("交换终止站出发时刻"));
    hlay->addWidget(ckExStart);
    hlay->addWidget(ckExEnd);
    vlay->addLayout(hlay);

    auto* form=new QFormLayout;
    auto* ed=new QLineEdit;
    ed->setText(train1->trainName().full());
    ed->setFocusPolicy(Qt::NoFocus);
    form->addRow(tr("当前车次"),ed);

    select=new SelectTrainCombo(coll);
    connect(select,&SelectTrainCombo::currentTrainChanged,
            this,&ExchangeIntervalDialog::onTrain2Changed);
    form->addRow(tr("另一车次"),select);
    vlay->addLayout(form);

    auto* sp=new QSplitter;
    sp->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    table1=new QTableView;
    table1->setEditTriggers(QTableView::NoEditTriggers);
    table1->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table1->setSelectionBehavior(QTableView::SelectRows);
    table1->setSelectionMode(QTableView::ContiguousSelection);
    model1->setTrain(train1);
    table1->setModel(model1);
    table1->resizeColumnsToContents();
    sp->addWidget(table1);

    table2=new QTableView;
    table2->setEditTriggers(QTableView::NoEditTriggers);
    table2->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table2->setSelectionBehavior(QTableView::SelectRows);
    table2->setSelectionMode(QTableView::ContiguousSelection);
    table2->setModel(model2);
    table2->resizeColumnsToContents();
    sp->addWidget(table2);

    vlay->addWidget(sp);

    auto* g=new ButtonGroup<2>({"确定","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(close())});
    setLayout(vlay);
}

void ExchangeIntervalDialog::onTrain2Changed(std::shared_ptr<Train> t2)
{
    train2=t2;
    model2->setTrain(t2);
    table2->resizeColumnsToContents();
}

void ExchangeIntervalDialog::actApply()
{
    if (!train2) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择另一车次！"));
        return;
    }
    auto sel1 = table1->selectionModel()->selectedRows();
    auto sel2 = table2->selectionModel()->selectedRows();
    if (sel1.isEmpty() || sel2.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请选择要交换运行线的区间！"));
        return;
    }
    auto rowlt = [](const QModelIndex& idx1,const QModelIndex& idx2)->bool {
        return idx1.row() < idx2.row();
    };

    int start1 = std::min_element(sel1.begin(), sel1.end(), rowlt)->row();
    int end1 = std::max_element(sel1.begin(), sel1.end(), rowlt)->row();
    int start2 = std::min_element(sel2.begin(), sel2.end(), rowlt)->row();
    int end2 = std::max_element(sel2.begin(), sel2.end(), rowlt)->row();

    //注意这些迭代器算法是线性复杂度
    auto ps1 = train1->timetable().begin(); std::advance(ps1, start1);
    auto pe1 = train1->timetable().begin(); std::advance(pe1, end1);
    auto ps2 = train2->timetable().begin(); std::advance(ps2, start2);
    auto pe2 = train2->timetable().begin(); std::advance(pe2, end2);

    emit exchangeApplied(train1, train2, ps1, pe1, ps2, pe2,
        ckExStart->isChecked(), ckExEnd->isChecked());
    done(QDialog::Accepted);
}
