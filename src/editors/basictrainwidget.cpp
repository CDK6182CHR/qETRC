#include "basictrainwidget.h"

#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "model/delegate/qetimedelegate.h"

#include "model/train/timetablestdmodel.h"
#include "util/qecontrolledtable.h"
#include "editors/timetablewidget.h"

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
    ctable->model()->setTrain(train);   //自带刷新操作
    refreshBasicData();
}

void BasicTrainWidget::refreshData()
{
    ctable->model()->refreshData();
    refreshBasicData();
}

void BasicTrainWidget::refreshBasicData()
{
    //table->resizeColumnsToContents();
}

TimetableStdModel* BasicTrainWidget::timetableModel()
{
    return ctable->model();
}

void BasicTrainWidget::initUI()
{
    auto* vlay = new QVBoxLayout;

    ctable = new TimetableWidget(commitInPlace, this);
    table = ctable->table();
    //暂时只搞一个table
    vlay->addWidget(ctable);

    auto* g = new ButtonGroup<2>({ "确定","还原"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, {
        SLOT(actApply()),SLOT(actCancel())
        });

    setLayout(vlay);
}

void BasicTrainWidget::actCancel()
{
    ctable->model()->actCancel();
}

void BasicTrainWidget::actApply()
{
    ctable->model()->actApply();
}

