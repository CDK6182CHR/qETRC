#include "readrulerpageconfig.h"

#include <QtWidgets>


ReadRulerPageConfig::ReadRulerPageConfig(QWidget *parent):
    QWizardPage(parent)
{
    initUI();
}

void ReadRulerPageConfig::initUI()
{
    setTitle(tr("参数配置"));
    setSubTitle("配置计算选项，点击[下一步]显示预览。\n"
                   "说明：\n"
                   "众数模式将选择出现次数最多的区间数据作为标尺数据，"
                   "均值模式将对所有符合条件的数据求平均值作为标尺数据；\n"
                   "默认起步、停车附加时分是当区间没有足够数据时使用的。\n"
                   "当选择均值模式时，可以选择清除偏离样本均值一定秒数或者一定样本标准差倍数的"
                   "数据，以排除极端数据的影响。\n");
    auto* flay=new QFormLayout(this);
    gpMode=new RadioButtonGroup<2>({"众数模式","均值模式"},this);
    gpMode->get(0)->setChecked(true);
    connect(gpMode->get(1),SIGNAL(toggled(bool)),this,SLOT(onMeanToggled(bool)));
    flay->addRow(tr("算法选择"),gpMode);

    cbPrec=new QComboBox;
    for(auto p:{1,5,10,15,30,60}){
        cbPrec->addItem(tr("%1秒").arg(p),p);
    }
    flay->addRow(tr("数据粒度"),cbPrec);

    spStart=new QSpinBox;
    spStart->setRange(0,100000);
    spStart->setSingleStep(10);
    spStart->setValue(120);
    spStart->setSuffix(tr(" 秒 (s)"));
    flay->addRow(tr("默认起步附加"),spStart);

    spStop=new QSpinBox;
    spStop->setRange(0,1000000);
    spStop->setSingleStep(10);
    spStop->setSuffix(tr(" 秒 (s)"));
    spStop->setValue(120);
    flay->addRow(tr("默认停车附加"),spStop);

    gpFilt=new QButtonGroup(this);
    auto* cvl=new QVBoxLayout;
    auto* rd=new QRadioButton(tr("不筛选"));
    rd->setChecked(true);
    gpFilt->addButton(rd,0);
    cvl->addWidget(rd);

    auto* chl=new QHBoxLayout;
    rd=new QRadioButton(tr("截断于"));
    gpFilt->addButton(rd,1);
    chl->addWidget(rd);
    spCutSec=new QSpinBox;
    spCutSec->setRange(1,100000);
    spCutSec->setValue(30);
    spCutSec->setSuffix(tr(" 秒 (s)"));
    spCutSec->setEnabled(false);
    chl->addWidget(spCutSec);
    cvl->addLayout(chl);
    connect(rd,SIGNAL(toggled(bool)),this,SLOT(onCutSecToggled(bool)));

    chl=new QHBoxLayout;
    rd=new QRadioButton(tr("截断于"));
    gpFilt->addButton(rd,2);
    chl->addWidget(rd);
    spCutStd=new QSpinBox;
    spCutStd->setRange(1,100000);
    spCutStd->setValue(30);
    spCutStd->setSuffix(tr(" 倍标准差"));
    spCutStd->setEnabled(false);
    chl->addWidget(spCutStd);
    cvl->addLayout(chl);
    connect(rd,SIGNAL(toggled(bool)),this,SLOT(onCutStdToggled(bool)));

    flay->addRow(tr("离群数据筛选"), cvl);

    spCutCount=new QSpinBox;
    spCutCount->setRange(1,1000000);
    flay->addRow(tr("最低类数据量"),spCutCount);

    onMeanToggled(false);
}

void ReadRulerPageConfig::onMeanToggled(bool on)
{
    for(int i=0;i<3;i++){
        gpFilt->button(i)->setEnabled(on);
    }
}

void ReadRulerPageConfig::onCutSecToggled(bool on)
{
    spCutSec->setEnabled(on);
}

void ReadRulerPageConfig::onCutStdToggled(bool on)
{
    spCutStd->setEnabled(on);
}
