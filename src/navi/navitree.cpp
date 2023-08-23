#include "navitree.h"
#include "data/rail/railway.h"

#ifndef QETRC_MOBILE_2

#include "model/diagram/diagramnavimodel.h"
#include "addpagedialog.h"
#include "dialogs/importtraindialog.h"
#include "util/buttongroup.hpp"
#include "data/train/train.h"
#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"
#include "util/selectrailwaystable.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTreeView>
#include <QInputDialog>

NaviTree::NaviTree(DiagramNaviModel* model_, QUndoStack* undo, QWidget* parent) :
    QWidget(parent), mePageList(new QMenu(this)), meRailList(new QMenu(this)),
    meTrainList(new QMenu(this)), mePage(new QMenu(this)), meTrain(new QMenu(this)),
    meRailway(new QMenu(this)), meRuler(new QMenu(this)), meForbid(new QMenu(this)),
    meRouting(new QMenu(this)),meRoutingList(new QMenu(this)),
    mePath(new QMenu(this)), mePathList(new QMenu(this)),
    _model(model_), _undo(undo)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    initUI();
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));

    initContextMenus();
}

void NaviTree::initUI()
{
    //setContentsMargins(0, 0, 0, 0);
    auto* vlay = new QVBoxLayout(this);
    auto* g = new ButtonGroup<3>({ "展开","折叠","刷新" });
    g->setMinimumWidth(50);
    vlay->addLayout(g);
    tree = new QTreeView;
    vlay->addWidget(tree);

    connect(tree, &QAbstractItemView::doubleClicked, this, &NaviTree::onDoubleClicked);
    tree->setModel(_model);
    tree->setColumnWidth(0, 150);
    tree->setColumnWidth(1, 40);
    connect(tree->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &NaviTree::onCurrentChanged);
    connect(g->get(0), SIGNAL(clicked()), tree, SLOT(expandAll()));
    connect(g->get(1), SIGNAL(clicked()), tree, SLOT(collapseAll()));
    connect(g->get(2), SIGNAL(clicked()), _model, SLOT(resetModel()));
}

