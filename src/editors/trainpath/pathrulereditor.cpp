#include "pathrulereditor.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>

#include "model/trainpath/pathrulermodel.h"
#include "model/trainpath/pathrulernamedelegate.h"
#include "data/common/qesystem.h"
#include "data/trainpath/trainpath.h"
#include "util/buttongroup.hpp"

PathRulerEditor::PathRulerEditor(std::shared_ptr<PathRuler>  ruler, QWidget* parent):
	QWidget(parent), 
	m_model(new PathRulerModel(*ruler, this)),
	m_delRulerName(new PathRulerNameDelegate(m_model->path(), this)),
	m_ruler(std::move(ruler)),
	m_existedRuler(true)
{
	setWindowTitle(tr("编辑径路标尺 - %1").arg(m_ruler->name()));
	initUI();
	refreshBasicData();
}

PathRulerEditor::PathRulerEditor(TrainPath* path, QWidget* parent) :
	QWidget(parent),
	m_model(new PathRulerModel(PathRuler{ path }, this)),
	m_delRulerName(new PathRulerNameDelegate(m_model->path(), this)),
	m_ruler(),
	m_existedRuler(false)
{
	setWindowTitle(tr("新建标尺"));
	initUI();
	refreshBasicData();
}

void PathRulerEditor::refreshBasicData()
{
	if (m_ruler) {
		m_edRulerName->setText(m_model->ruler().name());
	}
	m_edPathName->setText(m_model->path()->name());
	if (m_model->path()->valid()) {
		m_labValid->setText(tr("径路可用"));
	}
	else {
		m_labValid->setText(tr("径路不可用"));
	}
}

void PathRulerEditor::refreshData()
{
	if (m_ruler) {
		m_model->setRuler(*m_ruler);
		refreshBasicData();
	}
}

void PathRulerEditor::setRuler(std::shared_ptr<PathRuler>  ruler)
{
	m_ruler = std::move(ruler);
	m_delRulerName->setPath(m_ruler->path());
	refreshData();
}

void PathRulerEditor::initUI()
{
	resize(800, 600);
	setAttribute(Qt::WA_DeleteOnClose);

	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("列车径路的标尺是所涉及的各段线路标尺的分段组合。"
		"请在下表中选择列车径路各段（segment）所对应的标尺名称。"
		"如对应线路无标尺或无所需标尺，请转至线路的标尺编辑添加对应标尺。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* flay = new QFormLayout;
	m_edRulerName = new QLineEdit;
	flay->addRow(tr("标尺名称"), m_edRulerName);

	auto* hlay = new QHBoxLayout;
	m_edPathName = new QLineEdit;
	m_edPathName->setReadOnly(true);
	hlay->addWidget(m_edPathName);
	m_labValid = new QLabel;
	hlay->addWidget(m_labValid);
	flay->addRow(tr("所属径路"), hlay);
	vlay->addLayout(flay);

	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	m_table->setItemDelegateForColumn(PathRulerModel::ColRulerName, m_delRulerName);
	m_table->setModel(m_model);
	m_table->setEditTriggers(QTableView::AllEditTriggers);
	{
		int c = 0;
		for (int w : {100, 100, 100, 40, 80, 150, 70, 70}) {
			m_table->setColumnWidth(c++, w);
		}
	}
	vlay->addWidget(m_table);

	if (m_existedRuler) {
		auto* g = new ButtonGroup<3>({ "确定", "还原", "取消" });
		vlay->addLayout(g);
		g->connectAll(SIGNAL(clicked()), this,
			{ SLOT(actApplyModify()), SLOT(refreshData()), SLOT(close()) });
	}
	else {
		auto* g = new ButtonGroup<2>({ "确定", "取消" });
		vlay->addLayout(g);
		g->connectAll(SIGNAL(clicked()), this, { SLOT(actApplyAdd()), SLOT(close()) });
	}
}

bool PathRulerEditor::checkAppliedDataValidity()
{
	if (!m_model->path()->rulerNameIsValid(m_edRulerName->text(), m_ruler)) {
		QMessageBox::warning(this, tr("错误"), tr("非法的标尺名称：标尺名不得与本径路已有标尺重复或者为空"));
		return false;
	}
	for (int r = 0; r < m_model->rowCount({}); r++) {
		int idx = m_model->data(m_model->index(r, PathRulerModel::ColRulerName, {}), Qt::EditRole).toInt();
		if (idx < 0) {
			QMessageBox::warning(this, tr("错误"), tr("第%1行：未选择标尺。"
				"请保证每段线路都已选择非空的标尺。如果线路没有标尺，请转至线路工具页面添加。").arg(r));
			return false;
		}
	}
	return true;
}

void PathRulerEditor::actApplyAdd()
{
	if (!checkAppliedDataValidity())
		return;
	auto ruler = std::make_shared<PathRuler>(m_model->ruler());   // copy construct
	ruler->setName(m_edRulerName->text());
	emit rulerAdded(ruler);
	close();
}

void PathRulerEditor::actApplyModify()
{
	if (!checkAppliedDataValidity())
		return;
	auto newruler = std::make_shared<PathRuler>(m_model->ruler());   // copy
	newruler->setName(m_edRulerName->text());
	emit rulerModified(m_ruler, newruler);
	close();
}
