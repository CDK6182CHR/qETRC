#include "pathedit.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTextEdit>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>

#include "model/trainpath/pathmodel.h"
#include "data/common/qesystem.h"
#include "data/trainpath/trainpath.h"
#include "util/buttongroup.hpp"
#include "model/trainpath/pathrailwaydelegate.h"
#include "model/trainpath/pathstationdelegate.h"
#include "model/delegate/generaldoublespindelegate.h"

PathEdit::PathEdit(RailCategory& railcat, QWidget* parent):
	railcat(railcat), model(new PathModel(parent))
{
	initUI();
}

void PathEdit::setPath(TrainPath* path)
{
	_path = path;
	model->setPath(path);
	refreshData();
}

void PathEdit::refreshData()
{
	edName->setText(_path->name());
	edNote->setText(_path->note());
	model->refreshData();
}

void PathEdit::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* flay = new QFormLayout;

	edName = new QLineEdit;
	flay->addRow(tr("名称"), edName);
	vlay->addLayout(flay);

	table = new QTableView;
	table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	table->setModel(model);
	{
		int c = 0;
		for (int w : {120, 100, 100, 60, 60}) {
			table->setColumnWidth(c++, w);
		}
	}
	vlay->addWidget(table);
	table->setEditTriggers(QTableView::AllEditTriggers);
	table->setItemDelegateForColumn(PathModel::ColRail, new PathRailwayDelegate(railcat, model, this));
	auto* dele = new PathStationDelegate(model, this);
	table->setItemDelegateForColumn(PathModel::ColStart, dele);
	table->setItemDelegateForColumn(PathModel::ColEnd, dele);
	table->setItemDelegateForColumn(PathModel::ColMile, new GeneralDoubleSpinDelegate(this, 3));

	auto* g = new ButtonGroup<3>({ "前插", "后插", "删除" });
	g->connectAll(SIGNAL(clicked()), this, { SLOT(actAddBefore()), SLOT(actAddAfter()), SLOT(actRemove()) });
	vlay->addLayout(g);

	vlay->addWidget(new QLabel("备注: "));
	edNote = new QTextEdit;
	edNote->setMaximumHeight(120);
	vlay->addWidget(edNote);
	
	auto* g2 = new ButtonGroup<3>({ "确定", "取消", "刷新" });
	g2->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()),SLOT(actCancel()), SLOT(actRefresh()) });
	vlay->addLayout(g2);
}

void PathEdit::actAddBefore()
{
	const auto& idx = table->currentIndex();
	int row = idx.row();
	if (row < 0) row = 0;
	model->insertEmptyRow(row);
}

void PathEdit::actAddAfter()
{
	const auto& idx = table->currentIndex();
	int row = idx.row();
	if (row < 0) row = model->rowCount() - 1;
	model->insertEmptyRow(row + 1);
}

void PathEdit::actRemove()
{
	// todo
}

void PathEdit::actApply()
{
	// todo
}

void PathEdit::actCancel()
{
	// todo
}

void PathEdit::actRefresh()
{
	// todo
}
