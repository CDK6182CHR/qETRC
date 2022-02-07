#include "selectrailwaystable.h"

#include <model/diagram/railtablemodel.h>
#include <data/diagram/diagram.h>
#include <data/common/qesystem.h>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>

SelectRailwaysTable::SelectRailwaysTable(Diagram& diagram, QWidget* parent) :
	QTableView(parent), diagram(diagram), model(new RailTableModel(diagram, this))
{
	setEditTriggers(NoEditTriggers);
	verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	setModel(model);
	resizeColumnsToContents();
	setSelectionBehavior(SelectRows);
	setSelectionMode(MultiSelection);
}

QList<std::shared_ptr<Railway> > SelectRailwaysTable::selectedRailways() const
{
	auto sel = selectionModel()->selectedRows();
	QList<std::shared_ptr<Railway>> res{};
	foreach(const auto & idx, sel) {
		res.push_back(diagram.railwayAt(idx.row()));
	}
	return res;
}

QList<std::shared_ptr<Railway> > SelectRailwaysTable::dlgGetRailways(QWidget* parent,
	Diagram& diagram, const QString& title, const QString& prompt, bool* ok)
{
	QDialog* dialog = new QDialog(parent);
	dialog->setWindowTitle(title);
	auto* vlay = new QVBoxLayout(dialog);
	if (!prompt.isEmpty()) {
		auto* lab = new QLabel(prompt);
		lab->setWordWrap(true);
		vlay->addWidget(lab);
	}

	auto* cb = new SelectRailwaysTable(diagram);
	vlay->addWidget(cb);
	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vlay->addWidget(box);
	connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
	connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
	dialog->resize(800, 600);
	auto t = dialog->exec();
	QList<std::shared_ptr<Railway>> res{};
	if (ok) {
		*ok = static_cast<bool>(t);
	}
	if (t) {
		res = cb->selectedRailways();
	}
	dialog->setParent(nullptr);
	delete dialog;
	return res;
}
