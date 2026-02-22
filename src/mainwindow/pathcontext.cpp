#include "pathcontext.h"

#ifndef QETRC_MOBILE_2

#include <algorithm>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QInputDialog>
#include <QMessageBox>

#include <SARibbonContextCategory.h>
#include <SARibbonMenu.h>
#include <DockWidget.h>
#include <DockManager.h>

#include "mainwindow.h"
#include "traincontext.h"
#include "editors/trainpath/pathedit.h"
#include "editors/trainpath/pathlistwidget.h"
#include "editors/trainpath/pathtrainsdialog.h"
#include "editors/trainpath/addtrainstopathdialog.h"
#include "data/train/train.h"
#include "defines/icon_specs.h"
#include "editors/trainpath/selectpathdialog.h"
#include "editors/trainpath/pathrulereditor.h"
#include "model/diagram/diagramnavimodel.h"


PathContext::PathContext(Diagram &diagram, SARibbonContextCategory *cont,
                           MainWindow *mw, QObject *parent):
    QObject(parent), diagram(diagram), cont(cont), mw(mw)
{
    initUI();
}

void PathContext::setPath(TrainPath *path)
{
    this->path=path;
    refreshData();
}

void PathContext::refreshData()
{
    if (!path) {
        qWarning() << "empty path";
        return;
    }

    edName->setText(path->name());
}

