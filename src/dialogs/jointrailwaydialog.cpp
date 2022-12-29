#include "jointrailwaydialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include "data/rail/railway.h"

#include <util/buttongroup.hpp>
#include <util/selectrailwaycombo.h>

JointRailwayDialog::JointRailwayDialog(RailCategory &railcat,
                                       std::shared_ptr<Railway> railway,
                                       QWidget *parent):
    QDialog(parent), railcat(railcat), railway(railway)
{
    initUI();
    setAttribute(Qt::WA_DeleteOnClose);
}

void JointRailwayDialog::initUI()
{
    setWindowTitle(tr("基线拼接 - %1").arg(railway->name()));
    auto* vlay=new QVBoxLayout(this);

    auto* lab=new QLabel(tr("将所选线路拼接到当前线路之前或之后。"
        "若为标尺排图，执行后将变为按里程排图。请注意暂不支持合并标尺、天窗数据。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    cbRail=new SelectRailwayCombo(railcat);
    flay->addRow(tr("选择线路"),cbRail);

    auto* g2=new RadioButtonGroup<2>({"置于本线之前","置于本线之后"},this);
    g2->get(1)->setChecked(true);
    flay->addRow(tr("连接顺序"),g2);
    rdSeq=g2;

    ckReverse=new QCheckBox(tr("反排当前线路"));
    flay->addRow(tr("选项"),ckReverse);

    vlay->addLayout(flay);

    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal);
    connect(box, &QDialogButtonBox::accepted,
            this,&JointRailwayDialog::actApply);
    connect(box,&QDialogButtonBox::rejected,
            this,&QDialog::close);
    vlay->addWidget(box);
}

void JointRailwayDialog::actApply()
{
    auto nrail=cbRail->railway();
    if (!nrail){
        QMessageBox::warning(this,tr("错误"),tr("请先选择线路"));
        return;
    }

    auto lhs=railway->cloneBase();
    lhs->resetOrdinate();
    lhs->mergeIntervalData(*railway);
    lhs->jointWith(*nrail, rdSeq->get(0)->isChecked(),ckReverse->isChecked());
    emit applied(railway, lhs);
}