void NaviTree::initContextMenus()
{
    QAction* actExpand = new QAction(tr("展开"),this);
    connect(actExpand, &QAction::triggered, this, &NaviTree::actExpand);
    QAction* actCollapse = new QAction(tr("折叠"),this);
    connect(actCollapse, &QAction::triggered, this, &NaviTree::actCollapse);
    QAction* actSep = new QAction();
    actSep->setSeparator(true);
    QList<QAction*> common{ actExpand,actCollapse,actSep };

    //PageList
    mePageList->addActions(common);
    QAction* act = mePageList->addAction(tr("添加运行图视窗"));
    connect(act, SIGNAL(triggered()), this, SLOT(addNewPage()));

    //RailList
    meRailList->addActions(common);
    act = meRailList->addAction(tr("导入线路"));
    connect(act, SIGNAL(triggered()), this, SLOT(actImportRailways()));
    act = meRailList->addAction(tr("添加空白线路"));
    connect(act, SIGNAL(triggered()), this, SLOT(actAddRailway()));

    //TrainList
    meTrainList->addActions(common);
    act = meTrainList->addAction(tr("导入车次"));
    connect(act, SIGNAL(triggered()), this, SLOT(importTrains()));
    act = meTrainList->addAction(tr("新建空白车次"));
    connect(act, SIGNAL(triggered()), this, SLOT(actAddTrain()));

    //Page
    mePage->addAction(tr("切换到运行图"), this, &NaviTree::onActivatePageContext);
    act = mePage->addAction(tr("删除运行图"));
    connect(act, SIGNAL(triggered()), this, SLOT(onRemovePageContext()));
    mePage->addAction(tr("创建运行图页面副本"), this, &NaviTree::onDulplicatePageContext);
    mePage->addSeparator();
    mePage->addAction(tr("编辑包含的线路"), this, &NaviTree::onEditRailwayFromPageContext);
    mePage->addAction(tr("转到包含的线路"), this, &NaviTree::onSwitchToRailwayFromPageContext);

    //Train
    meTrain->addAction(tr("编辑列车"), this, &NaviTree::onEditTrainContext);
    meTrain->addAction(tr("编辑时刻表"), this, &NaviTree::onEditTimetableContext);
    act = meTrain->addAction(tr("删除列车"));
    connect(act, SIGNAL(triggered()), this, SLOT(onRemoveSingleTrainContext()));
    meTrain->addAction(tr("创建列车副本"), this, &NaviTree::onDulplicateTrainContext);
    meTrain->addSeparator();
    meTrain->addAction(tr("转到交路"), this, &NaviTree::onSwitchToTrainRoutingContext);

    //Railway
    meRailway->addActions(common);
    act = meRailway->addAction(tr("编辑基线"));
    connect(act, SIGNAL(triggered()), this, SLOT(onEditRailwayContext()));
    act = meRailway->addAction(tr("删除基线"));
    connect(act, SIGNAL(triggered()), this, SLOT(onRemoveRailwayContext()));
    meRailway->addAction(tr("创建基线副本"), this, &NaviTree::onDulplicateRailwayContext);
    meRailway->addSeparator();
    meRailway->addAction(tr("快速创建单线路运行图"), this, &NaviTree::onCreatePageByRailContext);

    //Ruler
    act = meRuler->addAction(tr("编辑标尺"));
    connect(act, SIGNAL(triggered()), this, SLOT(onEditRulerContext()));
    meRuler->addAction(tr("合并标尺"), this, &NaviTree::onMergeRulerContext);
    act = meRuler->addAction(tr("删除标尺"));
    connect(act, SIGNAL(triggered()), this, SLOT(onRemoveRulerContext()));
    meRuler->addAction(tr("创建标尺副本"), this, &NaviTree::onDulplicateRulerContext);

    //Forbid
    act = meForbid->addAction(tr("编辑天窗"));
    connect(act, SIGNAL(triggered()), this, SLOT(onEditForbidContext()));

    //Routing
    act = meRouting->addAction(tr("编辑交路"));
    connect(act, qOverload<bool>(&QAction::triggered), this, &NaviTree::onEditRoutingContext);
    act = meRouting->addAction(tr("删除交路"));
    connect(act, &QAction::triggered, this, &NaviTree::onRemoveRoutingContext);

    //RoutingList
    meRoutingList->addActions(common);
    act = meRoutingList->addAction(tr("添加交路"));
    connect(act, &QAction::triggered, this, &NaviTree::actAddRouting);
    act = meRoutingList->addAction(tr("批量解析"));
    connect(act, &QAction::triggered, this, &NaviTree::actBatchParseRouting);
    act = meRoutingList->addAction(tr("批量识别车次"));
    connect(act, &QAction::triggered, this, &NaviTree::actBatchDetectRouting);

    //PathList
    mePathList->addActions(common);
    mePathList->addAction(tr("添加列车径路"), this, &NaviTree::actAddPath);
    
    //Path
    mePath->addAction(tr("编辑列车径路"), this, &NaviTree::onEditPathContext);
    mePath->addAction(tr("删除列车径路"), this, &NaviTree::onRemovePathContext);
    mePath->addAction(tr("创建列车径路副本"), this, &NaviTree::onDuplicatePathContext);
}

NaviTree::ACI* NaviTree::getItem(const QModelIndex& idx)
{
    if (!idx.isValid())
        return nullptr;
    return static_cast<ACI*>(idx.internalPointer());
}

void NaviTree::addNewPageApply(std::shared_ptr<DiagramPage> page)
{
    _undo->push(new qecmd::AddPage(_model->diagram(), page, this));
}

void NaviTree::onRemovePageContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::PageItem::Type) {
        removePage(idx.row());
    }
}

void NaviTree::onRemoveSingleTrainContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::TrainModelItem::Type) {
        removeSingleTrain(idx.row());
    }
}

#if 0
[[deprecated]]
void NaviTree::onRemoveRailwayContext()
{
    auto res = QMessageBox::question(this, tr("删除基线"),
        tr("此操作将删除所选基线数据，同时从所有运行图中移除该基线。空白的运行图将被删除。\n"
            "请注意此操作关涉较广且不可撤销。是否继续？"));
    if (res != QMessageBox::Yes)
        return;
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RailwayItem::Type) {
        _model->removeRailwayAt(idx.row());
    }
}
#endif

