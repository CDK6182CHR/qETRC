#include "ribbonconfigdialog.h"

#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include <SARibbonBar.h>
#include <SARibbonMainWindow.h>

#include "util/buttongroup.hpp"
#include "data/common/qesystem.h"

RibbonConfigDialog::RibbonConfigDialog(QWidget *parent):
    QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Ribbon设置"));
    initUI();
    refreshData();
}

void RibbonConfigDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此处包含Ribbon工具栏风格相关的设置项。"
                              "点击确定或应用后，设置立即生效，且在下次启动时自动设置。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;

    cbStyle=new QComboBox;
    cbStyle->setToolTip(tr("Ribbon风格\n选择默认的Ribbon工具栏风格"));
    cbStyle->addItem(tr("宽松三行（Office风格）"), (int) SARibbonBar::RibbonStyleLooseThreeRow);
    cbStyle->addItem(tr("紧凑三行（WPS风格）"), (int)SARibbonBar::RibbonStyleCompactThreeRow);
    cbStyle->addItem(tr("宽松两行（Office两行风格）"), (int)SARibbonBar::RibbonStyleLooseTwoRow);
    cbStyle->addItem(tr("紧凑两行（WPS两行风格）"), (int)SARibbonBar::RibbonStyleCompactTwoRow);
    flay->addRow(tr("Ribbon工具栏风格"),cbStyle);

    cbTheme=new QComboBox; 
    cbTheme->addItem("Theme Win7", static_cast<int>(SARibbonTheme::RibbonThemeWindows7));
    cbTheme->addItem("Theme Office2013", static_cast<int>(SARibbonTheme::RibbonThemeOffice2013));
    cbTheme->addItem("Theme Office2016 Blue", static_cast<int>(SARibbonTheme::RibbonThemeOffice2016Blue));
    cbTheme->addItem("Theme Office2021 Blue", static_cast<int>(SARibbonTheme::RibbonThemeOffice2021Blue));
    cbTheme->addItem("Theme Dark", static_cast<int>(SARibbonTheme::RibbonThemeDark));
    cbTheme->addItem("Theme Dark2", static_cast<int>(SARibbonTheme::RibbonThemeDark2));
    flay->addRow(tr("Ribbon工具栏主题"), cbTheme);

    ckCenter=new QCheckBox(tr("启用"));
    flay->addRow(tr("居中布局"), ckCenter);
    vlay->addLayout(flay);

    auto* g=new ButtonGroup<3>({"应用", "确定", "取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this,
                  {SLOT(actApply()), SLOT(actOk()), SLOT(close())});
}

void RibbonConfigDialog::refreshData()
{
    const auto& d = SystemJson::get();
    cbStyle->setCurrentIndex(cbStyle->findData(d.ribbon_style));
    cbTheme->setCurrentIndex(cbTheme->findData(d.ribbon_theme));
    ckCenter->setChecked(d.ribbon_align_center);
}

void RibbonConfigDialog::actApply()
{
    auto& d = SystemJson::get();
    bool theme_changed = d.ribbon_theme != cbTheme->currentData();
    d.ribbon_style = cbStyle->currentData().toInt();
    d.ribbon_theme = cbTheme->currentData().toInt();
    d.ribbon_align_center = ckCenter->isChecked();
    emit configApplied(theme_changed);
}

void RibbonConfigDialog::actOk()
{
    actApply();
    accept();
}
