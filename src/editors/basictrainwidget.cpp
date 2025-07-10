#include "basictrainwidget.h"

#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "model/delegate/qetimedelegate.h"
#include "data/common/qeglobal.h"

#include "model/train/timetablestdmodel.h"
#include "util/qecontrolledtable.h"
#include "editors/timetablewidget.h"
#include "dialogs/selectraildirstationdialog.h"

#include <QTableView>
#include <QHeaderView>


BasicTrainWidget::BasicTrainWidget(TrainCollection &coll_,RailCategory& cat, bool commitInPlace_,
    QWidget *parent) : QWidget(parent),
    coll(coll_), _cat(cat),
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

    auto* g = new ButtonGroup<3>({ "确定","还原","导入站表"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, {
		SLOT(actApply()),SLOT(actCancel()), SLOT(actImportRailStations())
        });

    setLayout(vlay);
}

void BasicTrainWidget::actCancel()
{
    ctable->model()->actCancel();
}

void BasicTrainWidget::actImportRailStations()
{
	auto res = SelectRailDirStationDialog::dlgGetStations(_cat, this,
		tr("导入站表"), tr("请选择要导入的车站列表。"
			"注意：导入的车站将被添加到当前车次的时刻表中，"
			"请确保车站列表的正确性。"));
    if (res.valid) {
        ctable->appendStations(res.stations);
    }
}

void BasicTrainWidget::actApply()
{
    ctable->model()->actApply();
}