void NaviTree::onRemoveRailwayContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RailwayItem::Type) {
        emit actRemoveRailwayAt(item->row());
    }
}

void NaviTree::onDulplicateRailwayContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RailwayItem::Type) {
        auto rail = static_cast<navi::RailwayItem*>(item)->railway();
        actDulplicateRailway(rail);
    }
}

void NaviTree::onDulplicateTrainContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::TrainModelItem::Type) {
        auto train = static_cast<navi::TrainModelItem*>(item)->train();
        actDulplicateTrain(train);
    }
}

void NaviTree::onDulplicateRulerContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RulerItem::Type) {
        auto r = static_cast<navi::RulerItem*>(item)->ruler();
        emit dulplicateRuler(r);
    }
}

void NaviTree::onDulplicatePageContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::PageItem::Type) {
        auto r = static_cast<navi::PageItem*>(item)->page();
        actDulplicatePage(r);
    }
}


void NaviTree::onRemoveRulerContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RulerItem::Type) {
        emit removeRulerNavi(static_cast<navi::RulerItem*>(item)->ruler());
    }
}

void NaviTree::onRemoveRoutingContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RoutingItem::Type) {
        emit removeRoutingNavi(item->row());
    }
}

void NaviTree::onEditRailwayContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RailwayItem::Type) {
        emit editRailway(static_cast<navi::RailwayItem*>(item)->railway());
    }
}

void NaviTree::onEditTrainContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::TrainModelItem::Type) {
        emit editTrain(static_cast<navi::TrainModelItem*>(item)->train());
    }
}

void NaviTree::onEditTimetableContext()
{
    auto idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::TrainModelItem::Type) {
        emit editTimetable(static_cast<navi::TrainModelItem*>(item)->train());
    }
}

void NaviTree::onEditRulerContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RulerItem::Type) {
        emit editRuler(static_cast<navi::RulerItem*>(item)->ruler());
    }
}

void NaviTree::onMergeRulerContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RulerItem::Type) {
        emit mergeRuler(static_cast<navi::RulerItem*>(item)->ruler());
    }
}

void NaviTree::onEditForbidContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::ForbidItem::Type) {
        emit editForbid(static_cast<navi::ForbidItem*>(item)->forbid(),
            static_cast<navi::ForbidItem*>(item)->railway());
    }
}

void NaviTree::onEditRoutingContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RoutingItem::Type) {
        emit editRouting(static_cast<navi::RoutingItem*>(item)->routing());
    }
}

void NaviTree::onActivatePageContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::PageItem::Type) {
        emit activatePageAt(item->row());
    }
}

void NaviTree::onCreatePageByRailContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::RailwayItem::Type) {
        auto railway = static_cast<navi::RailwayItem*>(item)->railway();
        auto& dia = _model->diagram();
        auto page = std::make_shared<DiagramPage>(dia.config(), QList<std::shared_ptr<Railway>>{railway},
            dia.validPageName(railway->name()));
        addNewPageApply(page);
    }
}

void NaviTree::onSwitchToTrainRoutingContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::TrainModelItem::Type) {
        auto train = static_cast<navi::TrainModelItem*>(item)->train();
        if (auto rt = train->routing().lock()) {
            emit focusInRouting(rt);
        }
        else {
            QMessageBox::warning(this, tr("错误"), 
                tr("当前车次[%1]无交路。").arg(train->trainName().full()));
        }
    }
}

void NaviTree::onEditPathContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::PathItem::Type) {
        emit editPathNavi(item->row());
    }
}

void NaviTree::onRemovePathContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::PathItem::Type) {
        emit removePathNavi(item->row());
    }
}

void NaviTree::onDuplicatePathContext()
{
    auto&& idx = tree->currentIndex();
    ACI* item = getItem(idx);
    if (item && item->type() == navi::PathItem::Type) {
        emit duplicatePathNavi(item->row());
    }
}

