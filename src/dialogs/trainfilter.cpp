#include "trainfilter.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTableView>
#include <QHeaderView>
#include "util/buttongroup.hpp"
#include "data/train/traincollection.h"
#include "data/diagram/diagram.h"

#include "editors/train/trainfilterhelpers.h"


TrainFilter::TrainFilter(Diagram &diagram_, QWidget *parent):
    QDialog(parent), diagram(diagram_), core()
{
    resize(400,500);
    setWindowTitle(tr("车次筛选器"));
    initUI();
}

QList<std::shared_ptr<Train>> TrainFilter::selectedTrains() const
{
    QList<std::shared_ptr<Train>> res;
    foreach(auto train, diagram.trainCollection().trains()) {
        if (check(train))
            res.push_back(train);
    }
    return res;
}

void TrainFilter::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout();
    ckType=new QCheckBox(tr("列车种类"));
    auto* btn=new QPushButton(tr("选择类型"));
    connect(btn,&QPushButton::clicked,this,&TrainFilter::selectType);
    flay->addRow(ckType,btn);

    ckInclude = new QCheckBox(tr("包含车次"));
    btn=new QPushButton(tr("设置包含车次"));
    connect(btn,&QPushButton::clicked,this,&TrainFilter::setInclude);
    flay->addRow(ckInclude,btn);

    ckExclude=new QCheckBox(tr("排除车次"));
    btn=new QPushButton(tr("设置排除车次"));
    connect(btn,&QPushButton::clicked,this,&TrainFilter::setExclude);
    flay->addRow(ckExclude,btn);

    ckRouting=new QCheckBox(tr("属于交路"));
    btn=new QPushButton(tr("选择交路"));
    connect(btn,&QPushButton::clicked,this,&TrainFilter::selectRouting);
    flay->addRow(ckRouting,btn);

    gpPassen=new RadioButtonGroup<3>({"客车","非客车","全部"},this);
    gpPassen->get(2)->setChecked(true);
    flay->addRow(tr("是否客车"),gpPassen);

    ckShowOnly = new QCheckBox(tr("启用"));
    flay->addRow(tr("仅包括当前显示车次"),ckShowOnly);

    ckInverse = new QCheckBox(tr("启用"));
    flay->addRow(tr("反向选择"),ckInverse);
    vlay->addLayout(flay);
    auto* g=new ButtonGroup<3>({"确定","清空","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(clearFilter()),SLOT(close())});
}

bool TrainFilter::check(std::shared_ptr<const Train> train) const
{
    return core.check(train);
}

void TrainFilter::selectType()
{
    if(!dlgType){
        dlgType=new SelectTrainTypeDialog(diagram.trainCollection(),this);
    }
    dlgType->showDialog();
}

void TrainFilter::setInclude()
{
    if(!dlgInclude){
        dlgInclude=new TrainNameRegexDialog(diagram.trainCollection(),this);
        dlgInclude->setWindowTitle(tr("包含车次"));
    }
    dlgInclude->show();
}

void TrainFilter::setExclude()
{
    if(!dlgExclude){
        dlgExclude=new TrainNameRegexDialog(diagram.trainCollection(),this);
        dlgExclude->setWindowTitle(tr("排除车次"));
    }
    dlgExclude->show();
}

void TrainFilter::selectRouting()
{
    if(!dlgRouting){
        dlgRouting=new SelectRoutingDialog(diagram.trainCollection(),this);
    }
    dlgRouting->showDialog();
}

void TrainFilter::actApply()
{
    core.useType = ckType->isChecked();
    if (core.useType && dlgType) {
        core.types = dlgType->selected();   // copy assign
    }
    core.useInclude = ckInclude->isChecked();
    if (core.useInclude && dlgInclude) {
        core.includes = dlgInclude->names();
    }
    core.useExclude = ckExclude->isChecked();
    if (core.useExclude && dlgExclude) {
        core.excludes = dlgExclude->names();
    }
    core.useRouting = ckRouting->isChecked();
    if (core.useRouting && ckRouting) {
        core.routings = dlgRouting->selected();
        core.selNullRouting = dlgRouting->containsNull();
    }
    core.showOnly = ckShowOnly->isChecked();
    core.useInverse = ckInverse->isChecked();

    // 客车类型  Auto表示都行
    core.passengerType = TrainPassenger::Auto;
    if (gpPassen->get(0)->isChecked())
        core.passengerType = TrainPassenger::True;
    else if (gpPassen->get(1)->isChecked())
        core.passengerType = TrainPassenger::False;

    emit filterApplied(this);
    done(Accepted);
}

void TrainFilter::clearFilter()
{
    ckType->setChecked(false);
    ckInclude->setChecked(false);
    ckExclude->setChecked(false);
    ckRouting->setChecked(false);
    ckShowOnly->setChecked(false);
    ckInverse->setChecked(false);
    gpPassen->get(2)->setChecked(true);
}
