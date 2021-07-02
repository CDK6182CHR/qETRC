#include "navitree.h"

#include <QtWidgets>
#include "model/diagram/diagramnavimodel.h"
#include "addpagedialog.h"

NaviTree::NaviTree(DiagramNaviModel* model_, QWidget *parent):
    QTreeView(parent),mePageList(new QMenu(this)),_model(model_)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));
    initContextMenus();
    setModel(_model);
    expandAll();
}

void NaviTree::initContextMenus()
{
    //PageList
    QAction* act = mePageList->addAction(tr("添加运行图视窗"));
    connect(act, SIGNAL(triggered()), this, SLOT(addNewPage()));
}

NaviTree::ACI* NaviTree::getItem(const QModelIndex& idx)
{
    if (!idx.isValid())
        return nullptr;
    return static_cast<ACI*>(idx.internalPointer());
}

void NaviTree::addNewPage()
{
    auto* dialog = new AddPageDialog(_model->diagram(), this);
    connect(dialog, SIGNAL(creationDone(std::shared_ptr<DiagramPage>)),
        this, SIGNAL(pageAdded(std::shared_ptr<DiagramPage>)));
    dialog->open();
}

void NaviTree::currentChanged(const QModelIndex& cur, const QModelIndex& prev)
{
    ACI* ipre = getItem(prev), *icur = getItem(cur);
    QTreeView::currentChanged(cur, prev);
    //先处理取消选择的
    if (ipre&&(!icur||ipre->type()!=icur->type())) {
        switch (ipre->type()) {
        case navi::PageItem::Type:emit focusOutPage(); break;
        case navi::TrainModelItem::Type:emit focusOutTrain(); break;
        }
    }

    //处理新选择的
    if (icur) {
        switch (icur->type()) {
        case navi::PageItem::Type:emit focusInPage(static_cast<navi::PageItem*>(icur)->page()); break;
        case navi::TrainModelItem::Type:
            emit focusInTrain(static_cast<navi::TrainModelItem*>(icur)->train()); break;
        }
    }
}


void NaviTree::showContextMenu(const QPoint& pos)
{
    QModelIndex idx = currentIndex();
    if (!idx.isValid())return;
    auto* item = static_cast<navi::AbstractComponentItem*>(idx.internalPointer());

    auto p = mapToGlobal(pos);
    
    switch (item->type()) {
    case navi::PageListItem::Type:mePageList->popup(p);
    }
    
}