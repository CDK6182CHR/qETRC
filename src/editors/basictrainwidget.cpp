#include "basictrainwidget.h"

#include <QtWidgets>
#include "data/diagram/diagram.h"


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
    table->resizeColumnsToContents();
}

void BasicTrainWidget::initUI()
{
    model=new TimetableStdModel(commitInPlace,this);
    auto* vlay = new QVBoxLayout;
    //暂时只搞一个table
    ctable = new QEControlledTable;
    table = ctable->table();
    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    vlay->addWidget(ctable);

    setLayout(vlay);
}