void NaviTree::actAddRailway()
{
    const QString name = _model->diagram().validRailwayName(tr("新线路"));
    auto rail = std::make_shared<Railway>(name);
    _undo->push(new qecmd::AddRailway(_model, rail));
}

void NaviTree::addNewPage()
{
    auto* dialog = new AddPageDialog(_model->diagram(), this);
    connect(dialog, SIGNAL(creationDone(std::shared_ptr<DiagramPage>)),
        this, SLOT(addNewPageApply(std::shared_ptr<DiagramPage>)));
    dialog->open();
}

void NaviTree::importTrains()
{
    auto* dialog = new ImportTrainDialog(_model->diagram(), this);
    connect(dialog, SIGNAL(trainsImported()), this, SIGNAL(trainsImported()));
    dialog->open();
}

void NaviTree::commitAddPage(std::shared_ptr<DiagramPage> page)
{
    int cnt = _model->diagram().pages().size();
    _model->insertPage(page, cnt);
    emit pageInserted(page, cnt);
}

void NaviTree::undoAddPage()
{
    int idx = _model->diagram().pages().size() - 1;
    _model->removePageAt(idx);
    emit pageRemoved(idx);
}

void NaviTree::removePage(int index)
{
    _undo->push(new qecmd::RemovePage(_model->diagram(), index, this));
}

void NaviTree::removeSingleTrain(int index)
{
    auto train = _model->diagram().trainCollection().trainAt(index);
    emit removeTrainNavi(index);
    //_undo->push(new qecmd::RemoveSingleTrain(_model, train, index));
}

void NaviTree::commitRemovePage(int idx)
{
    _model->removePageAt(idx);
    emit pageRemoved(idx);
}

void NaviTree::undoRemovePage(std::shared_ptr<DiagramPage> page, int index)
{
    _model->insertPage(page, index);
    emit pageInserted(page, index);
}

void NaviTree::actDulplicateRailway(std::shared_ptr<Railway> origin)
{
    if (!origin)
        return;
    bool ok;
    auto name = QInputDialog::getText(this, tr("创建线路副本"),
        tr("创建线路[%1]的副本。请输入新线路名称：").arg(origin->name()), QLineEdit::Normal,
        origin->name()+tr("_副本"), &ok);
    if (!ok)return;
    if (!_model->diagram().railCategory().railNameIsValid(name, nullptr)) {
        QMessageBox::warning(this, tr("错误"), tr("请输入非空且不与既有重复的线名!"));
        return;
    }
    auto rail = std::make_shared<Railway>();
    rail->operator=(*origin);
    rail->setName(name);
    //_undo->push(new qecmd::ImportRailways(_model, { rail }));
    QList<std::shared_ptr<Railway>> lst{ rail };
    emit importRailways(lst);
}

void NaviTree::actDulplicateTrain(std::shared_ptr<Train> origin)
{
    if (!origin)return;
    bool ok;
    auto name = QInputDialog::getText(this, tr("创建列车副本"),
        tr("创建列车[%1]的副本。请输入新列车的全车次：").arg(origin->trainName().full()), QLineEdit::Normal,
        origin->trainName().full() + tr("_副本"), &ok);
    if (!ok)return;
    if (!_model->diagram().trainCollection().trainNameIsValid(name, nullptr)) {
        QMessageBox::warning(this, tr("错误"), tr("请输入非空且不与既有重复的全车次!"));
        return;
    }

    auto train = std::make_shared<Train>(*origin);
    train->setTrainName(TrainName(name));
    auto& dia = _model->diagram();
    dia.updateTrain(train);
    _undo->push(new qecmd::AddNewTrain(_model, train));
}

void NaviTree::actDulplicatePage(std::shared_ptr<DiagramPage> origin)
{
    if (!origin)return;
    bool ok;
    auto name = QInputDialog::getText(this, tr("创建运行图副本"),
        tr("创建运行图页面[%1]的副本。请输入新页面名称：").arg(origin->name()), QLineEdit::Normal,
        origin->name() + tr("_副本"), &ok);
    if (!ok)return;
    if (!_model->diagram().pageNameIsValid(name,nullptr)) {
        QMessageBox::warning(this, tr("错误"), tr("请输入非空且不与既有重复的运行图页面名称！"));
        return;
    }

    auto page = origin->clone();
    page->setName(name);
    _undo->push(new qecmd::AddPage(_model->diagram(), page, this));
}

