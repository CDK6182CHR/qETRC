#include "selectpagecombo.h"
#include "data/diagram/diagrampage.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

SelectPageCombo::SelectPageCombo(const QList<std::shared_ptr<DiagramPage>>& pages, QWidget* parent):
QComboBox(parent), pages(pages)
{
	initData();
}

std::shared_ptr<DiagramPage> SelectPageCombo::currentPage()
{
	if (auto idx = currentIndex(); idx >= 0 && idx < pages.size()) {
		return pages.at(idx);
	}
	else {
		return nullptr;
	}
}

std::shared_ptr<DiagramPage> SelectPageCombo::dialogGetPage(const QList<std::shared_ptr<DiagramPage>>& pages, QWidget* parent, const QString& title, const QString& prompt)
{
	QDialog dlg(parent);
	dlg.setWindowTitle(title);

	auto* vlay = new QVBoxLayout(&dlg);

	if (!prompt.isEmpty()) {
		auto* lab = new QLabel(prompt);
		lab->setWordWrap(true);
		vlay->addWidget(lab);
	}
	auto* combo = new SelectPageCombo(pages);
	vlay->addWidget(combo);

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	vlay->addWidget(box);
	connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
	connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

	auto ret = dlg.exec();

	if (ret) {
		auto page = combo->currentPage();
		return page;
	}
	else {
		return nullptr;
	}
}

void SelectPageCombo::initData()
{
	clear();
	foreach(auto p, pages) {
		addItem(p->name());
	}
}
