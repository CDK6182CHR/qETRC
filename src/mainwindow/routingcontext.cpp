#ifndef QETRC_MOBILE_2
#include "routingcontext.h"
#include "mainwindow.h"

#include "data/train/routing.h"
#include "editors/routing/parseroutingdialog.h"
#include "editors/routing/detectroutingdialog.h"
#include "editors/routing/routingdiagramwidget.h"
#include "data/train/train.h"
#include "editors/routing/routingwidget.h"
#include "defines/icon_specs.h"
#include "dialogs/selectroutingdialog.h"
#include "editors/routing/splitroutingdialog.h"
#include "editors/routing/mergeroutingdialog.h"
#include "dialogs/selectroutingdialog.h"
#include "viewers/routingmiledialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QMessageBox>
#include <SARibbonMenu.h>
#include <DockManager.h>

RoutingContext::RoutingContext(Diagram &diagram, SARibbonContextCategory *context,
                               MainWindow *mw_):
    QObject(mw_), _diagram(diagram), cont(context), mw(mw_)
{
    initUI();
}

void RoutingContext::initUI()
{
    auto* page=cont->addCategoryPage(tr("交路编辑(&0)"));

    auto* panel=page->addPanel(tr("基本信息"));

    auto* w = new QWidget;
    auto* vlay = new QVBoxLayout(w);
    auto* hlay = new QHBoxLayout;

    auto* lab = new QLabel(tr("当前交路"));
    hlay->addWidget(lab);
    lab->setAlignment(Qt::AlignCenter);
    auto* btn = new SARibbonToolButton;
    btn->setIcon(QEICN_change_routing);
    btn->setText(tr("切换"));
    connect(btn, &QToolButton::clicked, this, &RoutingContext::actChangeRouting);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    edName = new QLineEdit;
    edName->setFocusPolicy(Qt::NoFocus);
    edName->setAlignment(Qt::AlignCenter);
    edName->setFixedWidth(150);
    vlay->addWidget(edName);
    w->setObjectName(tr("交路名面板"));
    w->setWindowTitle(tr("交路名"));
    panel->addWidget(w, SARibbonPanelItem::Large);

    auto* act = mw->makeAction(QEICN_highlight_routing, tr("高亮显示"), tr("高亮显示交路"));
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &RoutingContext::toggleHighlight);
    panel->addLargeAction(act);
    btnHighlight = panel->actionToRibbonToolButton(act);

    panel=page->addPanel(tr("编辑"));
    act=mw->makeAction(QEICN_edit_routing,tr("交路编辑"));
    connect(act,SIGNAL(triggered()),this,SLOT(actEdit()));
    panel->addLargeAction(act);

    act = mw->makeAction(QEICN_split_routing, tr("拆分交路"));
    panel->addLargeAction(act);
    act->setToolTip(tr("拆分交路\n将当前交路拆分为一个或多个交路"));
    connect(act, &QAction::triggered, [this]() {
        if (!routing())return;
        auto* d = new SplitRoutingDialog(_diagram.trainCollection(), routing(), mw);
        connect(d, &SplitRoutingDialog::splitApplied, this,
            &RoutingContext::actSplitRouting);
        d->open();
        });

    act = mw->makeAction(QEICN_merge_routing, tr("合并交路"));
    panel->addLargeAction(act);
    act->setToolTip(tr("合并交路\n选择交路，将其合并到当前交路，并删除该交路"));
    connect(act, &QAction::triggered, this, &RoutingContext::actMergeRouting);

    act = mw->makeAction(QEICN_del_routing, tr("删除交路"));
    connect(act, &QAction::triggered, this, &RoutingContext::actRemoveRouting);
    panel->addLargeAction(act);

    panel = page->addPanel(tr("工具"));
    act = mw->makeAction(QEICN_routing_diagram, tr("交路图"));
    act->setToolTip(tr("交路图\n显示当前交路的（“中国动车组交路查询”风格的）示意图"));
    connect(act, &QAction::triggered, this, &RoutingContext::actRoutingDiagram);
    panel->addLargeAction(act);

    act = mw->makeAction(QEICN_routing_mile, tr("里程表"));
    act->setToolTip(tr("里程表\n显示当前交路的各车次总里程及交路累计里程（若可得）。"));
    connect(act, &QAction::triggered, [this]() {
        if (!_routing) return;
        auto* d = new RoutingMileDialog(_diagram.options(), routing(), mw);
        d->open();
        });
    panel->addLargeAction(act);

    act = mw->makeAction(QEICN_parse_routing, tr("文本解析"));
    act->setToolTip(tr("单交路文本解析\n"
        "输入车次套用的文本，从中解析交路序列，并直接应用于当前交路。"));
    connect(act, &QAction::triggered, this, &RoutingContext::actParseText);
    panel->addMediumAction(act);

    act = mw->makeAction(QEICN_identify_trains, tr("识别车次"));
    act->setToolTip(tr("单交路车次识别\n"
        "识别当前交路中的虚拟车次，尝试改变为实体车次，并直接应用结果。"));
    connect(act, &QAction::triggered, this, &RoutingContext::actDetectTrain);

    panel->addMediumAction(act);

    panel = page->addPanel("");
    act = mw->makeAction(QEICN_close_routing_context, tr("关闭面板"));
    act->setToolTip(tr("关闭面板\n关闭当前的交路编辑上下文工具栏面板。"));
    connect(act, &QAction::triggered, mw, &MainWindow::focusOutRouting);
    panel->addLargeAction(act);
}