void NaviTree::actAddTrain()
{
    TrainName&& tn = _model->diagram().trainCollection().validTrainFullName(tr("新车次"));
    auto train = std::make_shared<Train>(tn);
    _undo->push(new qecmd::AddNewTrain(_model, train));
}

void NaviTree::actAddPaintedTrain(std::shared_ptr<Train> train)
{
    _undo->push(new qecmd::AddNewTrain(_model, train));
}

void NaviTree::actBatchAddTrains(const QVector<std::shared_ptr<Train>>& trains)
{
    _undo->push(new qecmd::BatchAddTrain(_model, trains));
}

void NaviTree::actImportRailways()
{
    QString res = QFileDialog::getOpenFileName(this, tr("导入线路"), QString(),
        QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)"));
    if (res.isEmpty())
        return;

    Diagram _diatmp;
    bool flag = _diatmp.fromJson(res);
    if (!flag || _diatmp.isNull()) {
        QMessageBox::warning(this, tr("错误"), tr("文件错误或为空，无法导入"));
        return;
    }

    // 2022.12.19: add selection in case of more than 1 railway
    QList<std::shared_ptr<Railway>> rails;
    if (_diatmp.railways().size()>1){
        do{
            bool ok;
            rails=SelectRailwaysTable::dlgGetRailways(this, _diatmp,
                  tr("选择线路"),
                  tr("所选文件包含多于一条线路。请选择要导入的线路。"),&ok);
            if (!ok) return;
            if (!rails.empty()) break;
            QMessageBox::warning(this,tr("错误"),
                                 tr("未选择线路"));
        }while(true);
    }else{
        rails = std::move(_diatmp.railways());
    }

    foreach (auto p , rails) {
        p->setName(_model->diagram().validRailwayName(p->name()));
    }

    emit importRailways(rails);
    //_undo->push(new qecmd::ImportRailways(_model, rails));
}

void NaviTree::importRailwayFromDB(std::shared_ptr<Railway> railway)
{
    auto r = std::make_shared<Railway>();   // copy construct!
    r->operator=(*railway);
    r->setName(_model->diagram().validRailwayName(railway->name()));
    //_undo->push(new qecmd::ImportRailways(_model, { r }));
    QList<std::shared_ptr<Railway>> lst{ r };
    emit importRailways(lst);
    auto flag = QMessageBox::question(this, tr("qETRC主程序"),
        tr("基线[%1]添加成功。是否立即创建该线路的运行图窗口？").arg(r->name()));
    if (flag == QMessageBox::Yes) {
        auto&& dia = _model->diagram();
        QString name = dia.validPageName(r->name());
        auto pg = std::make_shared<DiagramPage>(_model->diagram().config(),
            QList<std::shared_ptr<Railway>>{ r }, name);
        addNewPageApply(pg);
    }
}

void NaviTree::onDoubleClicked(const QModelIndex& index)
{
    ACI* item = getItem(index);
    if (!item)return;
    switch (item->type()) {
    case navi::RailwayItem::Type:
        emit editRailway(static_cast<navi::RailwayItem*>(item)->railway()); break;
    case navi::TrainModelItem::Type:
        emit editTrain(static_cast<navi::TrainModelItem*>(item)->train()); break;
    case navi::RulerItem::Type:
        emit editRuler(static_cast<navi::RulerItem*>(item)->ruler()); break;
    case navi::ForbidItem::Type:
        emit editForbid(static_cast<navi::ForbidItem*>(item)->forbid(),
            static_cast<navi::ForbidItem*>(item)->railway()); break;
    case navi::RoutingItem::Type:
        emit editRouting(static_cast<navi::RoutingItem*>(item)->routing()); break;
    case navi::PageItem::Type:
        emit activatePageAt(item->row()); break;
    case navi::PathItem::Type:
        emit editPathNavi(index.row()); break;
    }
}

