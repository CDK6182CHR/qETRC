#include "edittraintagdialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QMessageBox>

#include "data/train/traintag.h"
#include "data/train/traintagmanager.h"
#include "util/buttongroup.hpp"

EditTrainTagDialog::EditTrainTagDialog(std::shared_ptr<TrainTag> tag, TrainTagManager& manager, QWidget* parent):
	QDialog(parent), m_tag(tag), m_manager(manager)
{
	setWindowTitle(tr("编辑标签"));
	initUI();
	refreshData();
}

EditTrainTagDialog::EditTrainTagDialog(TrainTagManager& manager, QWidget* parent):
	QDialog(parent), m_manager(manager)
{
	setWindowTitle(tr("新建标签"));
	initUI();
	refreshData();
}

void EditTrainTagDialog::initUI()
{
	setAttribute(Qt::WA_DeleteOnClose);
	resize(400, 400);;
	auto* vlay = new QVBoxLayout(this);
	auto* form = new QFormLayout;

	m_edName = new QLineEdit;
	form->addRow(tr("标签名"), m_edName);
	vlay->addLayout(form);

	auto* lab = new QLabel(tr("备注："));
	vlay->addWidget(lab);

	m_edNote = new QTextEdit;
	vlay->addWidget(m_edNote);

	auto* g = new ButtonGroup<3>({ "确定", "刷新", "关闭" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this,
		{ SLOT(accept()), SLOT(refreshData()), SLOT(reject()) });
}

void EditTrainTagDialog::accept()
{
	// First check the name validity
	if (!m_manager.tagNameIsValid(m_edName->text(), m_tag)) {
		QMessageBox::warning(this, tr("错误"),
			tr("非法的标签名称。标签名称必须非空且不与其他标签名重复。"));
		return;
	}

	auto note = m_edNote->toPlainText();
	if (m_tag) {
		// Editing existed tag
		if (m_edName->text() != m_tag->name()) {
			emit tagNameChanged(m_tag, std::make_shared<TrainTag>(m_edName->text(), note));
		}
		else if (note != m_tag->note()) {
			emit tagNoteChanged(m_tag, std::make_shared<TrainTag>(m_tag->name(), note));
		}
	}
	else {
		emit tagAdded(std::make_shared<TrainTag>(m_edName->text(), note));
	}
	QDialog::accept();
}

void EditTrainTagDialog::refreshData()
{
	if (m_tag) {
		m_edName->setText(m_tag->name());
		m_edNote->setPlainText(m_tag->note());
	}
	else {
		m_edName->clear();
		m_edNote->clear();
	}
}
