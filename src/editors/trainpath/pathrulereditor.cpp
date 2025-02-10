#include "pathrulereditor.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QFormLayout>

#include "model/trainpath/pathrulermodel.h"
#include "data/common/qesystem.h"
#include "data/trainpath/trainpath.h"
#include "util/buttongroup.hpp"

PathRulerEditor::PathRulerEditor(std::shared_ptr<PathRuler>  ruler, QWidget* parent):
	QWidget(parent), 
	m_model(new PathRulerModel(*ruler, this)),
	m_ruler(std::move(ruler))
{
	resize(800, 600);
	initUI();
}

void PathRulerEditor::refreshBasicData()
{
	m_edRulerName->setText(m_model->ruler().name());
	m_edPathName->setText(m_model->path()->name());
}

void PathRulerEditor::refreshData()
{
	m_model->setRuler(*m_ruler);
	refreshBasicData();
}

void PathRulerEditor::setRuler(std::shared_ptr<PathRuler>  ruler)
{
	m_ruler = std::move(ruler);
	refreshData();
}

void PathRulerEditor::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("�г���·�ı�������漰�ĸ�����·��ߵķֶ���ϡ�"
		"�����±���ѡ���г���·���Σ�segment������Ӧ�ı�����ơ�"
		"���Ӧ��·�ޱ�߻��������ߣ���ת����·�ı�߱༭��Ӷ�Ӧ��ߡ�"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* flay = new QFormLayout;
	m_edRulerName = new QLineEdit;
	flay->addRow(tr("�������"), m_edRulerName);

	auto* hlay = new QHBoxLayout;
	m_edPathName = new QLineEdit;
	m_edPathName->setReadOnly(true);
	hlay->addWidget(m_edPathName);
	m_labValid = new QLabel;
	hlay->addWidget(m_labValid);
	flay->addRow(tr("������·"), hlay);
	vlay->addLayout(flay);

	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	m_table->setModel(m_model);
	{
		int c = 0;
		for (int w : {100, 100, 100, 40, 40, 120, 60, 50}) {
			m_table->setColumnWidth(c++, w);
		}
	}
	vlay->addWidget(m_table);
}