void NaviTree::onCurrentChanged(const QModelIndex& cur, const QModelIndex& prev)
{
    ACI* ipre = getItem(prev), *icur = getItem(cur);
    //先处理取消选择的
    if (ipre&&(!icur||ipre->type()!=icur->type())) {
        switch (ipre->type()) {
        case navi::PageItem::Type:emit focusOutPage(); break;
        case navi::TrainModelItem::Type:emit focusOutTrain(); break;
        case navi::RailwayItem::Type:emit focusOutRailway(); break;
        }
    }

    //处理新选择的
    if (icur) {
        switch (icur->type()) {
        case navi::PageItem::Type:emit focusInPage(static_cast<navi::PageItem*>(icur)->page()); break;
        case navi::TrainModelItem::Type:
            emit focusInTrain(static_cast<navi::TrainModelItem*>(icur)->train()); break;
        case navi::RailwayItem::Type:
            emit focusInRailway(static_cast<navi::RailwayItem*>(icur)->railway()); break;
        case navi::RulerItem::Type:
            emit focusInRuler(static_cast<navi::RulerItem*>(icur)->ruler()); break;
        case navi::RoutingItem::Type:
            emit focusInRouting(static_cast<navi::RoutingItem*>(icur)->routing()); break;
        case navi::PathItem::Type:
            emit focusInPath(static_cast<navi::PathItem*>(icur)->getPath()); break;
        }
    }
}

void NaviTree::actExpand()
{
    tree->expand(tree->currentIndex());
}

void NaviTree::actCollapse()
{
    tree->collapse(tree->currentIndex());
}


void NaviTree::showContextMenu(const QPoint& pos)
{
    QModelIndex idx = tree->currentIndex();
    if (!idx.isValid())return;
    auto* item = static_cast<navi::AbstractComponentItem*>(idx.internalPointer());

    auto p = mapToGlobal(pos);
    
    switch (item->type()) {
    case navi::PageListItem::Type:mePageList->popup(p); break;
    case navi::RailwayListItem::Type:meRailList->popup(p); break;
    case navi::TrainListItem::Type:meTrainList->popup(p); break;
    case navi::PageItem::Type:mePage->popup(p); break;
    case navi::TrainModelItem::Type:meTrain->popup(p); break;
    case navi::RailwayItem::Type:meRailway->popup(p); break;
    case navi::RulerItem::Type:meRuler->popup(p); break;
    case navi::ForbidItem::Type:meForbid->popup(p); break;
    case navi::RoutingItem::Type:meRouting->popup(p); break;
    case navi::RoutingListItem::Type:meRoutingList->popup(p); break;
    case navi::PathListItem::Type: mePathList->popup(p); break;
    case navi::PathItem::Type: mePath->popup(p); break;
    }
    
}

qecmd::AddRailway::AddRailway(DiagramNaviModel* navi_, 
    std::shared_ptr<Railway> rail_, QUndoCommand* parent):
    QUndoCommand(QObject::tr("添加空白线路"),parent),
    navi(navi_),rail(rail_)
{
    if (!rail->empty()) {
        // 2023.08.22: this operation requires the railway to be empty.
        // The reason is that, the "affected trains" (through paths containing this railway) 
        // are not detected or updated in this operation.
        qCritical() << "Rail not empty " << rail->name();
    }
}

void qecmd::AddRailway::undo()
{
    navi->undoAddRailway();
}

void qecmd::AddRailway::redo()
{
    navi->commitAddRailway(rail);
}

qecmd::AddNewTrain::AddNewTrain(DiagramNaviModel *navi_,
       std::shared_ptr<Train> train_, QUndoCommand *parent):
    QUndoCommand(QObject::tr("新建车次: ")+train_->trainName().full(),parent),
    navi(navi_),train(train_)
{

}

void qecmd::AddNewTrain::undo()
{
    navi->undoAddTrain();
}

void qecmd::AddNewTrain::redo()
{

               navi->commitAddTrain(train);
}

void qecmd::BatchAddTrain::undo()
{
    navi->undoBatchAddTrains(trains.size());
}

void qecmd::BatchAddTrain::redo()
{
    navi->commitBatchAddTrains(trains);
}


#endif
