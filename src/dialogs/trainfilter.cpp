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
#include "model/delegate/qedelegate.h"
#include "data/diagram/diagram.h"
#include <util/qecontrolledtable.h>
#include "model/general/qemoveablemodel.h"
#include "data/train/routing.h"


TrainFilter::TrainFilter(Diagram &diagram_, QWidget *parent):
    QDialog(parent), diagram(diagram_), core(diagram_)
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

SelectTrainTypeDialog::SelectTrainTypeDialog(TrainCollection &coll_, QWidget *parent):
    QDialog(parent),coll(coll_),model(new QStandardItemModel(this))
{
    setWindowTitle(tr("选择类型"));
    initUI();
}

void SelectTrainTypeDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    view=new QListView;
    view->setModel(model);
    view->setSelectionMode(QListView::MultiSelection);
    vlay->addWidget(view);

    auto* g=new ButtonGroup<3>({"确定","刷新","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshTypes()),SLOT(close())});
}

void SelectTrainTypeDialog::actApply()
{
    _selected.clear();
    auto&& sel=view->selectionModel()->selectedRows();
    foreach(const auto& p,sel){
        auto tp= qvariant_cast<std::shared_ptr<TrainType>>(p.data(qeutil::TrainTypeRole));
        _selected.insert(tp);
    }
    done(Accepted);
}

void SelectTrainTypeDialog::refreshTypes()
{
    const auto& cnt=coll.typeCount();
    model->setRowCount(cnt.count());
    auto* sel=view->selectionModel();
    int row=0;
    for(auto p=cnt.begin();p!=cnt.end();++p){
        auto* it=new QStandardItem(p.key()->name());
        QVariant v;
        v.setValue(p.key());
        it->setData(v,qeutil::TrainTypeRole);
        model->setItem(row,it);
        if (_selected.contains(p.key())){
            sel->select(model->index(row,0),QItemSelectionModel::Select);
        }else{
            sel->select(model->index(row,0),QItemSelectionModel::Deselect);
        }
        row++;
    }
}

void SelectTrainTypeDialog::showDialog()
{
    refreshTypes();
    show();
}

void SelectTrainTypeDialog::clearSelected()
{
    _selected.clear();
}

TrainNameRegexDialog::TrainNameRegexDialog(TrainCollection &coll_, QWidget *parent):
    QDialog(parent),coll(coll_),model(new QEMoveableModel(this))
{
    initUI();
}

void TrainNameRegexDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    model->setColumnCount(1);
    model->setHorizontalHeaderLabels({tr("车次正则")});

    ctab=new QEControlledTable;
    table=ctab->table();
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setModel(model);

    vlay->addWidget(ctab);

    auto* g=new ButtonGroup<3>({"确定","还原","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshData()),SLOT(close())});
}

void TrainNameRegexDialog::actApply()
{
    _names.clear();
    for(int i=0;i<model->rowCount();i++){
        if(auto* it=model->item(i,0)){
            if(!it->text().isEmpty()){
                QRegExp re(it->text());
                _names.push_back(re);
            }
        }
    }
    done(Accepted);
}

void TrainNameRegexDialog::refreshData()
{
    model->setRowCount(_names.size());
    for(int i=0;i<model->rowCount();i++){
        model->setItem(i,new QStandardItem(_names.at(i).pattern()));
    }
}

void TrainNameRegexDialog::clearNames()
{
    _names.clear();
}

SelectRoutingDialog::SelectRoutingDialog(TrainCollection &coll_, QWidget *parent):
    QDialog(parent), coll(coll_), model(new QStandardItemModel(this))
{
    setWindowTitle(tr("选择交路"));
    initUI();
}

void SelectRoutingDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    view=new QListView;
    view->setModel(model);
    view->setSelectionMode(QListView::MultiSelection);
    vlay->addWidget(view);
    auto* g=new ButtonGroup<3>({"确定","刷新","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshRoutings()),SLOT(close())});
}

void SelectRoutingDialog::actApply()
{
    _containsNull=false;
    _selected.clear();
    auto&& sel=view->selectionModel()->selectedRows();
    foreach(const auto& p,sel){
        auto rt=qvariant_cast<std::shared_ptr<Routing>>(p.data(qeutil::RoutingRole));
        if(!rt)
            _containsNull=true;
        else
            _selected.insert(rt);
    }
    done(Accepted);
}

void SelectRoutingDialog::refreshRoutings()
{
    model->setRowCount(coll.routingCount()+1);
    auto sel=view->selectionModel();
    model->setItem(0,new QStandardItem(tr("(无交路)")));
    if(_containsNull)
        sel->select(model->index(0,0),QItemSelectionModel::Select);
    else
        sel->select(model->index(0,0),QItemSelectionModel::Deselect);
    for(int i=0;i<coll.routingCount();i++){
        auto* it=new QStandardItem(coll.routingAt(i)->name());
        QVariant v;
        v.setValue(coll.routingAt(i));
        it->setData(v,qeutil::RoutingRole);
        model->setItem(i+1,it);
        if(_selected.contains(coll.routingAt(i))){
            sel->select(model->index(i+1,0),QItemSelectionModel::Select);
        }else{
            sel->select(model->index(i+1,0),QItemSelectionModel::Deselect);
        }
    }
}

void SelectRoutingDialog::showDialog()
{
    refreshRoutings();
    show();
}

void SelectRoutingDialog::clearSelection()
{
    _containsNull=false;
    _selected.clear();
}
