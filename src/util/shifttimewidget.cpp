#include "shifttimewidget.h"

#include <QDialog>
#include <QSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>

ShiftTimeWidget::ShiftTimeWidget(QWidget* parent):
	QWidget(parent)
{
	initUI();
}

int ShiftTimeWidget::shiftSeconds() const
{
	return m_spMin->value() * 60 + m_spSec->value() * (m_radioGroup->get(0)->isChecked() ? -1 : 1);
}

int ShiftTimeWidget::dlgGetShiftSeconds(QWidget* parent, const QString& title, const QString& prompt, bool* ok)
{
	auto* dlg = new QDialog(parent);
	dlg->setWindowTitle(title);
	dlg->setAttribute(Qt::WA_DeleteOnClose, false);

	auto* vlay = new QVBoxLayout;
	if (!prompt.isEmpty()) {
		auto* label = new QLabel(prompt);
		label->setWordWrap(true);
		vlay->addWidget(label);
	}
	auto* w = new ShiftTimeWidget;
	vlay->addWidget(w);

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
	connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
	vlay->addWidget(box);

	dlg->setLayout(vlay);

	int flag = dlg->exec();
	if (ok) {
		*ok = (flag == QDialog::Accepted);
	}

	int res = 0;
	if (flag == QDialog::Accepted) {
		res = w->shiftSeconds();
	}

	w->deleteLater();
	return res;
}

void ShiftTimeWidget::initUI()
{
	auto* flay = new QFormLayout(this);
	m_radioGroup = new RadioButtonGroup<2>({ "提前", "延后" }, this);
	m_radioGroup->get(0)->setChecked(true);
	flay->addRow(tr("调整方向"), m_radioGroup);

	m_spMin = new QSpinBox;
	m_spMin->setRange(0, 1440);
	m_spMin->setSuffix(tr(" 分钟"));

	m_spSec = new QSpinBox;
	m_spSec->setRange(0, 59);
	m_spSec->setWrapping(true);
	m_spSec->setSuffix(tr(" 秒"));

	auto* hlay = new QHBoxLayout;
	hlay->addWidget(m_spMin);
	hlay->addWidget(m_spSec);

	flay->addRow(tr("调整时长"), hlay);
}
