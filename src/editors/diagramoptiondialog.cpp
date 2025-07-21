#include "diagramoptiondialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QLabel>
#include <QMessageBox>

#include "data/diagram/diagram.h"

#include "mainwindow/mainwindow.h"

DiagramOptionDialog::DiagramOptionDialog(Diagram& diagram, QWidget* parent) :
	QDialog(parent),
	m_diagram(diagram)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("运行图选项"));
	//resize(600, 600);
	initUI();
	setData();
}

void DiagramOptionDialog::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("关于运行图显示的更多细节设置，请移步“运行图显示设置”。"
		"此处所提供的设置项不仅涉及运行图的显示，也涉及运行图数据的计算，因此单列出来。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* flay = new QFormLayout;
	m_spPassedStations = new QSpinBox;
	m_spPassedStations->setRange(0, 100000000);
	flay->addRow(tr("最大跨越站数"), m_spPassedStations);
	m_spPassedStations->setToolTip(tr("最大跨越站数\n"
		"此选项指出自动安排列车运行线时，每条运行线内最大允许的连续非铺画站点数。若连续未铺画的站数超过此数值，"
		"则运行线将被断开。"
		"对于时刻表不完整的运行图（时刻表上不停车的车站被省略），应将此数值设置得充分大，以避免非预期的运行线中断或不完整。"));

	m_spPeriodHours = new QSpinBox;
	m_spPeriodHours->setRange(1, 100000000);
	flay->addRow(tr("周期小时数"), m_spPeriodHours);
	m_spPeriodHours->setToolTip(tr("周期小时数\n"
		"此选项指出运行图的周期长度（小时）。\n"
		"周期长度是指运行图中列车运行线的周期长度，通常为24小时。"
		"注意此选项将影响时间计算的逻辑。"));

	vlay->addLayout(flay);

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	connect(box, &QDialogButtonBox::accepted, this, &DiagramOptionDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &DiagramOptionDialog::reject);
	vlay->addWidget(box);
}

void DiagramOptionDialog::setData()
{
	m_spPassedStations->setValue(m_diagram.options().max_passed_stations);
	m_spPeriodHours->setValue(m_diagram.options().period_hours);
}

void DiagramOptionDialog::accept()
{
	if (m_spPassedStations->value() != m_diagram.options().max_passed_stations) {
		emit passedStationChanged(m_diagram.options().max_passed_stations, m_spPassedStations->value());
	}
	else if (m_spPeriodHours->value() != m_diagram.options().period_hours) {
		auto but = QMessageBox::question(this, tr("提示"), 
			tr("你正在尝试修改周期小时数。\n"
				"请注意，此改动影响范围极大，不仅影响运行图的显示，也影响运行线及所有相关数据的计算，"
				"将导致运行图中所有列车的时刻表被重置，"
				"并且所有运行线将被重新计算。\n"
				"如果仅需要调整显示的运行图宽度，请移步“运行图显示设置”更改起始、结束时间即可。\n"
				"如果确认更改，请提交本更改后立即保存运行图并重新启动软件。\n"
				"此操作不可撤销，且将导致以往所有操作都不可撤销。\n"
				"是否继续？"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if (but != QMessageBox::Yes) {
			return;
		}

		emit periodHoursChanged(m_diagram.options().period_hours, m_spPeriodHours->value());

		QMessageBox::information(this, tr("提示"), tr("周期小时数已更改。\n"
			"请立即保存运行图并重启软件，以防止出现异常情况。\n"
			"请注意此功能为试验性功能，请做好数据备份工作。"
		));
	}
	QDialog::accept();
}

void qecmd::ChangePeriodHours::undo()
{
	m_diagram.options().period_hours = m_old_period_hours;
	m_mw->refreshAll();
}

void qecmd::ChangePeriodHours::redo()
{
	m_diagram.options().period_hours = m_new_period_hours;
	m_mw->refreshAll();
}
