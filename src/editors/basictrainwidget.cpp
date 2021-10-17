#include "basictrainwidget.h"

#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "model/delegate/qetimedelegate.h"

#include "model/train/timetablestdmodel.h"
#include "util/qecontrolledtable.h"

#include <QTableView>
#include <QHeaderView>


BasicTrainWidget::BasicTrainWidget(TrainCollection &coll_, bool commitInPlace_,
    QWidget *parent) : QWidget(parent),
    coll(coll_),
    commitInPlace(commitInPlace_)
{
    initUI();
}

void BasicTrainWidget::setTrain(std::shared_ptr<Train> train)
{
    _train = train;
    model->setTrain(train);   //自带刷新操作
    refreshBasicData();
}

void BasicTrainWidget::refreshData()
{
    model->refreshData();
    refreshBasicData();
}

void BasicTrainWidget::refreshBasicData()
{
    //table->resizeColumnsToContents();
}

void BasicTrainWidget::initUI()
{
    model = new TimetableStdModel(commitInPlace, this);
    auto* vlay = new QVBoxLayout;

    //暂时只搞一个table
    ctable = new QEControlledTable;
    table = ctable->table();
    table->setModel(model);
    table->setEditTriggers(QTableView::CurrentChanged);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setItemDelegateForColumn(TimetableStdModel::ColArrive,
        new QETimeDelegate(this));
    table->setItemDelegateForColumn(TimetableStdModel::ColDepart,
        new QETimeDelegate(this));
    vlay->addWidget(ctable);

    int c = 0;
    for (int w : {120, 100, 100, 40, 60, 60, 60}) {
        table->setColumnWidth(c++, w);
    }

    auto* g = new ButtonGroup<2>({ "确定","还原"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, {
        SLOT(actApply()),SLOT(actCancel())
        });

    setLayout(vlay);
}

void BasicTrainWidget::actCancel()
{
    model->actCancel();
}

void BasicTrainWidget::actApply()
{
    model->actApply();
}

