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
	setWindowTitle(tr("����ͼѡ��"));
	resize(600, 600);
	initUI();
}

void DiagramOptionDialog::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("��������ͼ��ʾ�ĸ���ϸ�����ã����Ʋ�������ͼ��ʾ���á���"
		"�˴����ṩ����������漰����ͼ����ʾ��Ҳ�漰����ͼ���ݵļ��㣬��˵��г�����"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* flay = new QFormLayout;
	m_spPassedStations = new QSpinBox;
	m_spPassedStations->setRange(0, 100000000);
	flay->addRow(tr("����Խվ��"), m_spPassedStations);
	m_spPassedStations->setToolTip(tr("����Խվ��\n"
		"��ѡ��ָ���Զ������г�������ʱ��ÿ�������������������������̻�վ������������δ�̻���վ����������ֵ��"
		"�������߽����Ͽ���"
		"����ʱ�̱�����������ͼ��ʱ�̱��ϲ�ͣ���ĳ�վ��ʡ�ԣ���Ӧ������ֵ���õó�ִ��Ա����Ԥ�ڵ��������жϻ�������"));

	vlay->addLayout(flay);

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	connect(box, &QDialogButtonBox::accepted, this, &DiagramOptionDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &DiagramOptionDialog::reject);
}

void DiagramOptionDialog::accept()
{
	if (m_spPassedStations->value() != m_diagram.config().max_passed_stations) {
		emit passedStationChanged(m_diagram.config().max_passed_stations, m_spPassedStations->value());
	}
	QDialog::accept();
}
