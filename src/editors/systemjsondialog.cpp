#include "systemjsondialog.h"
#ifndef QETRC_MOBILE_2

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include "util/buttongroup.hpp"
#include <data/common/qesystem.h>
#include <SARibbonBar.h>
#include <QStyleFactory>
#include <QApplication>
#include <QMessageBox>
#include "defines/icon_specs.h"

// #define DISABLE_ENGLISH

SystemJsonDialog::SystemJsonDialog(QWidget *parent):
    QDialog(parent)
{
    setWindowTitle(tr("全局选项"));
    //resize(400,500);
    initUI();
    setData();
}

void SystemJsonDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    auto* lab=new QLabel(tr("这部分设置是全局的配置选项，保存在system.json文件中。"
        "提交更改后，退出程序并再次启动时，设置全部生效；在此之前，设置项不一定全部生效。"
        "此配置不会修改当前的运行图文件，亦不提供撤销。"
        "工具栏中可能有其它地方也展示或者可以修改这里的配置项，各处的配置数据不保证同步。\n"
        "若要调整Ribbon风格相关设置，请移步Ribbon设置。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;

    cbLanguage = new QComboBox;
    cbLanguage->addItem(QString("中文"), QLocale::Chinese);
#ifndef DISABLE_ENGLISH
    cbLanguage->addItem(QString("English"), QLocale::English);
#endif
    auto* hlay = new QHBoxLayout;
    hlay->addWidget(cbLanguage);
    hlay->addWidget(new QLabel(tr("重新启动软件生效")));
    flay->addRow(tr("语言"), hlay);

    spRowHeight = new QSpinBox;
    spRowHeight->setRange(1,100000);
    spRowHeight->setToolTip(tr("表格行高\n"
        "程序中各种表格的默认行高度，默认为25."));
    flay->addRow(tr("表格行高"),spRowHeight);

    edDefaultFile=new QLineEdit;
    edDefaultFile->setToolTip(tr("默认运行图文件\n程序启动时，如果上次打开的运行图不能正常打开，"
        "自动打开的运行图文件名。默认为sample.pyetgr"));
    flay->addRow(tr("默认文件"),edDefaultFile);

#if 0
    cbRibbonStyle=new QComboBox;
    cbRibbonStyle->setToolTip(tr("Ribbon风格\n选择默认的Ribbon工具栏风格"));
    cbRibbonStyle->addItem(tr("宽松三行（Office风格）"), SARibbonBar::RibbonStyleLooseThreeRow);
    cbRibbonStyle->addItem(tr("紧凑三行（WPS风格）"), SARibbonBar::RibbonStyleCompactThreeRow);
    cbRibbonStyle->addItem(tr("宽松两行（Office两行风格）"), SARibbonBar::RibbonStyleLooseTwoRow);
    cbRibbonStyle->addItem(tr("紧凑两行（WPS两行风格）"), SARibbonBar::RibbonStyleCompactTwoRow);
    flay->addRow(tr("Ribbon工具栏风格"),cbRibbonStyle);
#endif

    cbSysStyle = new QComboBox;
    cbSysStyle->setToolTip(tr("界面风格\n选择程序采用的界面风格。这里列出当前系统所支持的风格。"));
    cbSysStyle->addItems(QStyleFactory::keys());
    flay->addRow(tr("界面风格"), cbSysStyle);

    ckWeaken=new QCheckBox(tr("启用"));
    ckWeaken->setToolTip(tr("虚化非选择运行线\n在选择运行线时，是否虚化其他的未显示运行线。"));
    flay->addRow(tr("虚化非选择运行线"),ckWeaken);

    ckTooltip=new QCheckBox(tr("启用"));
    ckTooltip->setToolTip(tr("悬停提示运行情况\n对当前选中的运行线，鼠标悬停时显示"
        "鼠标所在区间（车站）的运行（停车）情况。理论上可能带来较大的计算负担。"));
    flay->addRow(tr("悬停提示运行情况"),ckTooltip);

    ckCentral=new QCheckBox(tr("启用"));
    ckCentral->setToolTip(tr("中心运行图面板\n"
        "如果启用，则窗口停靠面板管理系统总是保留中央面板（即使没有任何面板在上面），"
        "此时新的运行图窗口将添加到中央面板；如果不启用，则停靠面板可以任意安排，"
        "新的运行图窗口从右侧添加。"));
    flay->addRow(tr("中心运行图面板"),ckCentral);

    ckAutoHighlight = new QCheckBox(tr("启用"));
    ckAutoHighlight->setToolTip(tr("自动高亮选中车次的运行线\n"
        "在运行图资源管理器、列车管理面板中，选中车次后自动在运行图中高亮该运行线（pyETRC默认行为）。"
        "此功能导致选中车次时消耗更多资源，对于极端运行图可能导致卡顿。"));
    flay->addRow(tr("自动高亮选中车次"), ckAutoHighlight);

    ckStartup = new QCheckBox(tr("显示"));
    ckStartup->setToolTip(tr("启动程序时显示提示页面"));
    flay->addRow(tr("启动提示页"), ckStartup);

    hlay = new QHBoxLayout;
    ckDrag = new QCheckBox(tr("启用"));
    ckDrag->setToolTip(tr("拖动运行线上停点调整时刻。对于通过站，按住Ctrl调整到达时刻，Alt调整出发时刻，否则同时调整。"));
    hlay->addWidget(ckDrag);
    auto* btn = new QToolButton();
    btn->setIcon(QEICN_system_dialog_drag_info);
    hlay->addWidget(btn);
    connect(btn, &QToolButton::clicked, [this]() {
        QMessageBox::information(this, tr("提示"),
            tr("拖动调整时刻功能提示：\n"
                "直接拖动可调整单个车站时刻；\n"
                "对于无停点的车站，直接拖动将同时拖动到达、出发时刻并保持一致，按住Ctrl仅拖动到达时刻，按住Alt仅拖动出发时刻；\n"
                "按住Shift拖动，可平移所拖动点及之后所有车站的时刻；\n"
                "按住Shift+Ctrl拖动，可平移拖动点及之前所有车站的时刻；\n"
                "对于无停点的车站，按住Shift+Alt拖动，可平移出发时刻及之后所有车站时刻；\n"
                "对于无停点的车站，按住Shift+Alt+Ctrl拖动，可平移到达时刻及之前所有车站的时刻。\n"
                "本提示仅自动弹出这一次，后续可在“全局配置选项”中查看。"));
        });
    flay->addRow(tr("拖动调整时刻"), hlay);

    ckTransparentConfig = new QCheckBox(tr("启用"));
    ckTransparentConfig->setToolTip(tr("对新创建的运行图的显示设置、类型管理默认使用透明模式。"));
    flay->addRow(tr("透明设置"), ckTransparentConfig);

    vlay->addLayout(flay);

    auto* g=new ButtonGroup<3>({"确定","还原", "关闭"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(setData()), SLOT(close())});
}

void SystemJsonDialog::setLanguageCombo()
{
    // Currently, a brute-force implementation...
    switch (SystemJson::instance.language) {
    case QLocale::Chinese: cbLanguage->setCurrentIndex(0); break;
#ifndef DISABLE_ENGLISH
    case QLocale::English: cbLanguage->setCurrentIndex(1); break;
#endif
    default: cbLanguage->setCurrentIndex(0); break;
    }
}

void SystemJsonDialog::setData()
{
    const auto& t=SystemJson::instance;
    spRowHeight->setValue(t.table_row_height);
    edDefaultFile->setText(t.default_file);
#if 0
    {
        int ribbon_sty_idx = 0;
        switch (t.ribbon_style) {
        case SARibbonBar::RibbonStyleLooseThreeRow: ribbon_sty_idx = 0; break;
        case SARibbonBar::RibbonStyleCompactThreeRow: ribbon_sty_idx = 1; break;
        case SARibbonBar::RibbonStyleLooseTwoRow: ribbon_sty_idx = 2; break;
        case SARibbonBar::RibbonStyleCompactTwoRow: ribbon_sty_idx = 3; break;
        }
        cbRibbonStyle->setCurrentIndex(ribbon_sty_idx);
    }
#endif
    ckWeaken->setChecked(t.weaken_unselected);
    ckTooltip->setChecked(t.show_train_tooltip);
    ckCentral->setChecked(t.use_central_widget);
    ckAutoHighlight->setChecked(t.auto_highlight_on_selected);
    ckStartup->setChecked(t.show_start_page);
    cbSysStyle->setCurrentText(t.app_style);
    ckDrag->setChecked(t.drag_time);
    ckTransparentConfig->setChecked(t.transparent_config);
    setLanguageCombo();
}

void SystemJsonDialog::actApply()
{
    auto& t=SystemJson::instance;
    t.language = qvariant_cast<QLocale::Language>(cbLanguage->currentData());
    t.table_row_height = spRowHeight->value();
    t.default_file = edDefaultFile->text();
#if 0
    t.ribbon_style = qvariant_cast<SARibbonBar::RibbonStyles>(cbRibbonStyle->currentData());
#endif
    if (t.app_style != cbSysStyle->currentText()) {
        t.app_style = cbSysStyle->currentText();
        auto* sty = QStyleFactory::create(t.app_style);
        qApp->setStyle(sty);
    }
    t.weaken_unselected = ckWeaken->isChecked();
    t.show_train_tooltip = ckTooltip->isChecked();
    t.use_central_widget = ckCentral->isChecked();
    t.auto_highlight_on_selected = ckAutoHighlight->isChecked();
    t.show_start_page = ckStartup->isChecked();
    t.drag_time = ckDrag->isChecked();
    t.transparent_config = ckTransparentConfig->isChecked();
}

#endif
