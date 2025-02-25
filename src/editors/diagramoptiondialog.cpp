#include "diagramoptiondialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QLabel>

#include "data/diagram/diagram.h"

DiagramOptionDialog::DiagramOptionDialog(Diagram& diagram, QWidget* parent) :
	QDialog(parent),
	m_diagram(diagram)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("运行图选项"));
	//resize(600, 600);
	initUI();
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

	vlay->addLayout(flay);

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	connect(box, &QDialogButtonBox::accepted, this, &DiagramOptionDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &DiagramOptionDialog::reject);
	vlay->addWidget(box);
}

void DiagramOptionDialog::accept()
{
	if (m_spPassedStations->value() != m_diagram.config().max_passed_stations) {
		emit passedStationChanged(m_diagram.config().max_passed_stations, m_spPassedStations->value());
	}
	QDialog::accept();
}
