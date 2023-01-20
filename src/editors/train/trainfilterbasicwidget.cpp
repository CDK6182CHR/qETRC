#include "trainfilterbasicwidget.h"
#include "data/train/trainfiltercore.h"
#include "selecttraintypelistwidget.h"
#include "trainfilterhelpers.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QListWidget>
#include <QTextBrowser>

#include <util/qefoldwidget.h>

TrainFilterBasicWidget::TrainFilterBasicWidget(TrainCollection &coll,
                                               TrainFilterCore* core, QWidget *parent)
    : QWidget{parent}, coll(coll), _core(core)
{
    initUI();
}

void TrainFilterBasicWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    ckType=new QCheckBox(tr("列车种类"));
    lstType=new SelectTrainTypeListWidget(coll);
    auto* fold=new QEFoldWidget(ckType, lstType);
    fold->expand();
    vlay->addWidget(fold);

    //vlay->addStretch(1);

//    ckType=new QCheckBox(tr("列车种类"));
//    auto* btn=new QPushButton(tr("选择类型"));
//    connect(btn,&QPushButton::clicked,this,&TrainFilterBasicWidget::selectType);
//    flay->addRow(ckType,btn);

//    ckInclude = new QCheckBox(tr("包含车次"));
//    btn=new QPushButton(tr("设置包含车次"));
//    connect(btn,&QPushButton::clicked,this,&TrainFilterBasicWidget::setInclude);
//    flay->addRow(ckInclude,btn);

//    ckExclude=new QCheckBox(tr("排除车次"));
//    btn=new QPushButton(tr("设置排除车次"));
//    connect(btn,&QPushButton::clicked,this,&TrainFilterBasicWidget::setExclude);
//    flay->addRow(ckExclude,btn);

//    ckRouting=new QCheckBox(tr("属于交路"));
//    btn=new QPushButton(tr("选择交路"));
//    connect(btn,&QPushButton::clicked,this,&TrainFilterBasicWidget::selectRouting);
//    flay->addRow(ckRouting,btn);

    auto* flay = new QFormLayout;
    gpPassen=new RadioButtonGroup<3>({"客车","非客车","全部"},this);
    gpPassen->get(2)->setChecked(true);
    flay->addRow(tr("是否客车"),gpPassen);

    ckShowOnly = new QCheckBox(tr("启用"));
    flay->addRow(tr("仅包括当前显示车次"),ckShowOnly);

    ckInverse = new QCheckBox(tr("启用"));
    flay->addRow(tr("反向选择"),ckInverse);
    vlay->addLayout(flay);
}

void TrainFilterBasicWidget::selectType()
{
    //if(!dlgType){
    //    dlgType=new SelectTrainTypeDialog(coll,this);
    //}
    //dlgType->showDialog();
}


void TrainFilterBasicWidget::setInclude()
{
    if(!dlgInclude){
        dlgInclude=new TrainNameRegexDialog(coll,this);
        dlgInclude->setWindowTitle(tr("包含车次"));
    }
    dlgInclude->show();
}

void TrainFilterBasicWidget::setExclude()
{
    if(!dlgExclude){
        dlgExclude=new TrainNameRegexDialog(coll,this);
        dlgExclude->setWindowTitle(tr("排除车次"));
    }
    dlgExclude->show();
}

void TrainFilterBasicWidget::selectRouting()
{
    if(!dlgRouting){
        dlgRouting=new SelectRoutingDialog(coll,this);
    }
    dlgRouting->showDialog();
}

void TrainFilterBasicWidget::actApply()
{
    _core->useType = ckType->isChecked();
    //if (_core->useType && dlgType) {
    //    _core->types = dlgType->selected();   // copy assign
    //}
    _core->useInclude = ckInclude->isChecked();
    if (_core->useInclude && dlgInclude) {
        _core->includes = dlgInclude->names();
    }
    _core->useExclude = ckExclude->isChecked();
    if (_core->useExclude && dlgExclude) {
        _core->excludes = dlgExclude->names();
    }
    _core->useRouting = ckRouting->isChecked();
    if (_core->useRouting && ckRouting) {
        _core->routings = dlgRouting->selected();
        _core->selNullRouting = dlgRouting->containsNull();
    }
    _core->showOnly = ckShowOnly->isChecked();
    _core->useInverse = ckInverse->isChecked();

    // 客车类型  Auto表示都行
    _core->passengerType = TrainPassenger::Auto;
    if (gpPassen->get(0)->isChecked())
        _core->passengerType = TrainPassenger::True;
    else if (gpPassen->get(1)->isChecked())
        _core->passengerType = TrainPassenger::False;
}

void TrainFilterBasicWidget::clearFilter()
{
    ckType->setChecked(false);
    ckInclude->setChecked(false);
    ckExclude->setChecked(false);
    ckRouting->setChecked(false);
    ckShowOnly->setChecked(false);
    ckInverse->setChecked(false);
    gpPassen->get(2)->setChecked(true);
}

void TrainFilterBasicWidget::refreshData()
{
    lstType->refreshTypesWithSelection(_core->types);
}


