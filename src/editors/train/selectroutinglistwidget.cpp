#include "selectroutinglistwidget.h"

#include <QStandardItemModel>
#include "data/train/routing.h"
#include "data/train/traincollection.h"
#include "model/delegate/qedelegate.h"

SelectRoutingListWidget::SelectRoutingListWidget(TrainCollection &coll_, QWidget *parent):
    QListView(parent), coll(coll_), model(new QStandardItemModel(this))
{
    initUI();
}

typename SelectRoutingListWidget::res_t SelectRoutingListWidget::result() const
{
    bool _containsNull=false;
    typename res_t::first_type _selected;
    auto&& sel=selectionModel()->selectedRows();
    foreach(const auto& p,sel){
        auto rt=qvariant_cast<std::shared_ptr<Routing>>(p.data(qeutil::RoutingRole));
        if(!rt)
            _containsNull=true;
        else
            _selected.insert(rt);
    }
    return std::make_pair(_selected,_containsNull);
}

void SelectRoutingListWidget::refreshRoutings()
{
    refreshRoutingsWithSelection(result());
}

void SelectRoutingListWidget::refreshRoutingsWithSelection(const res_t &_data)
{
    const auto& _selected=_data.first;
    bool _containsNull=_data.second;
    model->setRowCount(coll.routingCount()+1);
    auto sel=selectionModel();
    model->setItem(0,new QStandardItem(tr("(无交路)")));
    if(_containsNull)
        sel->select(model->index(0,0),QItemSelectionModel::Select);
    else
        sel->select(model->index(0,0),QItemSelectionModel::Deselect);
    for(int i=0;i<coll.routingCount();i++){
        auto* it=new QStandardItem(coll.routingAt(i)->name());
        it->setData(QVariant::fromValue(coll.routingAt(i)), qeutil::RoutingRole);
        model->setItem(i+1,it);
        if(_selected.contains(coll.routingAt(i))){
            sel->select(model->index(i+1,0),QItemSelectionModel::Select);
        }else{
            sel->select(model->index(i+1,0),QItemSelectionModel::Deselect);
        }
    }
}

void SelectRoutingListWidget::initUI()
{
    setModel(model);
    setSelectionMode(QListView::MultiSelection);
}

void SelectRoutingListWidget::clearSelection()
{
    refreshRoutingsWithSelection({});
}
