#include "railstationwidget.h"
#include "util/buttongroup.hpp"
#include "model/delegate/combodelegate.h"
#include <QtWidgets>


RailStationWidget::RailStationWidget(QUndoStack* undo, QWidget *parent) : 
	QWidget(parent),_undo(undo),model(new RailStationModel(undo,this))
{
	//暂定Model的Parent就是自己！
	initUI();
	connect(model, &RailStationModel::stationTableChanged,
		this, &RailStationWidget::stationTableChanged);
}

void RailStationWidget::setRailway(std::shared_ptr<Railway> rail)
{
	railway = rail;
	model->setRailway(rail);
	ctable->table()->resizeColumnsToContents();
	//edName->setText(rail->name());
}

void RailStationWidget::initUI()
{
	auto* vlay = new QVBoxLayout;

	//auto* form = new QFormLayout;
	//edName = new QLineEdit;
	//form->addRow(tr("线名"), edName);
	//vlay->addLayout(form);

	ctable = new QEControlledTable;
	ctable->table()->verticalHeader()->setDefaultSectionSize(25);
	ctable->table()->setModel(model);
	auto* dele = new ComboDelegate({ tr("不通过"),tr("下行"),tr("上行"),tr("上下行") }, this);
	ctable->table()->setItemDelegateForColumn(RailStationModel::ColDir, dele);
	ctable->table()->setEditTriggers(QTableView::CurrentChanged);
	vlay->addWidget(ctable);

	auto* g = new ButtonGroup<2>({ "确定","还原" });
	g->setMinimumWidth(50);
	g->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()),SLOT(actCancel()) });
	vlay->addLayout(g);
	
	setLayout(vlay);
}

void RailStationWidget::actCancel()
{
	model->actCancel();
}

void RailStationWidget::actApply()
{
	model->actApply();
}
