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

PathEdit::PathEdit(QWidget* parent):
	model(new PathModel(parent))
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
	auto* g = new ButtonGroup<2>({ "添加", "删除" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this, { SLOT(actAdd()), SLOT(actRemove()) });

	vlay->addWidget(new QLabel("备注: "));
	edNote = new QTextEdit;
	edNote->setMaximumHeight(120);
	vlay->addWidget(edNote);
	
	auto* g2 = new ButtonGroup<3>({ "确定", "取消", "刷新" });
	g2->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()),SLOT(actCancel()), SLOT(actRefresh()) });
	vlay->addLayout(g2);
}

void PathEdit::actAdd()
{
	// todo
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
