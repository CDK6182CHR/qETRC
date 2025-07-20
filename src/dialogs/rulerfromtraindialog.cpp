#include "rulerfromtraindialog.h"

#include "data/rail/rail.h"
#include "data/train/train.h"
#include "data/diagram/trainadapter.h"
#include "data/diagram/diagramoptions.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>

RulerFromTrainDialog::RulerFromTrainDialog(DiagramOptions& ops, TrainCollection& coll_,
    std::shared_ptr<Ruler> ruler_, QWidget* parent) :
    QDialog(parent), _ops(ops), coll(coll_), ruler(ruler_)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("单车次标尺提取 - %1").arg(ruler->name()));
    resize(500, 200);
    initUI();
}

void RulerFromTrainDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* lab = new QLabel(tr("请选择一个车次，并设置起停附加时分。系统将读取该车次在本线的运行情况，"
        "更新当前标尺，并覆盖指定区间的数据。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* flay = new QFormLayout;

    cbTrain = new SelectTrainCombo(coll);
    flay->addRow(tr("选择车次"), cbTrain);

    spStart = new QSpinBox;
    spStart->setRange(0, 1000000);
    spStart->setSingleStep(10);
    spStart->setSuffix(tr(" 秒 (s)"));
    spStart->setValue(120);
    flay->addRow(tr("起步附加"), spStart);

    spStop = new QSpinBox;
    spStop->setRange(0, 1000000);
    spStop->setSingleStep(10);
    spStop->setSuffix(tr(" 秒 (s)"));
    spStop->setValue(120);
    flay->addRow(tr("停车附加"), spStop);

    vlay->addLayout(flay);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, SIGNAL(accepted()), this, SLOT(onApply()));
    connect(box, SIGNAL(rejected()), this, SLOT(close()));
    vlay->addWidget(box);
}

void RulerFromTrainDialog::onApply()
{
    auto train = cbTrain->train();
    if (!train) {
        QMessageBox::warning(this, tr("错误"),
            tr("请先选择一个车次！"));
        return;
    }
    auto adp = train->adapterFor(*(ruler->railway()));
    if (!adp) {
        QMessageBox::warning(this, tr("错误"),
            tr("所选择的车次[%1]在当前标尺窗线路[%2]上无运行线，无法读取，"
                "请重新选择车次！").arg(train->trainName().full())
            .arg(ruler->railway()->name()));
        return;
    }
    auto r = ruler->clone();
    int cnt = ruler->fromSingleTrain(adp, spStart->value(), spStop->value(), _ops.period_hours);
    if (cnt) {
        QMessageBox::information(this, tr("提示"),
            tr("成功从指定车次中读取到%1个区间的标尺数据。").arg(cnt));
        ruler->swap(*(r->getRuler(0)));
        emit rulerUpdated(ruler, r);
        done(QDialog::Accepted);
    }
    else {
        QMessageBox::information(this, tr("提示"),
            tr("未能从指定车次中读取到任何区间的标尺数据。"));
    }
}