int RoutingContext::getRoutingWidgetIndex(std::shared_ptr<Routing> routing)
{
    for(int i=0;i<routingEdits.size();i++){
        auto p=routingEdits.at(i);
        if(p->getRouting() == routing)
            return i;
    }
    return -1;
}

int RoutingContext::getRoutingWidgetIndex(RoutingEdit* w)
{
    for (int i = 0; i < routingEdits.size(); i++) {
        auto p = routingEdits.at(i);
        if (p == w)
            return i;
    }
    return -1;
}

void RoutingContext::updateRoutingEditBasic(std::shared_ptr<Routing> routing)
{
    for (int i = 0; i < routingEdits.size(); i++) {
        auto p = routingEdits.at(i);
        if (p->getRouting() == routing) {
            p->refreshBasicData();
            routingDocks.at(i)->setWindowTitle(tr("交路编辑 - %1").arg(routing->name()));
        }
    }
}

void RoutingContext::updateRoutingEdit(std::shared_ptr<Routing> routing)
{
    for (int i = 0; i < routingEdits.size(); i++) {
        auto p = routingEdits.at(i);
        if (p->getRouting() == routing) {
            p->refreshData();
            routingDocks.at(i)->setWindowTitle(tr("交路编辑 - %1").arg(routing->name()));
        }
    }
}

void RoutingContext::actEdit()
{
    openRoutingEditWidget(_routing);
}

void RoutingContext::refreshData()
{
    if (_routing) {
        edName->setText(_routing->name());
        btnHighlight->setChecked(_routing->isHighlighted());
    }
    foreach(auto w, syncEdits) {
        w->setRouting(_routing);
    }
}

void RoutingContext::toggleHighlight(bool on)
{
    if (on != _routing->isHighlighted()) {
        onRoutingHighlightChanged(_routing, on);
        emit routingHighlightChanged(_routing);
    }
}

void RoutingContext::removeRoutingEdit()
{
    auto* w = static_cast<RoutingEdit*>(sender());
    int i = getRoutingWidgetIndex(w);
    if (i >= 0) {
        auto* dock = routingDocks.takeAt(i);
        routingEdits.removeAt(i);
        dock->deleteDockWidget();
    }
}

void RoutingContext::actParseText()
{
    auto* dialog = new ParseRoutingDialog(_diagram.trainCollection(), true,
        _routing, mw);
    connect(dialog, &ParseRoutingDialog::routingParsed, this,
        &RoutingContext::actRoutingOrderChange);
    dialog->show();
}

void RoutingContext::actDetectTrain()
{
    auto* d = new DetectRoutingDialog(_diagram.trainCollection(), _routing, true, mw);
    connect(d, &DetectRoutingDialog::routingDetected, this,
        &RoutingContext::actRoutingOrderChange);
    d->show();
}

void RoutingContext::actRemoveRouting()
{
    if (_routing)
        emit removeRouting(_routing);
}


void RoutingContext::actRoutingDiagram()
{
    if (!_routing)return;
    openRoutingDiagramWidget(_routing);
}

void RoutingContext::actChangeRouting()
{
    auto ret = SelectRoutingDialog::selectRouting(_diagram.trainCollection(), false, mw);
    if (ret.isAccepted && ret.routing) {
        mw->focusInRouting(ret.routing);
    }
}