void PathContext::initUI()
{
    auto* page=cont->addCategoryPage(tr("列车径路(&A)"));
    auto* panel=page->addPanel(tr(""));

    if constexpr (true) {
        auto* ed = new QLineEdit;
        edName = ed;

        QWidget* w = new QWidget;
        auto* vlay = new QVBoxLayout;
        auto* hlay = new QHBoxLayout;

        auto* lab = new QLabel(tr("当前列车径路"));
        lab->setAlignment(Qt::AlignCenter);
        hlay->addWidget(lab);
        auto* btn = new SARibbonToolButton();
        btn->setIcon(QEICN_change_path);
        //btn->setText(tr("切换"));
        connect(btn, &QToolButton::clicked, this, &PathContext::actChangePath);
        hlay->addWidget(btn);
        vlay->addLayout(hlay);

        ed->setReadOnly(true);
        ed->setAlignment(Qt::AlignCenter);
        ed->setFixedWidth(100);

        vlay->addWidget(ed);
        w->setLayout(vlay);
        w->setObjectName(tr("径路名面板"));
        w->setWindowTitle(tr("径路名称"));
        panel->addWidget(w, SARibbonPanelItem::Large);
    }

    panel=page->addPanel(tr("编辑"));

    auto* act = mw->makeAction(QEICN_edit_path, tr("编辑"), tr("编辑径路"));
    connect(act,&QAction::triggered, this, &PathContext::actEditPath);
    panel->addLargeAction(act);

    panel = page->addPanel(tr("径路标尺"));
    act = mw->makeAction(QEICN_add_path_ruler, tr("新建标尺"), tr("新建径路标尺"));
    act->setToolTip(tr("新建径路标尺\n通过手动选择各段线路的标尺，来新建列车径路标尺"));
    connect(act, &QAction::triggered, this, &PathContext::actAddPathRuler);
    panel->addLargeAction(act);

    panel = page->addPanel(tr("列车"));

    act = mw->makeAction(QEICN_path_trains, tr("列车表"), tr("径路列车表"));
    connect(act, &QAction::triggered, this, &PathContext::actShowTrains);
    panel->addLargeAction(act);

    act = mw->makeAction(QEICN_path_add_trains, tr("添加列车"), tr("添加列车到径路"));
    connect(act, &QAction::triggered, this, &PathContext::actAddTrains);
    panel->addLargeAction(act);

    act = mw->makeAction(QEICN_path_clear_trains, tr("清空"), tr("清空径路列车"));
    connect(act, &QAction::triggered, this, &PathContext::actClearTrains);
    panel->addLargeAction(act);

    panel=page->addPanel(tr(""));
    act = mw->makeAction(QEICN_del_path, tr("删除径路"));
    act->setToolTip(tr("删除径路\n删除当前列车径路"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &PathContext::actRemovePath);

    act = mw->makeAction(QEICN_copy_path, tr("副本"), tr("径路副本"));
    act->setToolTip(tr("创建径路副本\n创建当前列车径路副本"));
    panel->addLargeAction(act);
    connect(act, &QAction::triggered, this, &PathContext::actDuplicatePath);

    act = mw->makeAction(QEICN_close_path_context, tr("关闭面板"), tr("关闭径路面板"));
    connect(act, &QAction::triggered, mw, &MainWindow::focusOutPath);
    act->setToolTip(tr("关闭面板\n关闭当前的列车径路上下文工具栏页面。"));
    panel->addLargeAction(act);

}

int PathContext::pathEditIndex(const TrainPath* path)
{
    for (int i = 0; i < pathEdits.size(); i++) {
        if (pathEdits.at(i)->path() == path)
            return i;
    }
    return -1;
}

void PathContext::actRemovePath()
{
    int idx = diagram.pathCollection().pathIndex(path);
    removePath(idx);
    //mw->pathListWidget->actRemovePath(idx);
}

void PathContext::actDuplicatePath()
{
    int idx = diagram.pathCollection().pathIndex(path);
    mw->pathListWidget->actDuplicate(idx);
}

void PathContext::actAddPathRuler()
{
    auto* w = new PathRulerEditor(path, mw);
    w->setWindowFlag(Qt::Dialog);
    w->setWindowModality(Qt::ApplicationModal);
    connect(w, &PathRulerEditor::rulerAdded, [this](std::shared_ptr<PathRuler> ruler) {
        mw->getUndoStack()->push(new qecmd::AddPathRuler(this, std::move(ruler)));
        });
    w->show();
}

void PathContext::actSwitchToPathRuler()
{
}

void PathContext::actShowTrains()
{
    auto* d = new PathTrainsDialog(diagram.options(), diagram.trainCollection(), path, mw);
    connect(d, &PathTrainsDialog::actAdd, this, &PathContext::actAddTrains);
    connect(d, &PathTrainsDialog::removeTrains, this, &PathContext::actRemoveTrains);
    d->open();
}

void PathContext::actAddTrains()
{
    auto* w = new AddTrainsToPathDialog(diagram.options(), diagram.trainCollection(), path, mw);
    connect(w, &AddTrainsToPathDialog::trainsAdded,
        this, &PathContext::addTrains);
    w->open();
}

void PathContext::actChangePath()
{
    auto res = SelectPathDialog::getPaths(mw, tr("请选择要切换到的列车径路，将同时设为当前列车径路。"),
        diagram.pathCollection().pathPointers(), true);
    if (!res.empty()) {
        mw->focusInPath(res.front());
    }
}

void PathContext::actEditPath()
{
    openPathEdit(path);
}


void PathContext::openPathEdit(TrainPath* path)
{
    int i = pathEditIndex(path);
    if (i == -1) {
        //创建
        auto* pw = new PathEdit(diagram.railCategory(), diagram.pathCollection());
        pw->setPath(path);
        auto* dock = new ads::CDockWidget(mw->manager, tr("列车径路 - %1").arg(path->name()));
        dock->setWidget(pw);
        pathEdits.append(pw);
        pathDocks.append(dock);

        connect(pw, &PathEdit::pathApplied,
            this, &PathContext::actUpdatePath);
        connect(mw->pathListWidget, &PathListWidget::pathRemoved,
            this, &PathContext::afterPathRemoved);

        mw->pathMenu->addAction(dock->toggleViewAction());
        mw->getManager()->addDockWidgetFloating(dock);
    }
    else {
        //显示
        auto* dock = pathDocks.at(i);
        if (dock->isClosed()) {
            dock->toggleView(true);
        }
        else {
            dock->setAsCurrentTab();
        }
    }
}

void PathContext::openPathEditIndex(int idx)
{
    auto* path = diagram.pathCollection().at(idx);
    openPathEdit(path);
}

void PathContext::removePathDocks()
{
    foreach(auto p, pathDocks) {
        p->deleteDockWidget();
    }
    pathDocks.clear();
    pathEdits.clear();
}

void PathContext::actUpdatePath(TrainPath* path, std::unique_ptr<TrainPath>& data)
{
    auto* cmd = new qecmd::UpdatePath(path, std::move(data), this);
    auto trains = path->trainsShared();
    new qecmd::RebindTrainsByPaths(std::move(trains), mw->getTrainContext(), cmd);
    mw->getUndoStack()->push(cmd);
}

void PathContext::commitUpdatePath(TrainPath* path)
{
    // inform the PathListWidget to update path
    mw->pathListWidget->updatePath(path);

    // update the path editor, if available
    if (int idx = pathEditIndex(path); idx >= 0) {
        auto* ed = pathEdits.at(idx);
        ed->refreshData();
        pathDocks.at(idx)->setWindowTitle(tr("列车径路 - %1").arg(path->name()));
    }
}

void PathContext::afterPathRemoved(TrainPath* path)
{
    int idx = pathEditIndex(path);
    if (idx >= 0) {
        auto* dock = pathDocks.takeAt(idx);
        dock->deleteDockWidget();

        pathEdits.removeAt(idx);
    }
    if (path == this->path) {
        mw->focusOutPath();
    }
}

void PathContext::actClearTrains()
{
    mw->getUndoStack()->push(new qecmd::ClearTrainsFromPath(path, this));
}

void PathContext::afterPathTrainsChanged([[maybe_unused]] TrainPath* path, const std::vector<std::weak_ptr<Train>>& trains)
{
    // rebind and repaint trains
    auto* c = mw->getTrainContext();
    for (auto t : trains) {
        auto train = t.lock();
        c->afterTimetableChanged(train);
    }
    c->refreshPath();

    // update path data (nothing to do for now)
}

void PathContext::afterPathTrainsChanged([[maybe_unused]] TrainPath* path, const std::vector<qecmd::TrainInfoInPath>& data)
{
    // rebind and repaint trains
    auto* c = mw->getTrainContext();
    for (const auto& d: data) {
        auto train = d.train.lock();
        c->afterTimetableChanged(train);
    }
    c->refreshPath();

    // update path data (nothing to do for now)
}

void PathContext::afterPathTrainsChanged(const QList<std::shared_ptr<Train>>& trains)
{
    // rebind and repaint trains
    auto* c = mw->getTrainContext();
    foreach(const auto& t , trains) {
        c->afterTimetableChanged(t);
    }
    c->refreshPath();

    // update path data (nothing to do for now)
}

void PathContext::removePath(int idx)
{
    auto p = diagram.pathCollection().at(idx);
    mw->getUndoStack()->push(new qecmd::RemoveTrainPath(p, idx, mw->pathListWidget, this));
}

void PathContext::actRemoveTrains(TrainPath* path, const std::set<int>& indexes)
{
    std::vector<qecmd::TrainInfoInPath> info{};
    for (int idx : indexes) {
        auto t = path->trains().at(idx).lock();
        int path_index_in_train = t->getPathIndex(path);
        info.emplace_back(t, idx, path_index_in_train);
    }

    mw->getUndoStack()->push(new qecmd::RemoveTrainsFromPath(path, std::move(info), this));
}

void PathContext::addTrains(TrainPath* path, QList<std::shared_ptr<Train>> trains)
{
    mw->getUndoStack()->push(new qecmd::AddTrainsToPath(path, std::move(trains), this));
}

void PathContext::insertRulerAt(TrainPath* path, int idx, std::shared_ptr<PathRuler> ruler)
{
    auto p = std::next(path->rulers().begin(), idx);
    path->rulers().insert(p, ruler);
    mw->naviModel->onPathRulerInserted(path, idx);
}

void PathContext::removeRulerAt(TrainPath* path, int idx)
{
    auto p = std::next(path->rulers().begin(), idx);
    path->rulers().erase(p);
    mw->naviModel->onPathRulerRemoved(path, idx);
}

void PathContext::actEditPathRuler(std::shared_ptr<PathRuler> ruler)
{
    auto* w = new PathRulerEditor(std::move(ruler), mw);
    w->setWindowFlag(Qt::Dialog, true);
    w->setWindowModality(Qt::ApplicationModal);
    connect(w, &PathRulerEditor::rulerModified,
        [this](std::shared_ptr<PathRuler> ruler, std::shared_ptr<PathRuler> data) {
            mw->getUndoStack()->push(new qecmd::UpdatePathRuler(this, std::move(ruler), std::move(data)));
        });
    w->show();
}

void PathContext::actRemovePathRuler(std::shared_ptr<PathRuler> ruler)
{
    mw->getUndoStack()->push(new qecmd::RemovePathRuler(this, ruler));
}

void PathContext::actDuplicatePathRuler(std::shared_ptr<PathRuler> ruler)
{
    bool ok;
    auto newName = QInputDialog::getText(mw, tr("径路标尺副本"),
        tr("请输入新的列车径路标尺名称"), QLineEdit::Normal, ruler->name() + tr("_副本"), &ok);
    if (!ok)
        return;
    if (!ruler->path()->rulerNameIsValid(newName, {})) {
        QMessageBox::warning(mw, tr("错误"), tr("列车径路标尺名称为空或已存在，无法创建副本"));
        return;
    }

    auto newRuler = std::make_shared<PathRuler>(*ruler);   // copy
    newRuler->setName(newName);
    mw->getUndoStack()->push(new qecmd::AddPathRuler(this, std::move(newRuler)));
}

void PathContext::onPathRulerUpdated([[maybe_unused]] std::shared_ptr<PathRuler> ruler)
{
    // Seems nothing to do for now
}

void PathContext::actBatchAssignPath(QList<std::shared_ptr<Train>> trains)
{
    auto paths = SelectPathDialog::getPaths(mw, tr("请选择要添加到所选列车的径路"),
        diagram.pathCollection().pathPointers(), true);
    if (paths.empty())
        return;
    auto p = paths.front();

    // avoid adding duplicate ones
    trains.erase(std::remove_if(trains.begin(), trains.end(), [p](const std::shared_ptr<Train>& t) -> bool {
        return t->hasPath(p);
        }), trains.end());
    if (trains.empty()) {
        QMessageBox::information(mw, tr("提示"), tr("所选列车为空，或所选列车皆已经包含所选径路，无需操作"));
        return;
    }

    mw->getUndoStack()->push(new qecmd::AddTrainsToPath(p, std::move(trains), this));
}

void PathContext::actBatchClearPaths(QList<std::shared_ptr<Train>> trains)
{
    trains.erase(std::remove_if(trains.begin(), trains.end(), [](const std::shared_ptr<Train>& t) -> bool {
        return t->paths().empty();
        }), trains.end());
    if (trains.empty()) {
        QMessageBox::information(mw, tr("提示"), tr("所选列车为空，或所选列车皆不含任何径路，无需操作"));
        return;
    }

    mw->getUndoStack()->push(new qecmd::ClearPathsFromTrainBatch(std::move(trains), this));
}


qecmd::UpdatePath::UpdatePath(TrainPath* path, std::unique_ptr<TrainPath>&& data_,
    PathContext* cont, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("更新列车径路: %1").arg(path->name()), parent),
    path(path), data(std::move(data_)), cont(cont)
{
}

void qecmd::UpdatePath::undo()
{
    path->swapDataWith(*data);
    cont->commitUpdatePath(path);
    QUndoCommand::undo();   // rebind trains
}

void qecmd::UpdatePath::redo()
{
    path->swapDataWith(*data);
    cont->commitUpdatePath(path);
    QUndoCommand::redo();   // rebind trains
}

qecmd::ClearTrainsFromPath::ClearTrainsFromPath(TrainPath* path, PathContext* cont, QUndoCommand* parent):
    QUndoCommand(QObject::tr("清空径路的列车: %1").arg(path->name()), parent), path(path), cont(cont)
{
    for (auto t : path->trains()) {
        auto train = t.lock();
        int idx = train->getPathIndex(path);
        indexes_in_train.emplace_back(idx);
    }
}

void qecmd::ClearTrainsFromPath::undo()
{
    std::swap(trains, path->trains());
    for (int i = 0; i < path->trains().size(); i++) {
        auto train = path->trains().at(i).lock();
        int idx = indexes_in_train.at(i);
        train->paths().insert(train->paths().begin() + idx, path);
    }
    cont->afterPathTrainsChanged(path, path->trains());
}

void qecmd::ClearTrainsFromPath::redo()
{
    std::swap(trains, path->trains());
    for (int i = 0; i < trains.size(); i++) {
        auto train = trains.at(i).lock();
        int idx = indexes_in_train.at(i);
        train->paths().erase(train->paths().begin() + idx);
    }
    cont->afterPathTrainsChanged(path, trains);
}

qecmd::ClearPathsFromTrainBatch::ClearPathsFromTrainBatch(QList<std::shared_ptr<Train>>&& trains,
    PathContext* cont, QUndoCommand* parent):
    QUndoCommand(QObject::tr("批量清除%1列车的径路").arg(trains.size()), parent),
    m_trains(std::move(trains)), m_train_paths(m_trains.size(), {}),
    m_cont(cont)
{
    for (const auto& train : m_trains) {
        for (auto p : train->paths()) {
            int idx = p->getTrainIndex(train);
            if (idx >= 0) {
                m_indexes_in_path[p].emplace(idx, train);
            }
            else {
                qWarning() << "ClearPathsFromTrainBatch: train " << train->trainName().full() << " not found in path "
                    << p->name();
            }
        }
    }
}

void qecmd::ClearPathsFromTrainBatch::undo()
{
    assert(m_trains.size() == m_train_paths.size());
    for (int n = 0; n < m_trains.size(); n++) {
        std::swap(m_trains.at(n)->paths(), m_train_paths.at(n));
    }

    for (const auto& itr : m_indexes_in_path) {
        auto* p = itr.first;
        const auto& path_indexes_map = itr.second;

        // The ORDER b.first is guaranteed!
        for (const auto& b : path_indexes_map) {
            p->trains().insert(p->trains().begin() + b.first, b.second);
        }
    }

    m_cont->afterPathTrainsChanged(m_trains);
}

void qecmd::ClearPathsFromTrainBatch::redo()
{
    assert(m_trains.size() == m_train_paths.size());
    for (int n = 0; n < m_trains.size(); n++) {
        std::swap(m_trains.at(n)->paths(), m_train_paths.at(n));
    }

    // Now, remove the trains from path data
    for (const auto& itr : m_indexes_in_path) {
        auto* p = itr.first;
        const auto& path_indexes_map = itr.second;

        // The ORDER b.first is guaranteed!
        for (auto a = path_indexes_map.rbegin(); a != path_indexes_map.rend(); ++a) {
            p->trains().erase(p->trains().begin() + a->first);
        }
    }

    m_cont->afterPathTrainsChanged(m_trains);
}


qecmd::RemoveTrainPath::RemoveTrainPath(TrainPath* p, int idx, PathListWidget* pw, PathContext* cont, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("删除径路: %1").arg(p->name()), parent), idx(idx), pw(pw), path(), cont(cont)
{
    if (!p->trains().empty()) {
        new ClearTrainsFromPath(p, cont, this);
    }
}

void qecmd::RemoveTrainPath::undo()
{
    pw->undoRemovePath(idx, std::move(path));
    QUndoCommand::undo();    // re-add the trains
}

void qecmd::RemoveTrainPath::redo()
{
    QUndoCommand::redo();    // clear the trains from path
    path = pw->commitRemovePath(idx);
    cont->afterPathRemoved(path.get());
}


#endif

qecmd::RemoveTrainsFromPath::RemoveTrainsFromPath(TrainPath* path, std::vector<TrainInfoInPath>&& data_,
    PathContext* cont, QUndoCommand* parent):
    QUndoCommand(QObject::tr("从径路[%1]删除%2列车").arg(path->name()).arg(data_.size()), parent), 
    path(path), data(std::move(data_)), cont(cont)
{
}

void qecmd::RemoveTrainsFromPath::undo()
{
    // add from first
    for (const auto& d : data) {
        path->insertTrainWithIndex(d.train.lock(), d.train_index_in_path, d.path_index_in_train);
    }
    cont->afterPathTrainsChanged(path, data);
}

void qecmd::RemoveTrainsFromPath::redo()
{
    // remove from back
    for (auto itr = data.crbegin(); itr != data.crend(); ++itr) {
        path->removeTrainWithIndex(itr->train.lock(), itr->train_index_in_path, itr->path_index_in_train);
    }
    cont->afterPathTrainsChanged(path, data);
}

qecmd::AddTrainsToPath::AddTrainsToPath(TrainPath* path, QList<std::shared_ptr<Train>>&& trains_, 
    PathContext* cont, QUndoCommand* parent):
    QUndoCommand(QObject::tr("添加%1列车到径路%2").arg(trains_.size()).arg(path->name()), parent), 
    path(path), trains(std::move(trains_)), cont(cont)
{
}

void qecmd::AddTrainsToPath::undo()
{
    for (auto itr = trains.crbegin(); itr != trains.crend(); ++itr) {
        path->removeTrainFromBack(*itr);
    }
    cont->afterPathTrainsChanged(trains);
}

void qecmd::AddTrainsToPath::redo()
{
    foreach (auto t , trains) {
        path->addTrain(t);
    }
    cont->afterPathTrainsChanged(trains);
}

qecmd::AssignPathsToTrainsBatch::AssignPathsToTrainsBatch(std::vector<TrainPath*> paths,
    QVector<std::shared_ptr<Train>> trains, PathContext* cont, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("批量添加径路至车次"), parent),
    paths(std::move(paths)), trains(std::move(trains)), cont(cont)
{
}

void qecmd::AssignPathsToTrainsBatch::redo()
{
    if (paths.empty())
        return;
    for (auto& p : paths) {
        foreach(auto t, trains) {
            p->addTrain(t);
        }
    }

    // This is exactly the same as the single-path operation for now
    cont->afterPathTrainsChanged(trains);
}

void qecmd::AssignPathsToTrainsBatch::undo()
{
    if (paths.empty())
        return;
    for (auto pit = paths.rbegin(); pit != paths.rend(); ++pit) {
        for (auto tit = trains.crbegin(); tit != trains.crend(); ++tit) {
            (*pit)->removeTrainFromBack(*tit);
        }
    }
    cont->afterPathTrainsChanged(trains);
}

qecmd::AddPathRuler::AddPathRuler(PathContext* context, std::shared_ptr<PathRuler> ruler, QUndoCommand* parent):
    QUndoCommand(QObject::tr("新增径路标尺: %1").arg(ruler->name()), parent),
    m_cont(context), m_ruler(std::move(ruler)),
    m_index((int)m_ruler->path()->rulers().size())
{
}

void qecmd::AddPathRuler::undo()
{
    m_cont->removeRulerAt(m_ruler->path(), m_index);
}

void qecmd::AddPathRuler::redo()
{
    m_cont->insertRulerAt(m_ruler->path(), m_index, m_ruler);
}

qecmd::RemovePathRuler::RemovePathRuler(PathContext* context, std::shared_ptr<PathRuler> ruler, QUndoCommand* parent):
    QUndoCommand(QObject::tr("删除列车径路标尺: %1").arg(ruler->name()), parent),
    m_cont(context),
    m_ruler(std::move(ruler)), m_index(m_ruler->path()->getRulerIndex(m_ruler))
{
}

void qecmd::RemovePathRuler::undo()
{
    m_cont->insertRulerAt(m_ruler->path(), m_index, m_ruler);
}

void qecmd::RemovePathRuler::redo()
{
    m_cont->removeRulerAt(m_ruler->path(), m_index);
}

qecmd::UpdatePathRuler::UpdatePathRuler(PathContext* context, std::shared_ptr<PathRuler> ruler, 
    std::shared_ptr<PathRuler> data, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新列车径路标尺: %1").arg(ruler->name()), parent),
    m_cont(context), m_ruler(std::move(ruler)), m_data(std::move(data))
{
}

void qecmd::UpdatePathRuler::undo()
{
    m_ruler->swapWith(*m_data);
    m_cont->onPathRulerUpdated(m_ruler);
}

void qecmd::UpdatePathRuler::redo()
{
    m_ruler->swapWith(*m_data);
    m_cont->onPathRulerUpdated(m_ruler);
}
