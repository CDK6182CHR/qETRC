#if 0

#include "trainfilterhelpers.h"

#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>


#include <util/buttongroup.hpp>
#include <util/qecontrolledtable.h>
#include <data/train/traincollection.h>
#include <data/train/traintype.h>
#include <model/delegate/qedelegate.h>
#include <model/general/qemoveablemodel.h>
#include <data/common/qesystem.h>
#include <data/train/routing.h>

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
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
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

#endif
