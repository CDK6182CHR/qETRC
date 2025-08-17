#include "rulerfromspeeddialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>

#include <data/common/qesystem.h>
#include <data/rail/ruler.h>
#include <model/rail/rulermodel.h>


RulerFromSpeedDialog::RulerFromSpeedDialog(std::shared_ptr<Ruler> ruler,
                                           QWidget *parent):
    QDialog(parent), ruler(ruler)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("从速度计算标尺 - %1").arg(ruler->name()));
    resize(600, 800);
    initUI();
}

void RulerFromSpeedDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此功能从所给的运行速度（通通运行速度，不包括起停时分）"
        "计算近似的标尺。可以给定数据粒度以将结果修约到指定精度。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;

    auto* hlay=new QHBoxLayout;
    spSpeed=new QDoubleSpinBox();
    spSpeed->setSingleStep(10);
    spSpeed->setRange(0.1, 1E10);
    spSpeed->setSuffix(" km/h");
    spSpeed->setValue(80);
    hlay->addWidget(spSpeed);

    ckAsMax=new QCheckBox(tr("作为上限"));
    hlay->addWidget(ckAsMax);
    flay->addRow(tr("运行速度"), hlay);

    cbPrec=new QComboBox;
    for(int p:{1, 5, 10, 15, 20, 30, 60}){
        cbPrec->addItem(tr("%1 秒").arg(p), p);
    }
    flay->addRow(tr("数据粒度"), cbPrec);

    spStart=new QSpinBox;
    spStart->setRange(0, 10000);
    spStart->setSingleStep(30);
    spStart->setValue(120);
    spStart->setSuffix(tr(" 秒 (s)"));
    flay->addRow(tr("起步附加"),spStart);

    spStop=new QSpinBox;
    spStop->setRange(0, 10000);
    spStop->setSingleStep(30);
    spStop->setValue(120);
    spStop->setSuffix(tr(" 秒 (s)"));
    flay->addRow(tr("停车附加"),spStop);
    vlay->addLayout(flay);

    lab = new QLabel(tr("请选择要应用的区间："));
    vlay->addWidget(lab);

    table = new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    model = new RulerModel(ruler, this);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setSelectionBehavior(QTableView::SelectRows);
    {
        int c = 0;
        for (int w : {150, 50, 50, 50, 50, 80, 80}) {
            table->setColumnWidth(c++, w);
        }
    }
    vlay->addWidget(table);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, SIGNAL(accepted()), this, SLOT(onApply()));
    connect(box, SIGNAL(rejected()), this, SLOT(close()));
    vlay->addWidget(box);
}

void RulerFromSpeedDialog::onApply()
{
    auto sel = table->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("未选择区间。\n"
            "请先在表中选择要应用的区间，只有所选区间会被计算。"));
        return;
    }
    std::vector<bool> selrows(model->rowCount(), false);
    foreach(const auto & idx, sel) {
        selrows.at(idx.row()) = true;
    }
    auto r=ruler->clone();
    r->getRuler(0)->fromSpeed(spSpeed->value(),spStart->value(),spStop->value(),
                     ckAsMax->isChecked(),cbPrec->currentData().toInt(), selrows);
    emit rulerUpdated(ruler, r);
    QMessageBox::information(this, tr("提示"), tr("从运行速度计算标尺完成。"));
    done(QDialog::Accepted);
}
