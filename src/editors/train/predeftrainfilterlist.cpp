#include "predeftrainfilterlist.h"
#include "util/buttongroup.hpp"

#include <QListView>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "data/train/traincollection.h"
#include "model/delegate/qedelegate.h"

PredefTrainFilterList::PredefTrainFilterList(TrainCollection &coll, QWidget *parent):
    QWidget(parent), coll(coll)
{
    initUI();
}

void PredefTrainFilterList::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    view=new QListView;
    view->setEditTriggers(QListView::NoEditTriggers);
    model=new QStandardItemModel(this);

    view->setModel(model);
    vlay->addWidget(view);

    connect(view->selectionModel(),&QItemSelectionModel::currentChanged,
            this,&PredefTrainFilterList::listSelectionChanged);

    auto* g=new ButtonGroup<2>({"添加","删除"});
    g->connectAll(&QPushButton::clicked, this,
        &PredefTrainFilterList::actAdd, &PredefTrainFilterList::actRemove);
    vlay->addLayout(g);
}

void PredefTrainFilterList::listSelectionChanged(const QModelIndex &idx)
{
    if (!idx.isValid())return;
    auto* f=qvariant_cast<PredefTrainFilterCore*>(
                idx.data(qeutil::PredefTrainFilterPointerRole));
    emit currentChanged(f);
}

void PredefTrainFilterList::actAdd()
{
    emit addFilter(coll);
}

void PredefTrainFilterList::actRemove()
{
    auto t=view->currentIndex();
    if (t.isValid()){
        emit removeFilter(coll,t.row());
    }
}

void PredefTrainFilterList::refreshList()
{
    const auto& filters=coll.filters();
    model->setRowCount(coll.filters().size());
    for(size_t i=0;i<filters.size();i++){
        auto* f=filters.at(i).get();
        auto* it=new QStandardItem(f->name());
        it->setData(QVariant::fromValue(f), qeutil::PredefTrainFilterPointerRole);
        model->setItem(i,it);
    }
}
