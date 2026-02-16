#include "selectcategorydialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QCheckBox>
#include "raildbmodel.h"

SelectCategoryDialog::SelectCategoryDialog(std::shared_ptr<RailDB> raildb, const QString& prompt, QWidget* parent):
	QDialog(parent), m_raildb(std::move(raildb)), 
	m_model(new RailDBModel(m_raildb, true, this))
{
	initUI(prompt);
}

void SelectCategoryDialog::accept()
{
	if (m_ckRoot->isChecked()) {
		m_selectedPath = {};
		QDialog::accept();
		return;
	}

	const auto& idx = m_tree->currentIndex();
	if (!idx.isValid()) {
		QMessageBox::warning(this, tr("错误"), tr("请先选择一个分类，或指定选择根分类！"));
		return;
	}

	// Store the result
	auto* it = m_model->getItem(idx);
	m_selectedPath = it->path();

	QDialog::accept();
}

SelectCategoryDialog::GetCategoryResult SelectCategoryDialog::getCategory(
	std::shared_ptr<RailDB> raildb, const QString& prompt, QWidget* parent)
{
	SelectCategoryDialog dialog(std::move(raildb), prompt, parent);
	dialog.setAttribute(Qt::WA_DeleteOnClose, false);
	auto flag = dialog.exec();

	if (flag == QDialog::Accepted) {
		return GetCategoryResult{ .accepted = true, .path = std::move(dialog.m_selectedPath) };
	}
	else {
		return GetCategoryResult{ .accepted = false, .path = {} };
	}
}

void SelectCategoryDialog::initUI(const QString& prompt)
{
	setWindowTitle(tr("选择线路分类"));
	resize(800, 600);
	auto* vlay = new QVBoxLayout(this);
	auto* label = new QLabel(prompt, this);
	vlay->addWidget(label);

	m_ckRoot = new QCheckBox(tr("选择根分类"));
	vlay->addWidget(m_ckRoot);

	m_tree = new QTreeView;
	m_tree->setModel(m_model);
	m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	vlay->addWidget(m_tree);

	{
		int c = 0;
		for (int w : {200, 80, 80, 80, 80, 80}) {
			m_tree->setColumnWidth(c++, w);
		}
	}

	connect(m_ckRoot, &QCheckBox::toggled, [this](bool checked) {
		m_tree->setEnabled(!checked);
		});

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vlay->addWidget(box);

	connect(box, &QDialogButtonBox::accepted, this, &SelectCategoryDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &SelectCategoryDialog::reject);
}
