#include "railstationwidget.h"
#include "util/buttongroup.hpp"
#include "model/delegate/combodelegate.h"
#include <QtWidgets>


RailStationWidget::RailStationWidget(RailCategory& cat_, bool inplace, QWidget* parent) :
	QWidget(parent), cat(cat_), commitInPlace(inplace), model(new RailStationModel(inplace, this))
{
	//暂定Model的Parent就是自己！
	initUI();
}

void RailStationWidget::setRailway(std::shared_ptr<Railway> rail)
{
	railway = rail;
	model->setRailway(rail);
	ctable->table()->resizeColumnsToContents();
	edName->setText(railway->name());
}

void RailStationWidget::refreshData()
{
	model->refreshData();
	ctable->table()->resizeColumnsToContents();
	edName->setText(railway->name());
}

void RailStationWidget::focusInEvent(QFocusEvent* e)
{
	if (railway)
		emit focusInRailway(railway);
}

void RailStationWidget::initUI()
{
	auto* vlay = new QVBoxLayout;

	auto* form = new QFormLayout;
	edName = new QLineEdit;
	form->addRow(tr("线名"), edName);
	vlay->addLayout(form);

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
	//先讨论线名的修改
	const QString& name = edName->text();
	if (name != railway->name()) {
		if (!cat.railNameIsValid(name, railway)) {
			QMessageBox::warning(this, tr("错误"), tr("线路名称不能为空或与其他线名冲突，请重新设置。"));
			return;
		}
		if (commitInPlace) {
			railway->setName(name);
		}
		else {
			emit railNameChanged(railway, name);
		}
	}
	model->actApply();
}
