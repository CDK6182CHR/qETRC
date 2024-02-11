#include "batchaddstopdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

BatchAddStopDialog::BatchAddStopDialog(QWidget* parent):
	QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("设置批量停点"));
	initUI();
}

std::optional<typename BatchAddStopDialog::BatchAddStopReport>
	BatchAddStopDialog::setBatchAdd(QWidget* parent)
{
	BatchAddStopDialog d{ parent };
	d.setAttribute(Qt::WA_DeleteOnClose, false);
	auto flag = d.exec();
	if (flag) {
		BatchAddStopReport rep;
		rep.constr_range = d.rdRange->get(1)->isChecked();
		rep.constr_level = d.ckLevel->isChecked();
		rep.stop_secs = d.spMin->value() * 60 + d.spSec->value();
		rep.lowest_level = d.spLevel->value();
		return rep;
	}
	return std::nullopt;
}

void BatchAddStopDialog::initUI()
{
	auto* vlay = new QVBoxLayout(this);

	auto* lab = new QLabel(tr("此功能为满足所选条件的车站统一添加停点（原有数据将被覆盖），" 
		"适用于批量添加相同停车时间的需求。"
		"请注意，“所选行”要求选中整行；“车站等级”数字越大为等级越低，0代表特等站为最高等级。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* flay = new QFormLayout;
	rdRange = new RadioButtonGroup<2>({ "全部车站", "所选行" }, this);
	rdRange->get(0)->setChecked(true);
	flay->addRow(tr("车站范围"), rdRange);

	auto* hlay = new QHBoxLayout;
	ckLevel = new QCheckBox(tr("不低于指定等级的车站: "));
	hlay->addWidget(ckLevel);
	spLevel = new QSpinBox;
	spLevel->setRange(0, 10000);
	spLevel->setEnabled(false);
	spLevel->setValue(3);
	hlay->addWidget(spLevel);
	flay->addRow(tr("车站等级"), hlay);

	connect(ckLevel, &QCheckBox::toggled,
		spLevel, &QSpinBox::setEnabled);

	hlay = new QHBoxLayout;
	spMin = new QSpinBox;
	spMin->setSuffix(tr(" 分 (min)"));
	spMin->setRange(0, 10000);
	hlay->addWidget(spMin);
	spSec = new QSpinBox;
	spSec->setSuffix(tr(" 秒 (s)"));
	hlay->addWidget(spSec);
	spSec->setRange(0, 10000);
	spSec->setSingleStep(30);

	flay->addRow(tr("停车时间"), hlay);

	vlay->addLayout(flay);

	auto* g = new ButtonGroup<2>({ "确定", "取消" });
	g->connectAll(SIGNAL(clicked()), this, { SLOT(accept()), SLOT(reject()) });
	vlay->addLayout(g);
}