void RoutingContext::actMergeRouting()
{
    if (!routing())
        return;
    auto* d = new MergeRoutingDialog(_diagram.trainCollection(), routing(), mw);
    connect(d, &MergeRoutingDialog::mergeApplied,
        [this](std::shared_ptr<Routing> routing, std::shared_ptr<Routing> other, bool sel_pos, int pos) {
            auto* cmd = new QUndoCommand(tr("合并至交路: %1").arg(routing->name()));
            int idx = _diagram.trainCollection().getRoutingIndex(other);
            new qecmd::RemoveRouting(idx, other, mw->routingWidget, cmd);

            auto rc = routing->copyBase();
            rc->order().operator=(routing->order());
            auto itr = rc->order().begin();
            if (sel_pos) {
                std::advance(itr, pos);
            }
            else {
                itr = rc->order().end();
            }
            auto other_order_copy = other->order();   // copy construct
            rc->order().splice(itr, std::move(other_order_copy));
            new qecmd::ChangeRoutingOrder(routing, rc, this, cmd);

            mw->getUndoStack()->push(cmd);
        });
    d->open();
}

void RoutingContext::openRoutingDiagramWidget(std::shared_ptr<Routing> routing)
{
    QString report;
    if (!routing->checkForDiagram(report)) {
        QMessageBox::warning(mw, tr("错误"), tr("当前交路[%1]无法绘制交路图，原因如下：\n%2")
            .arg(routing->name(), report));
        return;
    }
    auto* d = new RoutingDiagramWidget(_diagram.options(), routing);
    auto* dock = new ads::CDockWidget(mw->manager, d->windowTitle());
    dock->setWidget(d);
    diagramWidgets.push_back(d);
    diagramDocks.push_back(dock);
    dock->resize(1300, 870);
    mw->getManager()->addDockWidgetFloating(dock);
}

void RoutingContext::createRoutingByTrain(std::shared_ptr<Train> train)
{
    auto rout = std::make_shared<Routing>();
    rout->setName(_diagram.trainCollection().validRoutingName(train->trainName().full()));
    rout->appendTrain(train, true);

    mw->undoStack->push(new qecmd::AddRouting(rout, mw->routingWidget));

    openRoutingEditWidget(rout);
}

void RoutingContext::addTrainToRouting(std::shared_ptr<Routing> routing, std::shared_ptr<Train> train)
{
    auto data = std::make_shared<Routing>(*routing);   // copy construct;
    data->appendTrain(train, true);

    mw->undoStack->push(new qecmd::ChangeRoutingOrder(routing, data, this));

    openRoutingEditWidget(routing);
}

void RoutingContext::actSplitRouting(std::shared_ptr<Routing> routing, std::vector<SplitRoutingData>& data)
{
    mw->undoStack->push(new qecmd::SplitRouting(routing, std::move(data), this,
        mw->routingWidget));
}

void RoutingContext::refreshAllData()
{
    refreshData();
    // 2025.02.06: refresh also the docks
    foreach(auto * w, routingEdits) {
        w->refreshData();
    }
}

void RoutingContext::refreshDataFor(std::shared_ptr<Routing> r)
{
    if (r == _routing) {
        refreshData();
    }
    foreach(auto * w, routingEdits) {
        if (w->getRouting() == r) {
            w->refreshData();
        }
    }
}

void RoutingContext::setRouting(std::shared_ptr<Routing> routing)
{
    _routing=routing;
    refreshData();
}

void RoutingContext::openRoutingEditWidget(std::shared_ptr<Routing> routing)
{
    int i = getRoutingWidgetIndex(routing);
    if (i == -1) {
        auto* w = new RoutingEdit(_diagram.options(), _diagram.trainCollection(), routing);
        auto* dock = new ads::CDockWidget(mw->manager, tr("交路编辑 - %1").arg(routing->name()));
        dock->setWidget(w);
        connect(w, &RoutingEdit::closeDock, this, &RoutingContext::removeRoutingEdit);
        routingEdits.push_back(w);
        routingDocks.push_back(dock);
        mw->getManager()->addDockWidgetFloating(dock);
        mw->getRoutingMenu()->addAction(dock->toggleViewAction());

        connect(w, &RoutingEdit::routingInfoChanged, this,
            &RoutingContext::actRoutingInfoChange);
        connect(w, &RoutingEdit::routingOrderChanged, this,
            &RoutingContext::actRoutingOrderChange);
        connect(w, &QWidget::windowTitleChanged, dock, &ads::CDockWidget::setWindowTitle);
        connect(w, &RoutingEdit::focusInRouting,
            mw, &MainWindow::focusInRouting);
        connect(w, &RoutingEdit::routingSplit,
            this, &RoutingContext::actSplitRouting);
        connect(w, &RoutingEdit::synchronizationChanged, [this, w](bool on) {
            if (on) {
                this->syncEdits.insert(w);
                w->setRouting(this->routing());
            }
            else {
                this->syncEdits.remove(w);
            }
            });
    }
    else {
        auto dock = routingDocks.at(i);
        if (dock->isClosed()) {
            dock->toggleView(true);
        }
        else {
            dock->setAsCurrentTab();
        }
    }
}

