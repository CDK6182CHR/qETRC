#include "selectraildirstationdialog.h"

#include <QTableView>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDialogButtonBox>

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
	setWindowTitle(tr("ѡ����·��վ"));
	resize(800, 600);

	auto* vlay = new QVBoxLayout(this);
	auto* form = new QFormLayout;

	if (!prompt.isEmpty()) {
		auto* lab = new QLabel(prompt);
		lab->setWordWrap(true);
		vlay->addWidget(lab);
	}

	m_cbRail = new SelectRailwayCombo(m_cat, this);
	form->addRow(tr("��·"), m_cbRail);

	m_radioDireciton = new RadioButtonGroup<2>({ "����", "����" }, this);
	form->addRow(tr("���з���"), m_radioDireciton);
	m_radioDireciton->get(0)->setChecked(true); // Ĭ������
	vlay->addLayout(form);

	auto* lab2 = new QLabel(tr("�����±���ѡ��վ���ɶ�ѡ��"));
	vlay->addWidget(lab2);

	m_model = new RailStationModel(false, this);
	m_table = new QTableView;
	m_table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	
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

	connect(box, &QDialogButtonBox::accepted, this, &SelectRailDirStationDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &SelectRailDirStationDialog::reject);
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
