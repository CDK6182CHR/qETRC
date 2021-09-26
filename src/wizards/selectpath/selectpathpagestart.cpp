#include "selectpathpagestart.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>


SelectPathPageStart::SelectPathPageStart(QWidget *parent):
    QWizardPage(parent)
{
    initUI();
}

bool SelectPathPageStart::exportRuler() const
{
    return ckRuler->isChecked();
}

int SelectPathPageStart::minRulerCount() const
{
    return spRulerCount->value();
}

void SelectPathPageStart::initUI()
{
    setTitle(tr("开始"));
    setSubTitle(tr("欢迎使用径路生成向导。此功能从线网有向图模型中，支持使用"
        "最短路、邻线和邻站三种模式来确定线路关键点，以精确控制径路的生成。\n"
        "本系统也支持仅给出数个关键点，一律采用最短路算法确定径路的功能，即"
        "快速路径选择 （Ctrl+J）。"));

    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("以下是关于标尺的选项。选择[在结果中包含区间标尺信息]，将把"
        "线网中的区间标尺数据输出到结果中；但有数据的区间数量少于[最小标尺区间数]的标尺"
        "将被删除。\n"
        "各区间标尺仅按照标尺名称（而不考虑原来线路数据库中的线名）进行合并。相同标尺名称的，"
        "将判定为同一个标尺。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* flay=new QFormLayout();
    ckRuler=new QCheckBox(tr("在结果中包含区间标尺信息"));
    flay->addRow(tr("标尺选项"),ckRuler);

    spRulerCount=new QSpinBox();
    spRulerCount->setRange(1,100000);
    spRulerCount->setMaximumWidth(200);
    flay->addRow(tr("最小标尺区间数"),spRulerCount);
    vlay->addLayout(flay);
}