void RoutingContext::removeRoutingEditWidget(std::shared_ptr<Routing> routing)
{
    for (int i = 0; i < routingEdits.size();) {
        if (routingEdits.at(i)->getRouting() == routing)
            removeRoutingEditWidgetAt(i);
        else i++;
    }
}

void RoutingContext::removeNonSyncRoutingEditWidget(std::shared_ptr<Routing> routing)
{
    // for routing editors
    {
        auto ie = routingEdits.begin();
        auto id = routingDocks.begin();
        while (ie != routingEdits.end()) {
            auto* e = *ie;
            if (e->getRouting() != routing) {
                ++ie; ++id;
            }
            else if (e->isSynchronized()) {
                e->resetRouting();
                ++ie; ++id;
            }
            else {
                auto* d = *id;
                ie = routingEdits.erase(ie);
                id = routingDocks.erase(id);
                d->deleteDockWidget();
            }
        }
    }
}

void RoutingContext::removeRoutingEditWidgetAt(int i)
{
    auto* dock = routingDocks.takeAt(i);
    routingEdits.removeAt(i);
    dock->deleteDockWidget();
}

void RoutingContext::removeAllRoutingDocks()
{
    // for routing editors
    {
        auto ie = routingEdits.begin();
        auto id = routingDocks.begin();
        while (ie != routingEdits.end()) {
            auto* e = *ie;
            if (e->isSynchronized()) {
                e->resetRouting();
                ++ie; ++id;
            }
            else {
                auto* d = *id;
                ie = routingEdits.erase(ie);
                id = routingDocks.erase(id);
                d->deleteDockWidget();
            }
        }
    }

    // for routing diagrams
    foreach(auto d, diagramDocks) {
        d->deleteDockWidget();
    }
    diagramDocks.clear();
    diagramWidgets.clear();
}

void RoutingContext::onRoutingHighlightChanged(std::shared_ptr<Routing> routing, bool on)
{
    routing->setHighlight(on);
    mw->setRoutingHighlight(routing, on);
    if (routing == _routing) {
        btnHighlight->setChecked(on);
    }
}

void RoutingContext::actRoutingInfoChange(std::shared_ptr<Routing> routing, 
    std::shared_ptr<Routing> info)
{
    mw->getUndoStack()->push(new qecmd::ChangeRoutingInfo(routing, info, this));
}

void RoutingContext::commitRoutingInfoChange(std::shared_ptr<Routing> routing)
{
    if (routing == _routing) {
        refreshData();
    }
    updateRoutingEditBasic(routing);
    emit routingInfoChanged(routing);
}

void RoutingContext::actRoutingOrderChange(std::shared_ptr<Routing> routing, 
    std::shared_ptr<Routing> info)
{
    mw->getUndoStack()->push(new qecmd::ChangeRoutingOrder(routing, info, this));
}

void RoutingContext::commitRoutingOrderChange(std::shared_ptr<Routing> routing,
    [[maybe_unused]] QSet<std::shared_ptr<Train>> takenTrains)
{
    // 2024.03.26: change to mainWindow call, for updating of trainContext if required.
    mw->repaintRoutingTrainLines(routing);

    if (routing == _routing) {
        refreshData();
    }
    updateRoutingEdit(routing);
    emit routingInfoChanged(routing);
}

