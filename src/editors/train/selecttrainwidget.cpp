#include "selecttrainwidget.h"

#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>

#include "model/train/trainliststdmodel.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "util/selecttraincombo.h"


SelectTrainWidget::SelectTrainWidget(TrainCollection& coll, QWidget* parent):
	QWidget(parent), m_coll(coll),
	m_model(new TrainListStdModel(this))
{
	initUI();
}

std::vector<std::shared_ptr<Train>> SelectTrainWidget::trains() const
{
	return m_model->trains();
}

void SelectTrainWidget::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	vlay->setContentsMargins(0, 0, 0, 0);
	
	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
	m_table->setModel(m_model);

	{
		int c = 0;
		for (int w : {120, 100, 100, 80, 40, 60}) {
			m_table->setColumnWidth(c++, w);
		}
	}

	vlay->addWidget(m_table);

	auto* g = new ButtonGroup<4>({ "添加", "删除", "上移", "下移" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this,
		{ SLOT(actAdd()), SLOT(actRemove()), SLOT(actMoveUp()), SLOT(actMoveDown()) });
}

void SelectTrainWidget::actAdd()
{
	auto train = SelectTrainCombo::dialogGetTrain(m_coll, this);
	if (!train)
		return;
	m_model->addTrain(std::move(train));
}

void SelectTrainWidget::actRemove()
{
	const auto& idx = m_table->currentIndex();
	if (!idx.isValid())
		return;

	m_model->removeRow(idx.row());
}

void SelectTrainWidget::actMoveUp()
{
	const auto& idx = m_table->currentIndex();
	if (!idx.isValid())
		return;
	m_model->moveUp(idx.row());
}

void SelectTrainWidget::actMoveDown()
{
	const auto& idx = m_table->currentIndex();
	if (idx.isValid()) {
		m_model->moveDown(idx.row());
	}
}
