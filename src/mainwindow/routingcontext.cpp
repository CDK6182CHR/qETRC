#ifndef QETRC_MOBILE_2
#include "routingcontext.h"
#include "mainwindow.h"

#include "data/train/routing.h"
#include "editors/routing/parseroutingdialog.h"
#include "editors/routing/detectroutingdialog.h"
#include "editors/routing/routingdiagramwidget.h"
#include "data/train/train.h"
#include "editors/routing/routingwidget.h"

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

    auto* panel=page->addPannel(tr("基本信息"));

    auto* w = new QWidget;
    auto* vlay = new QVBoxLayout(w);

    auto* lab = new QLabel(tr("当前交路"));
    vlay->addWidget(lab);
    lab->setAlignment(Qt::AlignCenter);

    edName = new QLineEdit;
    edName->setFocusPolicy(Qt::NoFocus);
    edName->setAlignment(Qt::AlignCenter);
    edName->setFixedWidth(150);
    vlay->addWidget(edName);
    panel->addWidget(w, SARibbonPannelItem::Large);

    auto* act = new QAction(QIcon(":/icons/trainline.png"), tr("高亮显示"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &RoutingContext::toggleHighlight);
    panel->addLargeAction(act);
    btnHighlight = panel->actionToRibbonToolButton(act);

    panel=page->addPannel(tr("编辑"));
    act=new QAction(QIcon(":/icons/edit.png"),tr("交路编辑"),this);
    connect(act,SIGNAL(triggered()),this,SLOT(actEdit()));
    panel->addLargeAction(act);

    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
        tr("删除交路"), this);
    connect(act, &QAction::triggered, this, &RoutingContext::actRemoveRouting);
    panel->addLargeAction(act);

    panel = page->addPannel(tr("工具"));
    act = new QAction(QIcon(":/icons/routing-diagram.png"), tr("交路图"), this);
    act->setToolTip(tr("交路图\n显示当前交路的（“中国动车组交路查询”风格的）示意图"));
    connect(act, &QAction::triggered, this, &RoutingContext::actRoutingDiagram);
    panel->addLargeAction(act);

    act = new QAction(QIcon(":/icons/text.png"), tr("文本解析"), this);
    act->setToolTip(tr("单交路文本解析\n"
        "输入车次套用的文本，从中解析交路序列，并直接应用于当前交路。"));
    connect(act, &QAction::triggered, this, &RoutingContext::actParseText);
    panel->addMediumAction(act);

    act = new QAction(QIcon(":/icons/identify.png"), tr("识别车次"), this);
    act->setToolTip(tr("单交路车次识别\n"
        "识别当前交路中的虚拟车次，尝试改变为实体车次，并直接应用结果。"));
    connect(act, &QAction::triggered, this, &RoutingContext::actDetectTrain);

    panel->addMediumAction(act);

    panel = page->addPannel("");
    act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("关闭面板"), this);
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

void RoutingContext::openRoutingDiagramWidget(std::shared_ptr<Routing> routing)
{
    QString report;
    if (!routing->checkForDiagram(report)) {
        QMessageBox::warning(mw, tr("错误"), tr("当前交路[%1]无法绘制交路图，原因如下：\n%2")
            .arg(routing->name(), report));
        return;
    }
    auto* d = new RoutingDiagramWidget(routing);
    auto* dock = new ads::CDockWidget(d->windowTitle());
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

void RoutingContext::refreshAllData()
{
    refreshData();
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
        auto* w = new RoutingEdit(_diagram.trainCollection(), routing);
        auto* dock = new ads::CDockWidget(tr("交路编辑 - %1").arg(routing->name()));
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
        connect(w, &RoutingEdit::focusInRouting,
            mw, &MainWindow::focusInRouting);
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

void RoutingContext::removeRoutingEditWidgetAt(int i)
{
    auto* dock = routingDocks.takeAt(i);
    routingEdits.removeAt(i);
    dock->deleteDockWidget();
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
    QSet<std::shared_ptr<Train>> takenTrains)
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

    removeRoutingEditWidget(routing);
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
