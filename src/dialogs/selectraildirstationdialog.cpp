#include "selectraildirstationdialog.h"

#include <QTableView>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "util/selectrailwaycombo.h"
#include "model/rail/railstationmodel.h"
#include "data/common/qesystem.h"

SelectRailDirStationDialog::SelectRailDirStationDialog(RailCategory& cat, const QString& prompt, QWidget* parent):
	QDialog(parent), m_cat(cat)
{
	initUI(prompt);
}

QList<std::shared_ptr<const RailStation>> SelectRailDirStationDialog::selectedStations()
{
	QList<std::shared_ptr<const RailStation>> res{};

	auto rail = m_cbRail->railway();
	if (rail) {
		auto sel = m_table->selectionModel()->selectedRows();
		std::sort(sel.begin(), sel.end(),
			[](const QModelIndex& a, const QModelIndex& b) { return a.row() < b.row(); });
		for (const auto& idx : sel) {
			auto station = m_model->getRowStation(idx.row());
			if (station) {
				res.push_back(station);
			}
		}
	}
	return res;
}

SelectRailDirStationDialog::Result SelectRailDirStationDialog::dlgGetStations(RailCategory& cat, QWidget* parent,
	const QString& title, const QString& prompt)
{
	SelectRailDirStationDialog dlg(cat, prompt, parent);
	dlg.setWindowTitle(title);
	dlg.setAttribute(Qt::WA_DeleteOnClose, false);

	if (dlg.exec() == QDialog::Accepted) {
		return Result{
			.valid = true,
			.railway = dlg.m_cbRail->railway(),
			.direction = dlg.m_radioDireciton->get(0)->isChecked() ? Direction::Down : Direction::Up,
			.stations = dlg.selectedStations(),
		};
	}
	else {
		return Result{ .valid = false };
	}
}

void SelectRailDirStationDialog::initUI(const QString& prompt)
{
	setWindowTitle(tr("选择线路车站"));
	resize(600, 600);

	auto* vlay = new QVBoxLayout(this);
	auto* form = new QFormLayout;

	if (!prompt.isEmpty()) {
		auto* lab = new QLabel(prompt);
		lab->setWordWrap(true);
		vlay->addWidget(lab);
	}

	m_cbRail = new SelectRailwayCombo(m_cat, this);
	form->addRow(tr("线路"), m_cbRail);

	m_radioDireciton = new RadioButtonGroup<2>({ "下行", "上行" }, this);
	form->addRow(tr("运行方向"), m_radioDireciton);
	m_radioDireciton->get(0)->setChecked(true); // 默认下行
	vlay->addLayout(form);

	auto* lab2 = new QLabel(tr("请在下表中选择车站，可多选。"));
	vlay->addWidget(lab2);

	m_model = new RailStationModel(false, this);
	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
	
	m_table->setSelectionBehavior(QTableView::SelectRows);
	m_table->setSelectionMode(QTableView::MultiSelection);
	vlay->addWidget(m_table);
	m_table->setModel(m_model);

	connect(m_cbRail, &SelectRailwayCombo::currentRailwayChanged,
		this, &SelectRailDirStationDialog::setupTable);
	connect(m_radioDireciton->get(0), &QRadioButton::toggled, 
		this, &SelectRailDirStationDialog::setupTable);

	auto* box = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	vlay->addWidget(box);

	// We do not use signals here, leave for future...
	connect(box, &QDialogButtonBox::accepted, this, &SelectRailDirStationDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &SelectRailDirStationDialog::reject);

	setupTable();
}

void SelectRailDirStationDialog::setupTable()
{
	auto rail = m_cbRail->railway();
	if (rail) {
		m_model->setRailwayForDir(rail, m_radioDireciton->get(0)->isChecked() ? Direction::Down : Direction::Up);
		m_table->resizeColumnsToContents();
	}
	else {
		m_model->setRowCount(0);
	}
}

void SelectRailDirStationDialog::accept()
{
	auto sel = m_table->selectionModel()->selectedRows();
	if (sel.isEmpty()) {
		QMessageBox::warning(this, tr("错误"), tr("未选择车站。请在表中选择车站，然后再确认。"));
		return;
	}
	QDialog::accept();
}