void RoutingContext::onRoutingRemoved(std::shared_ptr<Routing> routing)
{
    //auto s = routing->trainSet();
    //if (!s.empty()) {
    //    mw->repaintTrainLines(s);
    //}
    // 2024.03.23: also update the current train, if affected
    mw->repaintRoutingTrainLines(routing);

    removeNonSyncRoutingEditWidget(routing);
    if (routing == _routing) {
        _routing.reset();
        mw->focusOutRouting();
    }
}

void RoutingContext::actBatchRoutingUpdate(const QVector<int>& indexes, 
    const QVector<std::shared_ptr<Routing>>& data)
{
    mw->getUndoStack()->push(new qecmd::BatchChangeRoutings(indexes, data, this));
}

void RoutingContext::commitBatchRoutingUpdate(const QVector<int>& indexes, 
    const QVector<std::shared_ptr<Routing>>& data)
{
    QSet<std::shared_ptr<Train>> takenTrains; 
    for (int i = 0; i < indexes.size(); i++) {
        int index = indexes.at(i);
        auto r = _diagram.trainCollection().routingAt(index);
        auto rd = data.at(i);
        r->swap(*rd);
        r->updateTrainHooks();
        takenTrains |= r->takenTrains(*rd);
        mw->repaintRoutingTrainLines(r);
        updateRoutingEdit(r);
        if (r == _routing)
            refreshData();
        emit routingInfoChangedAt(index);
    }
    foreach(auto p, takenTrains) {
        p->resetRoutingSimple();
    }
    mw->repaintTrainLines(takenTrains);
}


qecmd::ChangeRoutingInfo::ChangeRoutingInfo(std::shared_ptr<Routing> routing_,
    std::shared_ptr<Routing> info_,
    RoutingContext* context, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("更新标尺信息: %1").arg(info_->name()), parent),
    routing(routing_), info(info_), cont(context) {}

void qecmd::ChangeRoutingInfo::redo()
{
    routing->swapBase(*info);
    cont->commitRoutingInfoChange(routing);
}

void qecmd::ChangeRoutingInfo::undo()
{
    routing->swapBase(*info);
    cont->commitRoutingInfoChange(routing);
}

qecmd::ChangeRoutingOrder::ChangeRoutingOrder(std::shared_ptr<Routing> routing_, 
    std::shared_ptr<Routing> info_, RoutingContext* context, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新交路序列: %1").arg(info_->name()),parent),
    routing(routing_),data(info_),cont(context)
{
}

void qecmd::ChangeRoutingOrder::undo()
{
    commit();
}

void qecmd::ChangeRoutingOrder::redo()
{
    commit();
}

void qecmd::ChangeRoutingOrder::commit()
{
    routing->swap(*data);
    routing->updateTrainHooks();
    auto s = routing->takenTrains(*data);
    foreach(auto & p, s) {
        p->resetRoutingSimple();
    }
    cont->commitRoutingOrderChange(routing, std::move(s));
}

void qecmd::BatchChangeRoutings::undo()
{
    cont->commitBatchRoutingUpdate(indexes, routings);
}

void qecmd::BatchChangeRoutings::redo()
{
    cont->commitBatchRoutingUpdate(indexes, routings);
}

#endif

qecmd::SplitRouting::SplitRouting(std::shared_ptr<Routing> routing, std::vector<SplitRoutingData>&& data_, 
    RoutingContext* cont, RoutingWidget* rw, QUndoCommand* parent):
    QUndoCommand(QObject::tr("拆分交路: %1").arg(routing->name()), parent),
    routing(routing), data(std::move(data_)),cont(cont)
{
    auto rc = routing->copyBase();
    rc->order().operator=(routing->order());   // copy constructor
    std::list<RoutingNode> order_removed;
    auto itr = rc->order().begin();
    std::advance(itr, data.front().idx_first);
    order_removed.splice(order_removed.end(), rc->order(),
        itr, rc->order().end());

    // First: change the order of the current routing
    new qecmd::ChangeRoutingOrder(routing, rc, cont, this);

    // Now for each new routings
    for (const auto& d : data) {
        auto nr = routing->copyBase();
        nr->setName(d.name);
        new qecmd::AddRouting(nr, rw, this);

        auto nr_new = nr->copyBase();
        // Each time we take from the first of order_removed list
        auto itr_end = order_removed.begin();
        std::advance(itr_end, d.idx_last - d.idx_first);
        nr_new->order().splice(nr_new->order().end(), order_removed,
            order_removed.begin(), itr_end);
        new qecmd::ChangeRoutingOrder(nr, nr_new, cont, this);
    }
}


