#include "modifytimetabledialog.h"
#include <QtWidgets>
#include <algorithm>
#include "data/diagram/diagram.h"
#include "util/utilfunc.h"
#include "data/train/train.h"

ModifyTimetableDialog::ModifyTimetableDialog(std::shared_ptr<Train> train_, QWidget* parent) :
	QDialog(parent), train(train_), table(new TrainTimetablePlane)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("时刻表微调 - %1").arg(train->trainName().full()));
	table->setTrain(train);
	resize(500, 600);
	initUI();
}

void ModifyTimetableDialog::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* flay = new QFormLayout;
	gpDir = new RadioButtonGroup<2>({ "提前","延后" }, this);
	gpDir->get(0)->setChecked(true);
	flay->addRow(tr("调整方向"), gpDir);
	auto* hlay = new QHBoxLayout;
	spMin = new QSpinBox;
	spMin->setRange(0, 1000000);
	spMin->setSuffix(tr(" 分 (min)"));
	hlay->addWidget(spMin);
	spSec = new QSpinBox;
	spSec->setSingleStep(10);
	spSec->setRange(0, 59);
	spSec->setSuffix(tr(" 秒 (s)"));
	hlay->addWidget(spSec);
	flay->addRow(tr("调整时间"), hlay);

	hlay = new QHBoxLayout;
	ckFirst = new QCheckBox(tr("首站到达时刻"));
	ckFirst->setChecked(true);
	hlay->addWidget(ckFirst);
	ckLast = new QCheckBox(tr("末站出发时刻"));
	ckLast->setChecked(true);
	hlay->addWidget(ckLast);
	flay->addRow(tr("边界条件"), hlay);
	vlay->addLayout(flay);

	auto* lab = new QLabel(tr("请在下表中选择一连续范围进行调整。如果不选择，则没有车站时刻会被调整。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	table->setSelectionBehavior(QTableView::SelectRows);
	table->setSelectionMode(QTableView::ContiguousSelection);
	vlay->addWidget(table);

	auto* box = new ButtonGroup<2>({ "确定","取消" });
	box->connectAll(SIGNAL(clicked()), this, { SLOT(onApply()),SLOT(close()) });
	vlay->addLayout(box);

}

void ModifyTimetableDialog::onApply()
{
	auto&& sel = table->selectionModel()->selectedRows();
	if (sel.isEmpty()) {
		QMessageBox::warning(this, tr("错误"), tr("请先选择要调整的车站!"));
		return;
	}
	int secs = spMin->value() * 60 + spSec->value();
	if (gpDir->get(0)->isChecked()) {
		secs = -secs;
	}
	if (!secs) {
		QMessageBox::warning(this, tr("错误"), tr("调整时长为0，不产生任何效果。"));
		return;
	}
	int start = std::min_element(sel.begin(), sel.end(), qeutil::ltIndexRow)->row();
	int end = std::max_element(sel.begin(), sel.end(), qeutil::ltIndexRow)->row();
	auto t = std::make_shared<Train>(*train);    //copy construct
	t->adjustTimetable(start, end, ckFirst->isChecked(), ckLast->isChecked(), secs);
	emit trainUpdated(train, t);
	done(QDialog::Accepted);
}
